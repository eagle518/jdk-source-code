/*
 * @(#)ExtensionInstallerService.java	1.20 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;
import java.net.URL;

/**
 * The <code>ExtensionInstallerService</code> is used by an extension installer
 * to communicate with the JNLP Client. It provides the following type of
 * functionality:
 * <ul>
 * <li> Access to prefered installation location, and other information about
 *      the JNLP Client
 * <li> Manipulation of the JNLP Client's download screen
 * <li> Methods for updating the JNLP Client with the installed code
 * </ul>
 * <p>
 * The normal sequence of events for an installer is:
 * <ol><li>Get service using <code>ServiceManager.lookup("javax.jnlp.ExtensionInstallerService")</code>.
 * <li>Update status, heading, and progress as install progresses
 * (<code>setStatus</code>, <code>setHeading</code> and
 * <code>updateProgress</code>).
 * <li>Invoke either <code>setJREInfo</code> or <code>setNativeLibraryInfo</code> depending
 *     on if a JRE or a library is installed
 * <li>If successful invoke <code>installSucceeded</code>, otherwise invoke <code>installFailed</code>.
 * </ol>
 *
 * @since 1.0
 */
public interface ExtensionInstallerService {
    
    /**
     * Returns the directory where the installer is recommended to
     * install the extension in. It is not required that the installer
     * install in this directory, this is merely a suggested path.
     */
    public String getInstallPath();
    
    /**
     * Returns the version of the extension being installed
     */
    public String getExtensionVersion();
    
    /**
     * Returns the location of the extension being installed
     */
    public URL getExtensionLocation();
    
    /**
     * Hides the progress bar. Any subsequent calls to
     * <code>updateProgress</code> will force it to be visible.
     */
    public void hideProgressBar();
    
    /**
     * Hides the status window. You should only invoke this if you are
     * going to provide your own feedback to the user as to the progress
     * of the install.
     */
    public void hideStatusWindow();
    
    /**
     * Updates the status of the installer process.
     */
    public void setHeading(String heading);
    
    
    /**
     * Updates the status of the installer process.
     */
    public void setStatus(String status);
    
    /**
     * Updates the progress bar.
     *
     * @param value progress bar value - should be between 0 and 100.
     */
    public void updateProgress(int value);
    
    /**
     * Installers should invoke this upon a succesful installation of
     * the extension. This will cause the JNLP Client to regain control
     * and continue its normal operation.
     *
     * @param needsReboot  If true, a reboot is needed
     */
    public void installSucceeded(boolean needsReboot);
    
    /**
     * This should be invoked if the install fails. The JNLP Client will continue
     * its operation, and inform the user that the install has failed.
     */
    public void installFailed();
    
    /**
     * Informs the JNLP Client of the path to the executable for the JRE, if
     * this is an installer for a JRE, and about platform-version this JRE
     * implements.
     */
    public void setJREInfo(String platformVersion, String jrePath);
    
    /**
     * Informs the JNLP Client of a directory where it should search for
     * native libraries.
     */
    public void setNativeLibraryInfo(String path);
    
    /**
     * Returns the path to the executable for the given JRE. This method can be used
     * by extensions that needs to find information in a given JRE, or enhance a given
     * JRE.
     *
     *  @param url      product location of the JRE
     *  @param version  product version of the JRE
     *
     *  @return The path to the executable for the given JRE, or <code>null</code> if the
     *  JRE is not installed.
     */
    public String getInstalledJRE(URL url, String version);
}



