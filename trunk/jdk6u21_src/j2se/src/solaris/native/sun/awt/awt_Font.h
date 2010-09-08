/*
 * @(#)awt_Font.h	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

