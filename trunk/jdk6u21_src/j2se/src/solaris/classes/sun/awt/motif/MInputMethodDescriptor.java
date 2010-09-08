/*
 * @(#)MInputMethodDescriptor.java	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.awt.motif;

import java.awt.im.spi.InputMethod;
import sun.awt.X11InputMethodDescriptor;

/**
 * Provides sufficient information about an input method
 * to enable selection and loading of that input method.
 * The input method itself is only loaded when it is actually used.
 * 
 * @since JDK1.3
 */

class MInputMethodDescriptor extends X11InputMethodDescriptor {

    /**
     * @see java.awt.im.spi.InputMethodDescriptor#createInputMethod
     */
    public InputMethod createInputMethod() throws Exception {
        return new MInputMethod();
    }
}
