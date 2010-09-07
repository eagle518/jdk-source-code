/*
 * @(#)PrintService.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.awt.print.Pageable;
import java.awt.print.Printable;
import java.awt.print.PageFormat;

/**
 * <code>PrintService</code> provides methods for access to printing
 * functions, even for applications that are running in the untrusted
 * execution environment.  Using this service, an application can
 * submit a print job to the JNLP client.  The client can then display
 * this request to the user, and if accepted, queue the request to the
 * printer.
 *
 * @since 1.0
 */

public interface PrintService {

  /**
   * Creates a new PageFormat instance and sets it to the default
   * size and orientation.
   *
   * @return        a <code>PageFormat</code> set to the default
   *                size and orientation.
   */
  public PageFormat getDefaultPage();

  /**
   * Displays a dialog that allows modification of a
   * <code>PageFormat</code> instance.  The <code>page</code>
   * argument is used to initialize controls in the page setup dialog.  
   * If the user cancels the dialog then this method returns the
   * original <code>page</code> object unmodified.  If the user
   * okays the dialog then this method returns a new
   * <code>PageFormat</code> object with the indicated changes.  In
   * either case, the original <code>page</code> object is not
   * modified.
   *
   * @param page           the default <code>PageFormat</code> presented
   *                       to the user for modification.
   *
   * @return               the original <code>page</code> object if the
   *                       dialog is cancelled; a new
   *                       <code>PageFormat</code> object containing
   *                       the format indicated by the user if the
   *                       dialog is acknowledged.
   */
  public PageFormat showPageFormatDialog(PageFormat page);

  /**
   * Prints a document using the given <code>Pageable</code> object
   *
   * @param document       the pages to be printed.  It can not be null.
   *
   * @return               <code>true</code> if printing was successfull,
   *                       <code>false</code> otherwise.
   */
  public boolean print(Pageable document);

  /**
   * Prints a document using the given <code>Printable</code> object
   *
   * @param painter        the <code>Printable</code> called to render
   *                       each page of the document.
   *
   * @return               <code>true</code> if printing was successfull,
   *                       <code>false</code> otherwise.
   */
  public boolean print(Printable painter);

}
