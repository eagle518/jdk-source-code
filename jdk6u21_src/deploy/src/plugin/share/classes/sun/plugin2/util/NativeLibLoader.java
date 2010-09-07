/*
 * @(#)NativeLibLoader.java	1.9 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.io.*;

/** This class is responsible for loading the native libraries which
    the Java code of this version of the Java Plug-In relies on. <P>

    The difficulty here is that we need to consider both the Java code
    and the native code for the new plug-in as separate from the rest
    of the JRE. They both have to execute on and be loadable on
    multiple versions of the underlying Java platform. <P>

    The jar files (deploy.jar, plugin.jar) are not a problem; we just
    append them to the boot classpath of the underlying JVM. <P>

    The native code is an issue. The boot loader can only load
    libraries via System.loadLibrary() from the sun.boot.library.path
    system property. There is also a further restriction on all
    existing JREs out there that sun.boot.library.path can not contain
    multiple directories separated by File.pathSeparator. <P>

    The underlying JRE we are executing must have
    sun.boot.library.path pointing to its internal library directory;
    we can not change that. We can also not assume that the native
    libraries (i.e., "jp2native") we need are present in the
    underlying JRE. Even if they are there, they might be an older
    version not compatible with the current plugin.jar we are running
    (assumed from the "current" version of the JRE). <P>

    Fortunately, we can use System.load() instead of
    System.loadLibrary() to work around this issue, and load the
    native code we need by pointing directly at the DLLs / .so's /
    .jnilibs. We load the native code out of the same JRE from which
    the new plugin's classes (in plugin.jar / deploy.jar) came.
*/

public class NativeLibLoader {
    private NativeLibLoader() {}

    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    public static synchronized void load(String[] libs) {
        if (libs == null || libs.length == 0) {
            throw new InternalError("Wrong library name passed to load method");
        }

        // OK, now we know which libraries we need to load. We're
        // going to figure out how to find them by looking at a
        // combination of our current java.home,
        // sun.boot.library.path, and sun.boot.class.path. We assume
        // the structure of sun.boot.library.path hasn't changed
        // substantially between JRE versions.

        String[] bootClassPath =
            SystemUtil.getSystemProperty("sun.boot.class.path").split(File.pathSeparator);
        String[] possibleSubdirs = getPossibleSubdirs();
        if (!tryLoadingFromJRE(libs, possibleSubdirs, findSuffix(bootClassPath, "plugin.jar")) &&
            !tryLoadingFromJRE(libs, possibleSubdirs, findSuffix(bootClassPath, "deploy.jar")) &&
            !tryLoadingFromJRE(libs, possibleSubdirs, findSuffix(bootClassPath, "javaws.jar"))) {
            // We are in trouble; weren't able to deduce the location
            // of the native libraries from the location of plugin.jar
            // or deploy.jar (though they should be coming from the
            // same place)

            // Not sure what to do here. For now, raise an error.
            // Should probably at least try a couple of additional
            // locations (such as in the current JRE, though it's not
            // guaranteed to work). (FIXME)
            throw new InternalError("Unable to find plugin native libraries");
        }
    }

    // Given the given path to a jar file in a given JRE, try to find
    // and load the native libraries we need
    private static boolean tryLoadingFromJRE(String[] libs,
                                             String[] possibleSubdirs,
                                             String pathToJarInJRE) {
        File dir = new File(pathToJarInJRE).getParentFile();
        File dirParent = dir.getParentFile();
        File dirGrandParent = null;
        if (dirParent != null) {
            dirGrandParent = dirParent.getParentFile();
        }
        boolean foundAll = true;
        for (int i = 0; i < libs.length; i++) {
            boolean gotCurrent = false;
            for (int j = 0; j < possibleSubdirs.length && !gotCurrent; j++) {
                gotCurrent = tryLoading(libs[i], dir, possibleSubdirs[j]);
                if (!gotCurrent) {
                    gotCurrent = tryLoading(libs[i], dirParent, possibleSubdirs[j]);
                }
                if (!gotCurrent) {
                    gotCurrent = tryLoading(libs[i], dirGrandParent, possibleSubdirs[j]);
                }
            }
            if (!gotCurrent) {
                foundAll = false;
            }
        }
        // We need to find all of these libraries in order for things
        // to work
        return foundAll;
    }

