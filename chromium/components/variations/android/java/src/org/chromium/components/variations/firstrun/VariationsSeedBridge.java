// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.variations.firstrun;

import android.content.Context;
import android.preference.PreferenceManager;
import android.util.Base64;

import org.chromium.base.annotations.CalledByNative;

/**
 * VariationsSeedBridge is a class which used to pass variations first run seed that was fetched
 * before the actual Chrome first run to Chromium core. Class provides methods to store the seed
 * in SharedPreferences and to get the seed from there. To store raw seed data class serializes
 * byte[] to Base64 encoded string and decodes this string before passing to C++ side.
 */
public class VariationsSeedBridge {
    protected static final String VARIATIONS_FIRST_RUN_SEED_BASE64 = "variations_seed_base64";
    protected static final String VARIATIONS_FIRST_RUN_SEED_SIGNATURE = "variations_seed_signature";
    protected static final String VARIATIONS_FIRST_RUN_SEED_COUNTRY = "variations_seed_country";
    protected static final String VARIATIONS_FIRST_RUN_SEED_DATE = "variations_seed_date";
    protected static final String VARIATIONS_FIRST_RUN_SEED_IS_GZIP_COMPRESSED =
            "variations_seed_is_gzip_compressed";

    // This pref is used to store information about successful seed storing on the C++ side, in
    // order to not fetch the seed again.
    protected static final String VARIATIONS_FIRST_RUN_SEED_NATIVE_STORED =
            "variations_seed_native_stored";

    protected static String getVariationsFirstRunSeedPref(Context context, String prefName) {
        return PreferenceManager.getDefaultSharedPreferences(context).getString(prefName, "");
    }

    /**
     * Stores variations seed data (raw data, seed signature and country code) in SharedPreferences.
     * CalledByNative attribute is used by unit tests code to set test data.
     */
    @CalledByNative
    public static void setVariationsFirstRunSeed(Context context, byte[] rawSeed, String signature,
            String country, String date, boolean isGzipCompressed) {
        PreferenceManager.getDefaultSharedPreferences(context)
                .edit()
                .putString(VARIATIONS_FIRST_RUN_SEED_BASE64,
                        Base64.encodeToString(rawSeed, Base64.NO_WRAP))
                .putString(VARIATIONS_FIRST_RUN_SEED_SIGNATURE, signature)
                .putString(VARIATIONS_FIRST_RUN_SEED_COUNTRY, country)
                .putString(VARIATIONS_FIRST_RUN_SEED_DATE, date)
                .putBoolean(VARIATIONS_FIRST_RUN_SEED_IS_GZIP_COMPRESSED, isGzipCompressed)
                .apply();
    }

    @CalledByNative
    private static void clearFirstRunPrefs(Context context) {
        PreferenceManager.getDefaultSharedPreferences(context)
                .edit()
                .remove(VARIATIONS_FIRST_RUN_SEED_BASE64)
                .remove(VARIATIONS_FIRST_RUN_SEED_SIGNATURE)
                .remove(VARIATIONS_FIRST_RUN_SEED_COUNTRY)
                .remove(VARIATIONS_FIRST_RUN_SEED_DATE)
                .remove(VARIATIONS_FIRST_RUN_SEED_IS_GZIP_COMPRESSED)
                .apply();
    }

    /**
     * Returns the status of the variations first run fetch: was it successful or not.
     */
    public static boolean hasJavaPref(Context context) {
        return !PreferenceManager.getDefaultSharedPreferences(context)
                        .getString(VARIATIONS_FIRST_RUN_SEED_BASE64, "")
                        .isEmpty();
    }

    /**
     * Returns the status of the variations seed storing on the C++ side: was it successful or not.
     */
    public static boolean hasNativePref(Context context) {
        return PreferenceManager.getDefaultSharedPreferences(context).getBoolean(
                VARIATIONS_FIRST_RUN_SEED_NATIVE_STORED, false);
    }

    @CalledByNative
    private static void markVariationsSeedAsStored(Context context) {
        PreferenceManager.getDefaultSharedPreferences(context)
                .edit()
                .putBoolean(VARIATIONS_FIRST_RUN_SEED_NATIVE_STORED, true)
                .apply();
    }

    @CalledByNative
    private static byte[] getVariationsFirstRunSeedData(Context context) {
        return Base64.decode(
                getVariationsFirstRunSeedPref(context, VARIATIONS_FIRST_RUN_SEED_BASE64),
                Base64.NO_WRAP);
    }

    @CalledByNative
    private static String getVariationsFirstRunSeedSignature(Context context) {
        return getVariationsFirstRunSeedPref(context, VARIATIONS_FIRST_RUN_SEED_SIGNATURE);
    }

    @CalledByNative
    private static String getVariationsFirstRunSeedCountry(Context context) {
        return getVariationsFirstRunSeedPref(context, VARIATIONS_FIRST_RUN_SEED_COUNTRY);
    }

    @CalledByNative
    private static String getVariationsFirstRunSeedDate(Context context) {
        return getVariationsFirstRunSeedPref(context, VARIATIONS_FIRST_RUN_SEED_DATE);
    }

    @CalledByNative
    private static boolean getVariationsFirstRunSeedIsGzipCompressed(Context context) {
        return PreferenceManager.getDefaultSharedPreferences(context).getBoolean(
                VARIATIONS_FIRST_RUN_SEED_IS_GZIP_COMPRESSED, false);
    }
}
