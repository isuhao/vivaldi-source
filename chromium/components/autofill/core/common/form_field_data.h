// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_FORM_FIELD_DATA_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_FORM_FIELD_DATA_H_

#include <stddef.h>

#include <vector>

#include "base/i18n/rtl.h"
#include "base/strings/string16.h"

namespace base {
class Pickle;
class PickleIterator;
}

namespace autofill {

// Stores information about a field in a form.
struct FormFieldData {
  // Copied to components/autofill/ios/browser/resources/autofill_controller.js.
  enum RoleAttribute {
    // "presentation"
    ROLE_ATTRIBUTE_PRESENTATION,
    // Anything else.
    ROLE_ATTRIBUTE_OTHER,
  };

  FormFieldData();
  ~FormFieldData();

  // Returns true if two form fields are the same, not counting the value.
  bool SameFieldAs(const FormFieldData& field) const;

  // Comparison operator exposed for STL map. Uses label, then name to sort.
  bool operator<(const FormFieldData& field) const;

  // If you add more, be sure to update the comparison operator, SameFieldAs,
  // serializing functions (in the .cc file) and the constructor.
  base::string16 label;
  base::string16 name;
  base::string16 value;
  std::string form_control_type;
  std::string autocomplete_attribute;
  size_t max_length;
  bool is_autofilled;
  bool is_checked;
  bool is_checkable;
  bool is_focusable;
  bool should_autocomplete;
  RoleAttribute role;
  base::i18n::TextDirection text_direction;

  // For the HTML snippet |<option value="US">United States</option>|, the
  // value is "US" and the contents are "United States".
  std::vector<base::string16> option_values;
  std::vector<base::string16> option_contents;
};

// Serialize and deserialize FormFieldData. These are used when FormData objects
// are serialized and deserialized.
void SerializeFormFieldData(const FormFieldData& form_field_data,
                            base::Pickle* serialized);
bool DeserializeFormFieldData(base::PickleIterator* pickle_iterator,
                              FormFieldData* form_field_data);

// So we can compare FormFieldDatas with EXPECT_EQ().
std::ostream& operator<<(std::ostream& os, const FormFieldData& field);

// Prefer to use this macro in place of |EXPECT_EQ()| for comparing
// |FormFieldData|s in test code.
#define EXPECT_FORM_FIELD_DATA_EQUALS(expected, actual) \
  do { \
    EXPECT_EQ(expected.label, actual.label); \
    EXPECT_EQ(expected.name, actual.name); \
    EXPECT_EQ(expected.value, actual.value); \
    EXPECT_EQ(expected.form_control_type, actual.form_control_type); \
    EXPECT_EQ(expected.autocomplete_attribute, actual.autocomplete_attribute); \
    EXPECT_EQ(expected.max_length, actual.max_length); \
    EXPECT_EQ(expected.is_autofilled, actual.is_autofilled); \
    EXPECT_EQ(expected.is_checked, actual.is_checked); \
    EXPECT_EQ(expected.is_checkable, actual.is_checkable); \
  } while (0)

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_FORM_FIELD_DATA_H_
