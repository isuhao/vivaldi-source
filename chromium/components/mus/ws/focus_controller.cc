// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/mus/ws/focus_controller.h"

#include "base/macros.h"
#include "components/mus/public/interfaces/window_manager.mojom.h"
#include "components/mus/ws/focus_controller_delegate.h"
#include "components/mus/ws/focus_controller_observer.h"
#include "components/mus/ws/server_window.h"
#include "components/mus/ws/server_window_drawn_tracker.h"

namespace mus {
namespace ws {

namespace {

ServerWindow* GetDeepestLastDescendant(ServerWindow* window) {
  while (!window->children().empty())
    window = window->children().back();
  return window;
}

// This can be used to iterate over each node in a rooted tree for the purpose
// of shifting focus/activation.
class WindowTreeIterator {
 public:
  explicit WindowTreeIterator(ServerWindow* root) : root_(root) {}
  ~WindowTreeIterator() {}

  ServerWindow* GetNext(ServerWindow* window) const {
    if (window == root_ || window == nullptr)
      return GetDeepestLastDescendant(root_);

    // Return the next sibling.
    ServerWindow* parent = window->parent();
    if (parent) {
      const ServerWindow::Windows& siblings = parent->children();
      ServerWindow::Windows::const_reverse_iterator iter =
          std::find(siblings.rbegin(), siblings.rend(), window);
      DCHECK(iter != siblings.rend());
      ++iter;
      if (iter != siblings.rend())
        return GetDeepestLastDescendant(*iter);
    }

    // All children and siblings have been explored. Next is the parent.
    return parent;
  }

 private:
  ServerWindow* root_;

