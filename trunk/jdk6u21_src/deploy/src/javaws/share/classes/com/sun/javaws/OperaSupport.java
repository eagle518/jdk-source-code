/*
 * @(#)OperaSupport.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.Float;
import java.lang.NumberFormatException;
import java.lang.Object;
import java.lang.String;
import java.text.MessageFormat;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


/**
 This class provides Java Web Start support for the Opera browser.

 @author Michael W. Romanchuk
 @version 0.1, 05/28/03
 @since 1.5
 */
public abstract class OperaSupport extends Object
{
    /**
     Checks if Opera is installed.
     <br />
     This check is system dependent.

     @return <code>true</code> if Opera is installed; <code>false</code>
             otherwise.
     */
    public abstract boolean isInstalled();

    /**
     Enables Opera to handle the <code>application/x-java-jnlp-file</code> mime
     type and <code>jnlp</code> file extension.

     @param jaws      the location of the Java Web Start executable.
     @param override  if <code>true</code>, then the value of <code>jaws</code>
                      is used even if there is an existing application assigned
                      to the JNLP mime type.
     */
    public abstract void enableJnlp(File jaws, boolean override);

    /**
     Enables Java Web Start in the given preferences file.
     <br />
     This involves adding/updating the following sections as follows:
     <code><pre>
         [File Types Section Info]
         Version=<i>&lt;version&gt;</i>

         [File Types]
         application/x-java-jnlp-file=3,<i>&lt;path&gt;</i>,,,jnlp,|
     <i>
         [File Types Extension]
         application/x-java-jnlp-file=,0
     </i>
     </pre></code>
     <br />
     Where <code><i>&lt;version&gt;</i></code> is 1 for Opera releases prior to
     and including version 3, and 2 for all other known versions.
     <code><i>&lt;path&gt;</i></code> is the fully qualified path to the Java
     Web Start executable.  Also, note that the <code>File Types Extension</code>
     section is not used for Opera releases prior to and including version 3.

     @param prefs     the preferences file to update.  If <code>null</code>,
                      then <code>file</code> is used to get the preferences.
     @param file      the location of the preferences file.
     @param jaws      the location of the Java Web Start executable.
     @param override  if <code>true</code>, then the value of <code>jaws</code>
                      is used even if there is an existing application assigned
                      to the JNLP mime type.
     */
    protected void enableJnlp(OperaPreferences prefs,
                              File             file,
                              File             jaws,
                              boolean          override) throws IOException
    {
        if (prefs == null)
        {
            prefs = getPreferences(file);
        }
        // no else required; already have prefs to update

        if (prefs != null)
        {
            float  version    = OPERA_2_PREFERENCE_VERSION;
            String versionStr = prefs.get(INSTALL_SECTION, VERSION_KEY);

            if (versionStr != null)
            {
                try
                {
                    // figure out what version of the File Types to use
                    version = Float.parseFloat(versionStr.trim());
                }
                catch (NumberFormatException nfe)
                {
                    Trace.println("Unable to determine Opera version from the preference file; assuming " +
                                  OPERA_2_PREFERENCE_VERSION + " or higher.",
                                  TraceLevel.BASIC);
                }
            }
            // no else required; after version 7+ the version key is stored in
            // the system level file, so if it's missing here this file has to
            // be using OPERA_2_PREFERENCE_VERSION

            // make sure the File Types Section Info is set
            if (version < OPERA_2_PREFERENCE_VERSION)
            {
                // less than version 5.0, the File Types Section Info always uses
                // Version=1
                prefs.put(FILE_TYPES_SECTION_INFO, FILE_TYPES_VERSION_KEY, "1");
            }
            else
            {
                // after version 5.0 and upto at least version 7.11, the File Types
                // Section Info uses Version=2, but this could change later, so only
                // set it to 2 if it isn't there at all
                if (prefs.containsKey(FILE_TYPES_SECTION_INFO, FILE_TYPES_VERSION_KEY) == false)
                {
                    if (version > LAST_TESTED_OPERA_PREFERENCE_VERSION)
                    {
                        Trace.println("Setting '[" + FILE_TYPES_SECTION_INFO + "]" +
                                      FILE_TYPES_VERSION_KEY + "=2' in the Opera preference file.",
                                      TraceLevel.BASIC);
                    }
                    // no else required; don't bother to trace if this is known to
                    // be a valid setting

                    prefs.put(FILE_TYPES_SECTION_INFO, FILE_TYPES_VERSION_KEY, "2");
                }
                // no else required; what's there is good
            }
            // make sure the File Types Section Info is set

            // set the JNLP mime-type/extension handler
            if ((override == true) ||
                (prefs.containsKey(FILE_TYPES, FILE_TYPES_KEY) == false))
            {
                Object [] params = { null, null };

                if ((version < OPERA_2_PREFERENCE_VERSION) ||
                    (useDefault == false))
                {
                    // less than version 5.0, the File Types must include the path
                    // to the Java Web Start executable.
                    params[0] = EXPLICIT_PATH;
                    try
                    {
                        params[1] = jaws.getCanonicalPath();
                    }
                    catch (IOException ioe)
                    {
                        params[1] = jaws.getAbsolutePath();
                    }
                }
                else
                {
                    // after version 5.0 and upto at least version 7.11, the File Types
                    // can be set to use whatever the default extension handler is.
                    params[0] = IMPLICIT_PATH;
                    params[1] = "";
                }

                prefs.put(FILE_TYPES,
                          FILE_TYPES_KEY,
                          MessageFormat.format(FILE_TYPES_VALUE, params));
            }
            // no else required; already set and override not invoked

            // make sure the File Types Section Info is set
            if (version >= OPERA_2_PREFERENCE_VERSION)
            {
                if (prefs.containsKey(FILE_TYPES_EXTENSION, FILE_TYPES_EXTENSION_KEY) == false)
                {
                    prefs.put(FILE_TYPES_EXTENSION,
                              FILE_TYPES_EXTENSION_KEY,
                              FILE_TYPES_EXTENSION_VALUE);
                }
                // no else required; already set and don't ever want to override
                // this user setting, since it determines where files are saved when
                // user saves target instead of opening it
            }
            // no else required; File Types Extensions aren't used prior to
            // version 5

            prefs.store(new FileOutputStream(file));
        }
        // no else required; no preferences to update, and tracing messages
        // already written by getPreferences()
    }

