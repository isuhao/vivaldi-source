
// included by "extensions/browser/guest_view/web_view/web_view_guest.h" constructor

// WebViewGuest::WebViewGuest(...) :
// other variables
      has_left_mousebutton_down_(false),
      has_right_mousebutton_down_(false),
      eat_next_right_mouseup_(false),
      current_host_(NULL),
      gesture_state_(GestureStateNone),
      media_state_(TAB_MEDIA_STATE_NONE),
      is_visible_(false),
      is_fullscreen_(false),
      window_state_prior_to_fullscreen_(ui::SHOW_STATE_NORMAL),
      webcontents_was_created_as_guest_(false),


//    weak_ptr_factory_(this)
