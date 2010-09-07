/*
 * @(#)FileOpenService.java	1.17 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;
import java.io.IOException;
import java.io.InputStream;

/**
 * <code>FileOpenService</code> service allows the user to choose a file
 * from the local file system, even for applications that
 * are running in the untrusted execution environment. The JNLP Client is the mediator
 * and is therefore responsible for providing the specific implementation
 * of this, if any.
 * <p>
 * This service provides a similar function as the file input field for HTML-based
 * forms.
 *
 * @since 1.0
 *
 * @see FileSaveService
 * @see FileContents
 */
public interface FileOpenService {
    
    /** Asks the user to choose a single file. The contents of a potential
     *  selected file is returned as a <code>{@link FileContents}</code> object.
     *  The returned <code>FileContents</code> object contains the contents
     *  along with the name of the file. The full path is not returned.
     *
     *  @param  pathHint  A hint from the application to the initial directory for
     *                    the file chooser. This might be ignored by the JNLP
     *                    Client.
     *
     *  @param extensions A list of default extensions to show in the file chooser.
     *                    For example, <code>String[] { "txt", "java" }</code>.
     *                    This might be ignored by the JNLP Client.
     *
     *  @return A <code>FileContent</code> object with information about the
     *          chosen file, or <code>null</code> if the user did not choose a file.
     *
     *  @exception <code>IOException</code> if the request failed in any way other than the user did not
     *                    choose to select a file.
     */
    public FileContents openFileDialog(String pathHint, String[] extensions) throws IOException;

    /**
     * Asks the user to choose one or more files. Otherwise similar to
     * <code>{@link #openFileDialog}</code>
     */
    public FileContents [] openMultiFileDialog(String pathHint, String[] extensions) throws IOException;
}

