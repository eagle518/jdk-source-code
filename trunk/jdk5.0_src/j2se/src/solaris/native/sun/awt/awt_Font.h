/*
 * @(#)awt_Font.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni_util.h>

/* fieldIDs for Font fields that may be accessed from C */
struct FontIDs {
    jfieldID pData;
    jfieldID style;
    jfieldID size;
    jmethodID getPeer;
    jmethodID getFamily;
};

/* fieldIDs for MFontPeer fields that may be accessed from C */
struct MFontPeerIDs {
    jfieldID xfsname;
};

/* fieldIDs for PlatformFont fields that may be accessed from C */
struct PlatformFontIDs {
    jfieldID componentFonts;
    jfieldID fontConfig;
    jmethodID makeConvertedMultiFontString;
    jmethodID makeConvertedMultiFontChars;
};

