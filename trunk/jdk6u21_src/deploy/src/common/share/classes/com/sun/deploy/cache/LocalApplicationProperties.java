/*
 * @(#)LocalApplicationProperties.java	1.27 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.cache;

import java.io.*;
import java.net.*;
import java.text.*;
import java.util.*;

/**
 * LocalApplicationProperties is used to track additional information
 * pertaining to a particular application. One can not directly create
 * an instance of this class, instead use one of the Cache methods
 * to obtain one.
 * <p>
 * LocalApplicationProperties defines methods to access some of the more
 * interesting information about an application, but it also allows for
 * adding arbitrary key/value pairs.
 *
 * @version 1.27 03/24/10
 */
public interface LocalApplicationProperties {

    /** Return location and version id of application. The versionID will always
     *  be NULL for an application descriptor. These values are based on the cache
     *  location of the application/extension */
    public URL getLocation();
    public String getVersionId();

    /**
     * Date this application was last accessed (run).
     */
    public void setLastAccessed(Date date);
    public Date getLastAccessed();

    /**
     * Number of times this application was launched. This will be
     * at least 1.
     */
    public int getLaunchCount();
    public void incrementLaunchCount();

    /**
     * Indicates if the user has been prompted if they want to do an install.
     * (the "install" here referes to desktop integration)
     */
    public void setAskedForInstall(boolean askedForInstall);
    public boolean getAskedForInstall();

    /**
     * Used by ExtensionInstallerService to tell the core client that a reboot is needed.
     */
    public void setRebootNeeded(boolean reboot);
    public boolean isRebootNeeded();

    /** This is used for desktop integration bits for applications, and installed/not installed
     *  for extensions
     */
    public void setLocallyInstalled(boolean installed);
    public boolean isLocallyInstalled();
    public boolean isLocallyInstalledSystem();

    /** Determines if an force update check should be performed or not. This is used
     *  for background downloading
     */
    public boolean forceUpdateCheck();
    void  setForceUpdateCheck(boolean state);

    /** Returns true/false if this is properties for an application descriptor or
     *  an extension descriptor
     */
    boolean isApplicationDescriptor();
    boolean isExtensionDescriptor();

    /*
     * Associations
     */
    public AssociationDesc [] getAssociations();
    public void addAssociation(AssociationDesc association);
    public void setAssociations(AssociationDesc  [] associations);


    /** Returns the native library & install directory for an extension */
    public String getNativeLibDirectory();
    public String getInstallDirectory();
    public void setNativeLibDirectory(String path);
    public void setInstallDirectory(String path);

	/** Returns title attribute in jnlp file */
	// fix bug 6206216
	public String getRegisteredTitle();
	public void setRegisteredTitle(String title);

    /** General key/value setters and getters */
    public void put(String key, String value);
    public String get(String key);

    /**
     * Returns an integer representation of <code>key</code>, returns 0
     * if not found, or there is an error in parsing the string.
     */
    public int getInteger(String key);

    /**
     * Returns the boolean representation of <code>key</code>. This will
     * return false if <code>key</code> is not defined.
     */
    public boolean getBoolean(String key);

    /**
     * Returns the date representation of <code>key</code>. This will
     * return null if <code>key</code> is not defined, or there is a parse
     * error.
     */
    public Date getDate(String key);

    /**
     * Saves the properties.
     */
    public void store() throws IOException;

    /**
     * If the receiver is not dirty, and the disk version has changed,
     * the receiver is reloaded.
     */
    public void refreshIfNecessary();

    /**
     * Reloads the receiver, regardless of the state of the receiver.
     */
    public void refresh();

    /**
     * Dragged applet support
     */
    public void setDraggedApplet();
    public boolean isDraggedApplet();
    public void setDocumentBase(String documentBase);
    public String getDocumentBase();
    public void setCodebase(String codebase);
    public String getCodebase();

   
}