    /**
     Gets an <code>OperaPreferences</code> for the given file.
     <br />
     If the given file exists, and the current user has read/write access to
     the file, an <code>OperaPreferences</code> object is created and loaded
     with the files information; otherwise <code>null</code> is returned.

     @param file  the file containing Opera preferences.

     @return An <code>OperaPreferences</code> object loaded with the files
             information; otherwise <code>null</code>.
     */
    protected OperaPreferences getPreferences(File file) throws IOException
    {
        OperaPreferences result = null;

        if (file.exists())
        {
            if (file.canRead())
            {
                if (file.canWrite())
                {
                    result = new OperaPreferences();
                    result.load(new FileInputStream(file));
                }
                else
                {
                    Trace.println("No write access to the Opera preference file (" +
                                  file.getAbsolutePath() + ").",
                                  TraceLevel.BASIC);
                }
            }
            else
            {
                Trace.println("No read access to the Opera preference file (" +
                              file.getAbsolutePath() + ").",
                              TraceLevel.BASIC);
            }
        }
        else
        {
            Trace.println("The Opera preference file (" + file.getAbsolutePath() +
                          ") does not exist.",
                          TraceLevel.BASIC);
        }

        return (result);
    }

    /**
     Constructs an <code>OperaSupport</code> support object.

     @param useDefault  if <code>true</code> then Opera will be configured to
                        use the default installation of Java Web Start, which
                        may not be the one that is currently being setup by
                        the application manager.
     */
    protected OperaSupport(boolean useDefault)
    {
        this.useDefault = useDefault;
    }

    /** The name of the Opera preferences file in version prior to 6.0 */
    protected static final String OPERA_PREFERENCES = "opera.ini";

    /**
     The name of the Opera preferences file in version after and including 6.0
     */
    protected static final String OPERA_6_PREFERENCES = "opera6.ini";

    /**
     If <code>useDefault</code> is <code>true</code> then Opera will be
     configured to use the default installation of Java Web Start, which may
     not be the one that is currently being setup by the application manager.
     */
    protected boolean useDefault;

    /**
     The name of the section in the preferences file where a key can be found
     that indicates which version of Opera is installed.
     */
    private static final String INSTALL_SECTION = "INSTALL";

    /**
     The name of the key that indicates which version of Opera is installed.
     */
    private static final String VERSION_KEY = "OVER";

    /**
     The first version of Opera known to use version 2 of the preference file
     format.
     */
    private static final float OPERA_2_PREFERENCE_VERSION = 5.0f;

    /** The last version of Opera that this code was tested with. */
    private static final float LAST_TESTED_OPERA_PREFERENCE_VERSION = 7.11f;

    /**
     The name of one of the section in the preferences file that must be added
     to enable JaWS support.
     */
    private static final String FILE_TYPES_SECTION_INFO = "File Types Section Info";

    /**
     The name of the key in the File Types Section Info section of the preferences
     file that must be added to enable JaWS support.
     */
    private static final String FILE_TYPES_VERSION_KEY = "Version";

    /**
     The name of one of the section in the preferences file that must be added
     to enable JaWS support.
     */
    private static final String FILE_TYPES = "File Types";

    /**
     The name of the key in the File Types section of the preferences file that
     must be added to enable JaWS support.
     */
    private static final String FILE_TYPES_KEY = "application/x-java-jnlp-file";

    /** A MessageFormat string used to enable JaWS support. */
    private static final String FILE_TYPES_VALUE = "{0},{1},,,jnlp,|";

    /**
     The substitution parameter used to enable an explicit path to the JaWS
     executable.
     */
    private static final String EXPLICIT_PATH = "3";

    /**
     The substitution parameter used to enable implicit path lookup of the JaWS
     executable.
     */
    private static final String IMPLICIT_PATH = "4";

    /**
     The name of one of the section in the preferences file that must be added
     to enable JaWS support.
     */
    private static final String FILE_TYPES_EXTENSION = "File Types Extension";

    /**
     The name of the key in the File Types Extensions section of the preferences
     file that must be added to enable JaWS support.
     */
    private static final String FILE_TYPES_EXTENSION_KEY = "application/x-java-jnlp-file";

    /**
     The value of the key in the File Types Extensions section of the preferences
     file that must be added to enable JaWS support.
     */
    private static final String FILE_TYPES_EXTENSION_VALUE = ",0";
}
