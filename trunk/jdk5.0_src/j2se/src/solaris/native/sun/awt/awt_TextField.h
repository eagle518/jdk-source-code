/*
 * @(#)awt_TextField.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"

/* fieldIDs for TextField fields that may be accessed from C */
static struct TextFieldIDs {
    jfieldID echoChar;
};

/* fieldIDs for MTextFieldPeer fields that may be accessed from C */
struct MTextFieldPeerIDs {
    jfieldID firstChangeSkipped;
};
