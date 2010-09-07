/*
 * @(#)XTextTransferHelper.java	1.6 04/06/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.Transferable;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.lang.reflect.Field;
import javax.swing.JComponent;
import javax.swing.TransferHandler;
import javax.swing.text.JTextComponent;

class XTextTransferHelper {
    private static Object transferHandlerKey = null;
    static Object getTransferHandlerKey() {
        if (transferHandlerKey == null) {
            try {
                Field field = XToolkit.getField(JComponent.class, "TRANSFER_HANDLER_KEY");
                transferHandlerKey = field.get(null);
            } catch (IllegalAccessException ex) {
                return null;
            }
        }
        return transferHandlerKey;
    }
}
