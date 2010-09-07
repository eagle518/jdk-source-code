/*
 * @(#)SystemUtil.java	1.9 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.security.*;
import com.sun.deploy.config.Config;
import sun.plugin2.os.windows.*;

/** System-level helper utilities. */

public class SystemUtil {
    // This is needed for bootstrapping -- do not remove it
    private static String javaHome;
    public static String getJavaHome() {
        if (javaHome == null) {
            javaHome = getSystemProperty("java.home");
        }
        return javaHome;
    }

    public static String getSystemProperty(final String property) {
        return (String) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return System.getProperty(property);
                }
            });
    }

    /** Indicates whether the given module is being debugged -- via
        specification of the -Dsun.plugin2.debug.[module] system
        property. */
    public static boolean debug(String module) {
        return (getSystemProperty("sun.plugin2.debug." + module) != null);
    }

    private static volatile boolean getenvSupported = true;
    /** Wrapper for System.getenv(), which doesn't work on platforms
        earlier than JDK 5 */
    public static String getenv(final String variableName) {
        if (getenvSupported) {
            try {
                return (String) AccessController.doPrivileged(new PrivilegedAction() {
			public Object run() {
			    return System.getenv(variableName);
			}
		    });
	    } catch (Error e) {
                getenvSupported = false;
            }
        }
        return null;
    }

    // Kinds of operating systems we know about
    public static final int WINDOWS = 1;
    public static final int UNIX    = 2;
    public static final int MACOSX  = 3;
    private static int osType;
    private static boolean isVista;

    private static void computeIsVista() {
        // Need this to future-proof this code
        OSVERSIONINFOA info = OSVERSIONINFOA.create();
        info.dwOSVersionInfoSize(OSVERSIONINFOA.size());
        if (Windows.GetVersionExA(info)) {
            isVista = (info.dwPlatformId() == Windows.VER_PLATFORM_WIN32_NT &&
                       info.dwMajorVersion() >= 6);
        }
    }

    /** Returns the type of OS we're running on. */
    public static int getOSType() {
        if (osType == 0) {
            String osName = getSystemProperty("os.name").toLowerCase();
            if (osName.startsWith("windows")) {
                osType = WINDOWS;
                computeIsVista();
            } else if (osName.startsWith("mac os x")) {
                osType = MACOSX;
            } else {
                osType = UNIX;
            }
        }
        return osType;
    }

    /** Indicates whether we're running on Windows Vista -- currently
        some alternate code paths are needed at the transport layer
        for this OS. */
    public static boolean isWindowsVista() {
        getOSType();
        return isVista;
    }

    /** Formats a native library name according to the platform
        conventions. */
    public static String formatNativeLibraryName(String libName) {
        switch (getOSType()) {
            case WINDOWS:
                return libName + ".dll";
            case UNIX:
                return "lib" + libName + ".so";
            case MACOSX:
                return "lib" + libName + ".jnilib";
            default:
                throw new InternalError("Unknown OS type");
        }
    }

    /** Formats an executable name according to the platform
        conventions. */
    public static String formatExecutableName(String executableName) {
        if (getOSType() == WINDOWS) {
            return executableName + ".exe";
        }
        return executableName;
    }
}

