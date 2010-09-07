/*
 * @(#)JNLPClassLoaderIf.java	1.49 07/08/31
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.JarURLConnection;
import java.util.jar.JarFile;
import java.util.ArrayList;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.CodeSource;
import java.security.CodeSigner;
import java.security.Permission;
import java.security.PermissionCollection;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.security.AppPolicy;
import com.sun.javaws.progress.ProgressListener;

/**
 * This is the interface for the JNLP classloader 
 *
 */
public interface JNLPClassLoaderIf 
{
    //----------------------------------------------------------------------
    // JNLP Applet Launcher Workaround
    //
    public void quiescenceRequested(Thread thread, boolean initiator);
    public void quiescenceCancelled(boolean initiator);


    //----------------------------------------------------------------------
    // ClassLoader interface
    //

    public URL getResource(final String name);

    //----------------------------------------------------------------------
    // URLClassLoader interface
    //

    public URL findResource(String name) ;


    //----------------------------------------------------------------------
    // JNLP interface
    //

    /** Get main JNLP file */
    public LaunchDesc getLaunchDesc() ;
    
    /*
     *  Methods for downloading parts of the application
     */
    public void downloadResource(URL location, String version, 
	ProgressListener dp, boolean isCacheOk)
        throws JNLPException, IOException ;
    
    public void downloadParts(String[] parts, 
	ProgressListener dp, boolean isCacheOk)
        throws JNLPException, IOException ;
    
    public void downloadExtensionParts(URL location, String version, 
	String[] parts, ProgressListener dp, boolean isCacheOk)
        throws JNLPException, IOException ;
    
    public void downloadEager(ProgressListener dp, boolean 
	isCacheOk) throws JNLPException, IOException ;
    
    /** Returns JARDesc if the JAR file is part of the JNLP codebase */
    public JARDesc getJarDescFromURL(URL url) ;
    
    /** Returns the default security model for this application */
    public int getDefaultSecurityModel() ;
    
    public JarFile getJarFile(URL url) throws IOException ;

    /* addResource(URL resource, String version, String id)
     *
     *     When the DownloadService.loadResource(URL, String) is called, 
     * the given item no longer needs to be listed in the jnlp file, 
     * providing it is at least from the same codebase.
     *     In such a case, this method is used to add the resource to the 
     * JNLP2ClassLoader, so it can later be loaded.
     */
    public void addResource(URL resource, String version, String id) ;

    //----------------------------------------------------------------------
    // JNLP Service interface
    //

    public javax.jnlp.BasicService getBasicService();
    public javax.jnlp.FileOpenService getFileOpenService();
    public javax.jnlp.FileSaveService getFileSaveService();
    public javax.jnlp.ExtensionInstallerService getExtensionInstallerService();
    public javax.jnlp.DownloadService getDownloadService();
    public javax.jnlp.ClipboardService getClipboardService();
    public javax.jnlp.PrintService getPrintService();
    public javax.jnlp.PersistenceService getPersistenceService();
    public javax.jnlp.ExtendedService getExtendedService();
    public javax.jnlp.SingleInstanceService getSingleInstanceService();

    public javax.jnlp.IntegrationService getIntegrationService();
    public javax.jnlp.DownloadService2 getDownloadService2();

}


