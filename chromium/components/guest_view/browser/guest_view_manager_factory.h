// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_GUEST_VIEW_BROWSER_GUEST_VIEW_MANAGER_FACTORY_H_
#define COMPONENTS_GUEST_VIEW_BROWSER_GUEST_VIEW_MANAGER_FACTORY_H_

namespace content {
class BrowserContext;
}

namespace guest_view {

class GuestViewManager;
class GuestViewManagerDelegate;

class GuestViewManagerFactory {
 public:
  virtual GuestViewManager* CreateGuestViewManager(
      content::BrowserContext* context,
      scoped_ptr<GuestViewManagerDelegate> delegate) = 0;

 protected:
  virtual ~GuestViewManagerFactory() {}
};

}  // namespace guest_view

#endif  // COMPONENTS_GUEST_VIEW_BROWSER_GUEST_VIEW_MANAGER_FACTORY_H_

