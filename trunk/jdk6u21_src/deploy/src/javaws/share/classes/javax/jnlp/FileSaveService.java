/*
 * @(#)FileSaveService.java	1.19 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;
import java.io.IOException;
import java.io.InputStream;

/**
 * <code>FileSaveService</code> service allows the user to save a file
 * to the local file system, even for applications that
 * are running in the untrusted execution environment. The JNLP Client is the mediator
 * and is therefore responsible for providing the specific implementation
 * of this, if any.
 * <p>
 * This service provides similar functionatlity as the <i>Save as...</i>
 * functionality provided by most browsers.
 *
 * @since 1.0
 *
 * @see FileOpenService
 * @see FileContents
 */
public interface FileSaveService {
    
    /** Asks the users to save a file.
     *
     *  @param  pathHint  A hint from the application to the default directory to be
     *                    used. This might be ignored by the JNLP Client.
     *
     *  @param extensions A list of default extensions to show in the file chooser.
     *                    For example, <code>String[] { "txt", "java" }</code>.
     *                    These might be ignored by the JNLP Client.
     *
     *  @param stream     The content of the file to save along represented as an
     *                    <code>InputStream</code>
     *
     *  @param name       The suggested filename, which might be ignored by the JNLP client
     *
     *  @return A <code>FileContents</code> object for the saved file if the save was successfully, or
     *          <code>null</code> if the user canceled the request.
     *
     *  @exception <code>IOException</code> if the requested failed in any way other than the user chose
     *  not to save the file
     */
    public FileContents saveFileDialog(String pathHint, String[] extensions, InputStream stream, String name) throws IOException;

    /** Asks the users to save a file.
     *
     *  @param  pathHint  A hint from the application to the default directory to be
     *                    used. This might be ignored by the JNLP Client.
     *
     *  @param extensions A list of default extensions to show in the file chooser.
     *                    For example, <code>String[] { "txt", "java" }</code>.
     *                    These might be ignored by the JNLP Client.
     *
     *  @param contents The content of the file to save along with the suggested filename.
     *                     The suggested filename might be ignored by the JNLP Client.
     *
     *
     *
     *  @return A <code>FileContents</code> object for the saved file if the save was successfully, or
     *          <code>null</code> if the user canceled the request.
     *
     *  @exception <code>IOException</code> if the requested failed in any way other than the user chose
     *  not to save the file
     */
    public FileContents saveAsFileDialog(String pathHint, String[] extensions, FileContents contents) throws IOException;
}