    private static String[] getPossibleSubdirs() {
        String osName = SystemUtil.getSystemProperty("os.name").toLowerCase();
        if (osName.startsWith("mac os x")) {
            // We need a different search algorithm on Mac OS X, where
            // the directory layout of the JRE is significantly
            // different than on other OSs.
            //
            // For the time being, while the .jnilibs live alongside
            // the .jars in the new plug-in's bundle, we can get away
            // with a very simple answer.
            //
            // Note that we also search the MacOS directory because we
            // load the same bundle loaded by the web browser for the
            // implementation of our native methods. Severe
            // difficulties were encountered attempting to split off
            // portions of the native code into a separate .jnilib.
            return new String[] { null, "../MacOS" };
        } else {
            String bootLibraryPath = SystemUtil.getSystemProperty("sun.boot.library.path");
            // Do not use Config.getJREHome() for this -- it breaks bootstrapping
            String javaHome = SystemUtil.getJavaHome();
            if (!bootLibraryPath.startsWith(javaHome)) {
                // FIXME: I'm not sure if this can ever happen, but we can
                // probably do better than this if it does
                throw new InternalError("sun.boot.library.path (\"" +
                                        bootLibraryPath +
                                        "\") did not start with java.home (\"" +
                                        javaHome + "\")");
            }
            String bootSubdir = bootLibraryPath.substring(javaHome.length());
            if (bootSubdir.startsWith(File.separator)) {
                bootSubdir = bootSubdir.substring(1);
            }
            // This is the same directory, only with "jre" prepended to
            // try to account for JRE / JDK layout differences
            String bootJreSubdir = "jre" + File.separator + bootSubdir;

            // This is a workaround for the fact that we currently need to
            // ship the old and new plug-ins side by side, and that we
            // can't have the new plug-in's Firefox DLL in the same
            // directory as the old one's on Windows
            String newPluginBootSubdir = bootSubdir + File.separator + "new_plugin";
            String newPluginBootJreSubdir = bootJreSubdir + File.separator + "new_plugin";
            return new String[] { bootSubdir, bootJreSubdir, newPluginBootSubdir, newPluginBootJreSubdir };
        }
    }

    private static boolean tryLoading(String libName,
                                      File dir,
                                      String baseSubdir) {
        if (dir == null)
            return false;
        File tmpDir = dir;
        if (baseSubdir != null) {
            tmpDir = new File(dir, baseSubdir);
        }
        File file = new File(tmpDir, SystemUtil.formatNativeLibraryName(libName));
        if (!file.exists()) {
            // Query system property rather than asking SystemUtil for
            // bootstrapping reasons
            String osName = SystemUtil.getSystemProperty("os.name").toLowerCase();
            if (osName.startsWith("mac os x")) {
                // Try again without the suffix to load the bundle as
                // a native library
                file = new File(tmpDir, libName);
            }
        }

        if (!file.exists()) {
            if (DEBUG) {
                System.err.println("NativeLibLoader: " + file.getAbsolutePath() + " doesn't exist");
            }
            return false;
        }

        if (DEBUG) {
            System.err.println("NativeLibLoader: trying to load " + file.getAbsolutePath());
        }

        System.load(file.getAbsolutePath());

        if (DEBUG) {
            System.err.println("  (Succeeded)");
        }

        return true;
    }

    private static String findSuffix(String[] strs, String str) {
        for (int i = 0; i < strs.length; i++) {
            if (strs[i].endsWith(str))
                return strs[i];
        }
        return null;
    }
}
