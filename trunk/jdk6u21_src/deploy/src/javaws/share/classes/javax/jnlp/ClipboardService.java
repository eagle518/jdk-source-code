/*
 * @(#)ClipboardService.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.awt.datatransfer.Transferable;

/**
 * <code>ClipboardService</code> provides methods for accessing
 * the shared system-wide clipboard, even for applications that
 * are running in the untrusted execution environment.
 * Implementors should warn the user of the potential security
 * risk of letting an untrusted application have access to
 * potentially confidential information stored in the clipboard,
 * or overwriting the contents of the clipboard.
 *
 * @since 1.0
 */

public interface ClipboardService {

  /**
   *  Returns a <code>Transferable</code> object representing
   *  the current contents of the clipboard.  If the clipboard
   *  currently has no contents, it returns null.
   *
   *  @return     The current <code>Transferable</code> object on
   *              the clipboard.
   */
  public Transferable getContents();

  /**
   *  Sets the current contents of the clipboard to the specified
   *  <code>Transferable</code> object.
   *
   *  @param contents     The <code>Transferable</code> object
   *                      representing clipboard content.
   */
  public void setContents(Transferable contents);

}
