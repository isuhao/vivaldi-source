// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SYNC_IOS_CHROME_PROFILE_SYNC_TEST_UTIL_H_
#define IOS_CHROME_BROWSER_SYNC_IOS_CHROME_PROFILE_SYNC_TEST_UTIL_H_

#include "base/memory/scoped_ptr.h"
#include "components/browser_sync/browser/profile_sync_service.h"

namespace ios {
class ChromeBrowserState;
}

namespace sync_driver {
class SyncClient;
}

namespace web {
class BrowserState;
}

// Helper method for constructing ProfileSyncService mocks. If |sync_client|
// is null, a fresh one is created.
ProfileSyncService::InitParams CreateProfileSyncServiceParamsForTest(
    scoped_ptr<sync_driver::SyncClient> sync_client,
    ios::ChromeBrowserState* browser_state);

// Helper routine to be used in conjunction with
// BrowserStateKeyedServiceFactory::SetTestingFactory().
scoped_ptr<KeyedService> BuildMockProfileSyncService(
    web::BrowserState* context);

#endif  // IOS_CHROME_BROWSER_SYNC_IOS_CHROME_PROFILE_SYNC_TEST_UTIL_H_
