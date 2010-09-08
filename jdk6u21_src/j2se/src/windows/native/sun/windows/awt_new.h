/*
 * @(#)awt_new.h	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

};

#endif /* AWT_NEW_H */
