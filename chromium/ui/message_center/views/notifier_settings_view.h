// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_MESSAGE_CENTER_VIEWS_NOTIFIER_SETTINGS_VIEW_H_
#define UI_MESSAGE_CENTER_VIEWS_NOTIFIER_SETTINGS_VIEW_H_

#include <set>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "ui/message_center/message_center_export.h"
#include "ui/message_center/notifier_settings.h"
#include "ui/message_center/views/message_bubble_base.h"
#include "ui/views/controls/button/checkbox.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/menu_button_listener.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/view.h"

namespace views {
class Label;
class MenuButton;
class MenuRunner;
}

namespace message_center {
class NotifierGroupMenuModel;

// A class to show the list of notifier extensions / URL patterns and allow
// users to customize the settings.
class MESSAGE_CENTER_EXPORT NotifierSettingsView
    : public NotifierSettingsObserver,
      public views::View,
      public views::ButtonListener,
      public views::MenuButtonListener {
 public:
  explicit NotifierSettingsView(NotifierSettingsProvider* provider);
  ~NotifierSettingsView() override;

  bool IsScrollable();

  // Overridden from NotifierSettingsDelegate:
  void UpdateIconImage(const NotifierId& notifier_id,
                       const gfx::Image& icon) override;
  void NotifierGroupChanged() override;
  void NotifierEnabledChanged(const NotifierId& notifier_id,
                              bool enabled) override;

  void set_provider(NotifierSettingsProvider* new_provider) {
    provider_ = new_provider;
  }

 private:
  FRIEND_TEST_ALL_PREFIXES(NotifierSettingsViewTest, TestLearnMoreButton);

  class MESSAGE_CENTER_EXPORT NotifierButton : public views::CustomButton,
                         public views::ButtonListener {
   public:
    NotifierButton(NotifierSettingsProvider* provider,
                   Notifier* notifier,
                   views::ButtonListener* listener);
    ~NotifierButton() override;

    void UpdateIconImage(const gfx::Image& icon);
    void SetChecked(bool checked);
    bool checked() const;
    bool has_learn_more() const;
    const Notifier& notifier() const;

    void SendLearnMorePressedForTest();

   private:
    // Overridden from views::ButtonListener:
    void ButtonPressed(views::Button* button, const ui::Event& event) override;
    void GetAccessibleState(ui::AXViewState* state) override;

    bool ShouldHaveLearnMoreButton() const;
    // Helper function to reset the layout when the view has substantially
    // changed.
    void GridChanged(bool has_learn_more, bool has_icon_view);

    NotifierSettingsProvider* provider_;  // Weak.
    const scoped_ptr<Notifier> notifier_;
    // |icon_view_| is owned by us because sometimes we don't leave it
    // in the view hierarchy.
    scoped_ptr<views::ImageView> icon_view_;
    views::Label* name_view_;
    views::Checkbox* checkbox_;
    views::ImageButton* learn_more_;

    DISALLOW_COPY_AND_ASSIGN(NotifierButton);
  };

  // Given a new list of notifiers, updates the view to reflect it.
  void UpdateContentsView(const std::vector<Notifier*>& notifiers);

  // Overridden from views::View:
  void Layout() override;
  gfx::Size GetMinimumSize() const override;
  gfx::Size GetPreferredSize() const override;
  bool OnKeyPressed(const ui::KeyEvent& event) override;
  bool OnMouseWheel(const ui::MouseWheelEvent& event) override;

  // Overridden from views::ButtonListener:
  void ButtonPressed(views::Button* sender, const ui::Event& event) override;

  // Overridden from views::MenuButtonListener:
  void OnMenuButtonClicked(views::View* source,
                           const gfx::Point& point) override;

  views::ImageButton* title_arrow_;
  views::Label* title_label_;
  views::MenuButton* notifier_group_selector_;
  views::ScrollView* scroller_;
  NotifierSettingsProvider* provider_;
  std::set<NotifierButton*> buttons_;
  scoped_ptr<NotifierGroupMenuModel> notifier_group_menu_model_;
  scoped_ptr<views::MenuRunner> notifier_group_menu_runner_;

  DISALLOW_COPY_AND_ASSIGN(NotifierSettingsView);
};

}  // namespace message_center

#endif  // UI_MESSAGE_CENTER_VIEWS_NOTIFIER_SETTINGS_VIEW_H_