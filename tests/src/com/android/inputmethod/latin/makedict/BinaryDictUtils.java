/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.inputmethod.latin.makedict;

import com.android.inputmethod.latin.makedict.FormatSpec.FormatOptions;
import com.android.inputmethod.latin.makedict.FusionDictionary.DictionaryOptions;

import java.io.File;
import java.util.HashMap;

public class BinaryDictUtils {
    public static final int USE_BYTE_ARRAY = 1;
    public static final int USE_BYTE_BUFFER = 2;

    public static final String TEST_DICT_FILE_EXTENSION = ".testDict";

    public static final FormatSpec.FormatOptions VERSION2 = new FormatSpec.FormatOptions(2);
    public static final FormatSpec.FormatOptions VERSION3_WITHOUT_DYNAMIC_UPDATE =
            new FormatSpec.FormatOptions(3, false /* supportsDynamicUpdate */);
    public static final FormatSpec.FormatOptions VERSION3_WITH_DYNAMIC_UPDATE =
            new FormatSpec.FormatOptions(3, true /* supportsDynamicUpdate */);
    public static final FormatSpec.FormatOptions VERSION4_WITHOUT_DYNAMIC_UPDATE =
            new FormatSpec.FormatOptions(4, false /* supportsDynamicUpdate */);
    public static final FormatSpec.FormatOptions VERSION4_WITH_DYNAMIC_UPDATE =
            new FormatSpec.FormatOptions(4, true /* supportsDynamicUpdate */);
    public static final FormatSpec.FormatOptions VERSION4_WITH_DYNAMIC_UPDATE_AND_TIMESTAMP =
            new FormatSpec.FormatOptions(4, true /* supportsDynamicUpdate */,
                    true /* hasTimestamp */);

    public static DictionaryOptions getDictionaryOptions(final String id, final String version) {
        final DictionaryOptions options = new DictionaryOptions(new HashMap<String, String>(),
                false /* germanUmlautProcessing */, false /* frenchLigatureProcessing */);
        options.mAttributes.put("dictionary", id);
        options.mAttributes.put("version", version);
        return options;
    }

    public static File getDictFile(final String name, final String version,
            final FormatOptions formatOptions, final File directory) {
        if (formatOptions.mVersion == 2 || formatOptions.mVersion == 3) {
            return new File(directory, name + "." + version + TEST_DICT_FILE_EXTENSION);
        } else if (formatOptions.mVersion == 4) {
            return new File(directory, name + "." + version);
        } else {
            throw new RuntimeException("the format option has a wrong version : "
                    + formatOptions.mVersion);
        }
    }

    public static DictEncoder getDictEncoder(final File file, final FormatOptions formatOptions,
            final File cacheDir) {
        if (formatOptions.mVersion == FormatSpec.VERSION4) {
            return new Ver4DictEncoder(cacheDir);
        } else if (formatOptions.mVersion == 3 || formatOptions.mVersion == 2) {
            return new Ver3DictEncoder(file);
        } else {
            throw new RuntimeException("The format option has a wrong version : "
                    + formatOptions.mVersion);
        }
    }

    public static DictUpdater getDictUpdater(final File file, final FormatOptions formatOptions) {
        if (formatOptions.mVersion == FormatSpec.VERSION4) {
            return new Ver4DictUpdater(file, DictDecoder.USE_WRITABLE_BYTEBUFFER);
        } else if (formatOptions.mVersion == 3) {
            return new Ver3DictUpdater(file, DictDecoder.USE_WRITABLE_BYTEBUFFER);
        } else {
            throw new RuntimeException("The format option has a wrong version : "
                    + formatOptions.mVersion);
        }
    }
}