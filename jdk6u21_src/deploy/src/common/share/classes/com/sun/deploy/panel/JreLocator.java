/*
 * @(#)JreLocator.java	1.30 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.io.*;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;

/**
 * Utility class to search for JREs.
 *
 * @version 1.30 03/24/10
 */
public class JreLocator {
    public static final int DEFAULT_TIMEOUT = 15000;

    private static final String PRODUCT_ID = "productVersion=";
    private static final String PLATFORM_ID = "platformVersion=";


    public static JREInfo getVersion(File javaPath) {
        return getVersion(javaPath, DEFAULT_TIMEOUT);
    }

    /**
     * Returns the version for the JVM at <code>path</code>.
     * This will return null if there is problem with executing
     * the path. This gives the child process at most <code>msTimeout</code>
     * mili-seconds to execute before assuming it is bogus.
     */
    public static JREInfo getVersion(File javaPath, int msTimeout) {
        // Execute the process

	String deployJar = Config.getJavaHome() + File.separator + "lib" +
				File.separator + "deploy.jar";
        String[] result = execute(new String[]
                  { javaPath.getPath(), "-classpath", deployJar,
                    JreLocator.class.getName() }, msTimeout);
        JREInfo jre = null;

        if (result != null) {
            jre = extractVersion(javaPath.getPath(), result[0]);

            if (jre == null) {
                jre = extractVersion(javaPath.getPath(), result[1]);
            }

            if (jre != null && jre.getPlatform().equals("1.2")) {
                // 1.2 platform versions don't contain full version
                // extract from -fullversion
                result = execute(new String[] { javaPath.getPath(),
                                                "-fullversion" }, msTimeout);

                if (result != null) {
                    jre = extractVersionFor12(javaPath.getPath(), result[0]);

                    if (jre == null) {
                        jre = extractVersionFor12(javaPath.getPath(),
                                                  result[1]);
                    }
                }
            }
        }
        if (Trace.isTraceLevelEnabled(TraceLevel.BASIC)) {
            Trace.println("\tjre search returning: " + jre, TraceLevel.BASIC);
        }
        
        return jre;
    }

    private static String[] execute(String[] commands, int msTimeout) {
        Process p = null;
        boolean done = false;

	Trace.println("jre search executing", TraceLevel.BASIC);
	for (int counter = 0; counter < commands.length; counter++) {
	    Trace.println(counter + ": " + commands[counter], TraceLevel.BASIC);
	}
        
        try {
            p = Runtime.getRuntime().exec(commands);
        } catch (IOException ioe) {
            done = true;
        }
        int exitValue = -1;
        int waitCount = msTimeout / 100;
        while (!done) {
            // Wait a bit.
            try {
                Thread.sleep(100);
            }
            catch (InterruptedException ite) {}

            // Check if done.
            try {
                exitValue = p.exitValue();
                done = true;       
		Trace.println("\tfinished executing " + exitValue, TraceLevel.BASIC);                
            }
            catch (IllegalThreadStateException itse) {
                if (--waitCount == 0) {
                    // Give up on it!
                    done = true;             
		    Trace.println("\tfailed " + exitValue, TraceLevel.BASIC);
		    p.destroy();
                }
            }
        }
        if (done && exitValue == 0) {
            String[] results = new String[2];

            results[0] = readFromStream(p.getErrorStream());
            results[1] = readFromStream(p.getInputStream());
     
	    Trace.println("result: " + results[0], TraceLevel.BASIC);
	    Trace.println("result: " + results[1], TraceLevel.BASIC);
            
            return results;
        }
        return null;
    }

    /**
     * Continually executes read on <code>is</code> until -1 is returned,
     * returning the read result.
     */
    private static String readFromStream(InputStream is) {
        StringBuffer sb = new StringBuffer();
        try {
            byte[] buff = new byte[80];
            boolean done = false;

            while (!done) {
                int amount = is.read(buff, 0, 80);
                if (amount == -1) {
                    done = true;
                }
                else if (amount > 0) {
                    sb.append(new String(buff, 0, amount));
                }
            }
        } catch (IOException ioe) { }
        try {
            is.close();
        }
        catch (IOException ioe) {}
        return sb.toString();
    }

    /**
     * This extracts the JRE version from the passed in string.
     * <p>
     * This returns null if the product or platform could not be determined.
     */
    private static JREInfo extractVersion(
                         String path, String vString) {
        String platform = extractString(PLATFORM_ID, vString);
        String product = extractString(PRODUCT_ID, vString);

        if (platform != null && product != null) {
            return new JREInfo(platform, product, null, path, null,
			Config.getOSName(), Config.getOSArch(), true, false);
        }
        return null;
    }

    /**
     * Extracts the string after <code>id</code> in the <code>string</code>.
     * The returned string ends with the next newline after <code>id</code>,
     * or the end of the string.
     * <p>
     * If <code>id</code> is not found, null will be returned.
     */
    private static String extractString(String id, String string) {
        int index = string.indexOf(id);

        if (index != -1) {
            int end = string.indexOf('\n', index);
            String result;

            if (end != -1) {
                result = string.substring(index + id.length(), end);
            }
            else {
                result = string.substring(index + id.length());
            }
            if (result.length() > 0 && result.charAt(result.length() - 1) ==
                '\r') {
                result = result.substring(0, result.length() - 1);
            }
            return result;
        }
        return null;
    }

    /**
     * This extracts the JRE version from the passed in string. This is
     * only used for vms with 1.2, in which the complete product version was
     * not specified.
     * <p>
     * This will have to evolve as the version strings change.
     */
    private static JREInfo extractVersionFor12(String path, String vString) {
        int index = vString.indexOf("1.2");
        int length = vString.length();

        if (index != -1 && index < (length - 1)) {
            int endIndex = vString.indexOf('"', index);

            if (endIndex != -1) {
                String version = vString.substring(index, endIndex);

                return new JREInfo("1.2", version, null, path, null,
			Config.getOSName(), Config.getOSArch(), true, false); 
            }
        }
        return null;
    }

    /**
     * Returns the classpath to use when determining the java version.
     */
    private static String getClassPath(File javaPath) {
        File file = javaPath;

        file = file.getParentFile();
        if (file != null) {
            file = file.getParentFile();

            if (file != null) {
                file = new File(file, "lib");

                if (file != null && file.exists()) {
                    file = new File(file, "classes.zip");

                    if (file != null && file.exists()) {
                        return getThisPath() + File.pathSeparator + file.getPath();
                    }
                }
            }
        }
        return getThisPath();
    }


    public static void main(String[] args) {
        write(PLATFORM_ID, System.getProperty("java.specification.version"));
        write(PRODUCT_ID, System.getProperty("java.version"));
    }

    private static void write(String left, String right) {
        if (right != null) {
            System.out.println(left + right);
        }
    }

    private static String getThisPath() {
	return Config.getJavaHome() + File.separator + "lib" + 
		File.separator + "javaws.jar";
    }

}
