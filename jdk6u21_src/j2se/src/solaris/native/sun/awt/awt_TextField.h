/*
 * @(#)awt_TextField.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
