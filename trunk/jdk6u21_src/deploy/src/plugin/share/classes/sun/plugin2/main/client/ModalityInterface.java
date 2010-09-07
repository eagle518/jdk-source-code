/*
 * @(#)ModalityInterface.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.awt.Dialog;

/** Provides hooks to detect when a modal dialog is popped up or
    dismissed, abstracting away certain AWT functionality that isn't
    available in all JDK versions. */

public interface ModalityInterface {
    public void modalityPushed(Dialog source);
    public void modalityPopped(Dialog source);
}
