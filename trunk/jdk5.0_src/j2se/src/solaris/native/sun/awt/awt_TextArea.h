/*
 * @(#)awt_TextArea.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"

/* fieldIDs for TextArea fields that may be accessed from C */
static struct TextAreaIDs {
    jfieldID scrollbarVisibility;
};

/* fieldIDs for MTextAreaPeer fields that may be accessed from C */
struct MTextAreaPeerIDs {
    jfieldID firstChangeSkipped;
};
