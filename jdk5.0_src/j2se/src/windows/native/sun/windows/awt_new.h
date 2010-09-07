/*
 * @(#)awt_new.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_NEW_H
#define AWT_NEW_H

#include "awt.h"


// This class is used for establishing and implementing an operator new/
// malloc out of memory handler. The handler attempts to correct the
// out of memory condition by initiating a Java GC.
class NewHandler {
public:
    static void init();

private:
    // Don't construct instances of this class.
    NewHandler();

    static int handler(size_t);

    static jclass runtime;
    static jmethodID getRuntimeID;
    static jmethodID gcID;
    static jmethodID totalMemoryID;
};

#endif /* AWT_NEW_H */
