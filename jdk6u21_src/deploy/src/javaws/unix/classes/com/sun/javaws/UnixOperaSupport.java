/*
 * @(#)UnixOperaSupport.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.File;
import java.lang.Exception;
import java.lang.Object;
import java.lang.String;
import java.lang.System;
import com.sun.deploy.util.Trace;


/**
 This class provides Java Web Start support for the Opera browser on Unix and
 Linux.

 @author Michael W. Romanchuk
 @version 0.1, 05/28/03
 @since 1.5
 */
public class UnixOperaSupport extends OperaSupport
{
    /**
     Checks if Opera is installed.
     <br />
     This checks the user's home directory for the presence of a <code>.opera</code>
     directory, which is created by all known versions of the Opera web browser.

     @return <code>true</code> if Opera is installed; <code>false</code>
             otherwise.
     */
    public boolean isInstalled()
    {
        return (getUserDir().exists());
    }

    /**
     Enables Opera to handle the <code>application/x-java-jnlp-file</code> mime
     type and <code>jnlp</code> file extension.

     @param jaws      the location of the Java Web Start executable.
     @param override  if <code>true</code>, then the value of <code>jaws</code>
                      is used even if there is an existing application assigned
                      to the JNLP mime type.
     */
    public void enableJnlp(File jaws, boolean override)
    {
        File opera = getUserDir();
        File user  = null;

        if (opera.exists())
        {
            try
            {
                // If this is Opera 6+ then there is a single preference
                // file for all users named "Opera6.ini" in the opera
                // directory
                user = new File(opera, OPERA_6_PREFERENCES);
                if (user.exists() == false)
                {
                    // If this is Opera 5+ then there is a single preference
                    // file for all users named "Opera.ini" in the opera
                    // directory
                    user = new File(opera, OPERA_PREFERENCES);
                }
                // no else required; Opera 6+

                enableJnlp(null, user, jaws, override);
            }
            catch (Exception e)
            {
                Trace.ignoredException(e);
            }
        }
        // no else required; Opera not installed
    }

    /**
     Construct a <code>UnixOperaSupport</code> object.
     */
    public UnixOperaSupport()
    {
        super(false);
    }

    /**
     Gets the user's home directory as a <code>File</code>.
     <br />
     This is the actual user home directory (i.e ~), not the directory returned
     by <code>Config.getUserHome()</code>.

     @return The user's home directory as a <code>File</code>.
     */
    private File getUserDir()
    {
        return (new File(System.getProperty(USER_HOME), OPERA_DIR));
    }

    /** The name of the System property that stores the user's home directory. */
    private static final String USER_HOME = "user.home";

    /** The name of the directory Opera uses to store user settings. */
    private static final String OPERA_DIR = ".opera";
}