  DISALLOW_COPY_AND_ASSIGN(WindowTreeIterator);
};

}  // namespace

FocusController::FocusController(FocusControllerDelegate* delegate,
                                 ServerWindow* root)
    : delegate_(delegate),
      root_(root),
      focused_window_(nullptr),
      active_window_(nullptr),
      activation_reason_(ActivationChangeReason::UNKNONW) {
  DCHECK(delegate_);
  DCHECK(root_);
}

FocusController::~FocusController() {
}

void FocusController::SetFocusedWindow(ServerWindow* window) {
  if (GetFocusedWindow() == window)
    return;

  SetFocusedWindowImpl(FocusControllerChangeSource::EXPLICIT, window);
}

ServerWindow* FocusController::GetFocusedWindow() {
  return focused_window_;
}

void FocusController::ActivateNextWindow() {
  WindowTreeIterator iter(root_);
  ServerWindow* activate = active_window_;
  while (true) {
    activate = iter.GetNext(activate);
    if (activation_reason_ == ActivationChangeReason::CYCLE) {
      if (activate == active_window_) {
        // We have cycled over all the activatable windows. Remove the oldest
        // window that was cycled.
        if (!cycle_windows_->windows().empty()) {
          cycle_windows_->Remove(cycle_windows_->windows().front());
          continue;
        }
      } else if (cycle_windows_->Contains(activate)) {
        // We are cycling between activated windows, and this window has already
        // been through the cycle. So skip over it.
        continue;
      }
    }
    if (activate == active_window_ || CanBeActivated(activate))
      break;
  }
  SetActiveWindow(activate, ActivationChangeReason::CYCLE);

  if (active_window_) {
    // Do not shift focus if the focused window already lives in the active
    // window.
    if (focused_window_ && active_window_->Contains(focused_window_))
      return;
    // Focus the first focusable window in the tree.
    WindowTreeIterator iter(active_window_);
    ServerWindow* focus = nullptr;
    do {
      focus = iter.GetNext(focus);
    } while (focus != active_window_ && !CanBeFocused(focus));
    SetFocusedWindow(focus);
  } else {
    SetFocusedWindow(nullptr);
  }
}

void FocusController::AddObserver(FocusControllerObserver* observer) {
  observers_.AddObserver(observer);
}

void FocusController::RemoveObserver(FocusControllerObserver* observer) {
  observers_.RemoveObserver(observer);
}

void FocusController::SetActiveWindow(ServerWindow* window,
                                      ActivationChangeReason reason) {
  DCHECK(!window || CanBeActivated(window));
  if (active_window_ == window)
    return;
  if (reason != ActivationChangeReason::CYCLE) {
    cycle_windows_.reset();
  } else if (activation_reason_ != ActivationChangeReason::CYCLE) {
    DCHECK(!cycle_windows_);
    cycle_windows_.reset(new ServerWindowTracker());
    if (active_window_)
      cycle_windows_->Add(active_window_);
  }

  ServerWindow* old_active = active_window_;
  active_window_ = window;
  activation_reason_ = reason;
  FOR_EACH_OBSERVER(FocusControllerObserver, observers_,
                    OnActivationChanged(old_active, active_window_));
  if (active_window_ && activation_reason_ == ActivationChangeReason::CYCLE)
    cycle_windows_->Add(active_window_);
}

bool FocusController::CanBeFocused(ServerWindow* window) const {
  // All ancestors of |window| must be drawn, and be focusable.
  for (ServerWindow* w = window; w; w = w->parent()) {
    if (!w->IsDrawn())
      return false;
    if (!w->can_focus())
      return false;
  }

  // |window| must be a descendent of an activatable window.
  return GetActivatableAncestorOf(window) != nullptr;
}

bool FocusController::CanBeActivated(ServerWindow* window) const {
  DCHECK(window);
  // A detached window cannot be activated.
  if (!root_->Contains(window))
    return false;

  // The parent window must be allowed to have active children.
  if (!delegate_->CanHaveActiveChildren(window->parent()))
    return false;

  // The window must be drawn, or if it's not drawn, it must be minimized.
  if (!window->IsDrawn()) {
    bool is_minimized = false;
    const ServerWindow::Properties& props = window->properties();
    if (props.count(mojom::WindowManager::kShowState_Property)) {
      is_minimized =
          props.find(mojom::WindowManager::kShowState_Property)->second[0] ==
          mus::mojom::SHOW_STATE_MINIMIZED;
    }
    if (!is_minimized)
      return false;
  }

  // TODO(sad): If there's a transient modal window, then this cannot be
  // activated.
  return true;
}

ServerWindow* FocusController::GetActivatableAncestorOf(
    ServerWindow* window) const {
  for (ServerWindow* w = window; w; w = w->parent()) {
    if (CanBeActivated(w))
      return w;
  }
  return nullptr;
}

void FocusController::SetFocusedWindowImpl(
    FocusControllerChangeSource change_source,
    ServerWindow* window) {
  if (window && !CanBeFocused(window))
    return;
  ServerWindow* old_focused = GetFocusedWindow();

  DCHECK(!window || window->IsDrawn());

  // Activate the closest activatable ancestor window.
  // TODO(sad): The window to activate doesn't necessarily have to be a direct
  // ancestor (e.g. could be a transient parent).
  SetActiveWindow(GetActivatableAncestorOf(window),
                  ActivationChangeReason::FOCUS);

  FOR_EACH_OBSERVER(FocusControllerObserver, observers_,
                    OnFocusChanged(change_source, old_focused, window));

  focused_window_ = window;
  // We can currently use only a single ServerWindowDrawnTracker since focused
  // window is expected to be a direct descendant of the active window.
  if (focused_window_ && active_window_) {
    DCHECK(active_window_->Contains(focused_window_));
  }
  ServerWindow* track_window = focused_window_;
  if (!track_window)
    track_window = active_window_;
  if (track_window)
    drawn_tracker_.reset(new ServerWindowDrawnTracker(track_window, this));
  else
    drawn_tracker_.reset();
}

void FocusController::OnDrawnStateWillChange(ServerWindow* ancestor,
                                             ServerWindow* window,
                                             bool is_drawn) {
  DCHECK(!is_drawn);
  DCHECK(root_->Contains(window));
  drawn_tracker_.reset();

  auto will_be_hidden = [ancestor, window](ServerWindow* w) {
    return w != ancestor && ancestor->Contains(w) && w->Contains(window);
  };

  // If |window| is |active_window_|, then activate the next activatable window
  // that does not belong to the subtree which is getting hidden.
  if (window == active_window_) {
    WindowTreeIterator iter(root_);
    ServerWindow* activate = active_window_;
    do {
      activate = iter.GetNext(activate);
    } while (activate != active_window_ &&
             (will_be_hidden(activate) || !CanBeActivated(activate)));
    if (activate == window)
      activate = nullptr;
    SetActiveWindow(activate, ActivationChangeReason::DRAWN_STATE_CHANGED);

    // Now make sure focus is in the active window.
    ServerWindow* focus = nullptr;
    if (active_window_) {
      WindowTreeIterator iter(active_window_);
      focus = nullptr;
      do {
        focus = iter.GetNext(focus);
      } while (focus != active_window_ &&
               (will_be_hidden(focus) || !CanBeFocused(focus)));
      DCHECK(focus && CanBeFocused(focus));
    }
    SetFocusedWindowImpl(FocusControllerChangeSource::DRAWN_STATE_CHANGED,
                         focus);
    return;
  }

  // Move focus to the next focusable window in |active_window_|.
  DCHECK_EQ(focused_window_, window);
  WindowTreeIterator iter(active_window_);
  ServerWindow* focus = focused_window_;
  do {
    focus = iter.GetNext(focus);
  } while (focus != focused_window_ &&
           (will_be_hidden(focus) || !CanBeFocused(focus)));
  if (focus == window)
    focus = nullptr;
  SetFocusedWindowImpl(FocusControllerChangeSource::DRAWN_STATE_CHANGED, focus);
}

void FocusController::OnDrawnStateChanged(ServerWindow* ancestor,
                                          ServerWindow* window,
                                          bool is_drawn) {
  DCHECK(false);
}

}  // namespace ws
}  // namespace mus
