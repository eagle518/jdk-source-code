/*
 * @(#)SplitJRE.java	1.44 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.kernel;

import java.io.*;
import java.lang.reflect.*;
import java.net.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;
import java.util.regex.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import sun.jkernel.BackgroundDownloader;
import sun.jkernel.DownloadManager;
import sun.jkernel.BundleCheck;

/** 
 * Splits a JRE image into multiple bundles based on an XML descriptor file. 
 * Takes the Java build path (the parent directory of j2se-image and 
 * j2sdk-image) and creates two additional directories underneath this path:
 * tmp/kernel/bundles and tmp/kernel/kernel-image.
 * <p>
 * The generated bundles directory contains a large number of zip files,
 * which are the downloadable bundles themselves.  The kernel-image
 * directory contains the contents of the "core" bundle defined in bundles.xml,
 * which is the minimal set of files and resources necessary to launch a
 * functional Java VM.
 * <p>
 * Any files or resources which are not allocated to a bundle defined in
 * bundles.xml will be assigned to an automatically-generated bundle.  Classes
 * are placed into bundles based on their package names, e.g. 
 * javax.swing.JButton is automatically placed into a bundle named javax_swing
 * unless otherwise specified.  All other files (e.g. DLL files) which are not
 * assigned to a bundle will be assigned to the "misc" bundle.
 * <p>
 * As part of this process, the rt.jar file will have an additional
 * entry added to it: resource_map.  This file contains a terse
 * representation of which file/resource lives in which bundle, and is used by
 * sun.jkernel.DownloadManager to determine which bundles need to be installed at
 * runtime.
 *
 *@author Ethan Nicholas
 */
public class SplitJRE {
    private static final String KERNEL_DEPLOY_DEBUG = "kernel.deploy.debug";
    static final boolean debug =
        "true".equals(System.getProperty(KERNEL_DEPLOY_DEBUG));
    private static final String RESOURCE_MAP_PATH    = "sun/jkernel/resource_map";
    private static final String FILE_MAP_PATH        = "sun/jkernel/file_map";
    private static final String BUNDLE_PROPERTIES_PATH       = "sun/jkernel/bundle.properties";

    // The dummy bundles are created from internal resource pack files.
    // TODO: Lots of "/" that may need to be File.separator? I couldn't
    // figure out when the platform-specific value is needed and when it isn't.

    private static final String DUMMY_CLASS_1        = "sun/kernel/Dummy1.class";
    private static final String DUMMY_CLASS_2        = "sun/kernel/Dummy2.class";
    private static final String DUMMY_PACK_1         = "/dummy_bundle_1/classes.pack";
    private static final String DUMMY_PACK_2         = "/dummy_bundle_2/classes.pack";
    // The dummy bundle paths are below the JRE image copy path
    private static final String DUMMY_BUNDLE_1_PATH  = "/lib/bundles/dummy_bundle_1.zip";
    private static final String DUMMY_BUNDLE_2_PATH  = "/lib/bundles/dummy_bundle_2.zip";
    private static final String DUMMY_BUNDLE_1_NAME = "dummy_bundle_1";
    private static final String DUMMY_BUNDLE_2_NAME = "dummy_bundle_2";

    // The bundle security (crypto hash code) properties resource path

    private static final String CHECK_VALUES_PATH    = "sun/jkernel/" + 
        DownloadManager.CHECK_VALUES_FILE;

    private static final String CLASS_SUFFIX         = ".class";
    
    // directory into which files with "wait='true'" are placed until moved during
    // cleanup
    private static final String WAIT_PREFIX = "lib/bundles/tmp/";
    
    /** Path to the Java build output directory. */
    private static File buildPath;
    
    /** Path to the copy of j2re-image. */
    private static File j2reImageCopyPath;
    
    private static Set/*<String>*/ fileNames = new HashSet/*<String>*/();
    
    /** Maps resource relative paths to the jar files from which they came. */
    private static Map/*<String, JarFile>*/ resourceNames = new HashMap/*<String, JarFile>*/();
    
    /** 
     * Maps resource paths (e.g. java/lang/Object.class) to their respective
     * bundles.
     */
    private static Map/*<String, Bundle>*/ resourceMap = new HashMap/*<String, Bundle>*/();
    
    /** 
     * Maps relative file paths (e.g. bin/jvm.dll) to their respective bundles.
     */
    private static Map/*<String, Bundle>*/ fileMap = new HashMap/*<String, Bundle>*/();

    /**
     * Maps bundle names to a secondary map of key-value pairs to be stored in
     * bundle.properties.  bundle.properties is a properties file with each value
     * containing secondary key-value pairs separated by pipes (|):
     * bundle1 = size=42125|jarpath=lib/bundle_name.jar|dependencies=bundle1
     * bundle2 = size=82125
     * In this example bundle1 has a size, jarpath, and dependencies specified
     * whereas bundle2 only has a size defined.  All bundles are guaranteed to
     * at least have a size specified.
     */
    private static Map/*<String, Map<String, String>>*/ bundleProperties = 
            new HashMap/*<String, Map<String, String>>*/();

    /**
     * The directory to which the generated bundles are written.
     */
    private static File bundleDirectory;
    
    /**
     * Properties to pass in to pack200.
     */
    private static File packProperties;
    
    /** 
     * Returns the path (specified on the command line) where Java has been
     * previously built.
     */
    private static File getBuildPath() {
         return buildPath;
    }
    
    
    static File getPackProperties() {
        return packProperties;
    }
    

    /** Returns the path to the j2re-image directory. */
    private static File getJ2REImagePath() throws IOException {
        return new File(getBuildPath(), "tmp" + File.separator + "ishield" + 
                File.separator + "patch" + File.separator + "jre-image");
    }
    
    
    /** 
     * Returns the path to a specially constructed copy of the JRE, which
     * is set up with two dummy bundles.  These dummy bundles are
     * then downloaded by DownloadTest to determine which classes are needed
     * to perform bundle downloads.
     */
    private static File getJ2REImageCopyPath() throws IOException {
        if (j2reImageCopyPath == null) {
            File rawPath = getJ2REImagePath();
            System.out.println("### Basing Kernel on " + rawPath + "...");
            j2reImageCopyPath = new File(getKernelPath(), "j2re-image-copy");
            if (j2reImageCopyPath.exists())
                deleteAll(j2reImageCopyPath);
            Set/*<String>*/ excludes = new HashSet/*<String>*/();
            excludes.add("rt.jar");
            excludes.add("resources.jar");
            excludes.add("meta-index");
            copyAll(rawPath, j2reImageCopyPath, excludes);

            // rejar resources.jar and exclude the resource_map file 
            // (otherwise the DownloadTest won't work)            
            createJ2REImageCopyResources(j2reImageCopyPath);

            // rejar rt.jar and add extra files to it
            File rtJar = new File(j2reImageCopyPath, "lib" + File.separator + 
                                  "rt.jar");
            File rtPath = new File(rawPath, "lib" + File.separator + "rt.jar");
            JarInputStream in = new JarInputStream(
                new BufferedInputStream(new FileInputStream(rtPath)));
            JarOutputStream out = new JarOutputStream(
                new BufferedOutputStream(new FileOutputStream(rtJar)));
            for (;;) {
                JarEntry e = in.getNextJarEntry();
                if (e == null)
                    break;
                String name = e.getName();
                if (!name.equals(RESOURCE_MAP_PATH) &&
                        !name.equals(FILE_MAP_PATH) &&
                        !name.equals(BUNDLE_PROPERTIES_PATH) &&
                        !name.equals(CHECK_VALUES_PATH)) {
                    out.putNextEntry(e);
                    DownloadManager.send(in, out);
                }
            }


            Bundle dummy1 = new Bundle(DUMMY_BUNDLE_1_NAME);
            Bundle dummy2 = new Bundle(DUMMY_BUNDLE_2_NAME);
            Bundle core = new Bundle("core");
            out.putNextEntry(new JarEntry(RESOURCE_MAP_PATH));
            HashMap/*<String, Bundle>*/ resourceMap = new HashMap/*<String, Bundle>*/();
            resourceMap.put(DUMMY_CLASS_1, dummy1);
            resourceMap.put(DUMMY_CLASS_2, dummy2);

            // need core to show up in bundle list so completion check works 
            // properly
            resourceMap.put("java/lang/Object.class", core);
            writeTreeMap(resourceMap, out);

            // Restart the check value properties so they aren't
            // polluted with the dummy bundle checks

            BundleCheck.resetProperties();

            out.putNextEntry(new JarEntry(FILE_MAP_PATH));
            writeTreeMap(new HashMap/*<String, Bundle>*/(), out);
            out.putNextEntry(new JarEntry(BUNDLE_PROPERTIES_PATH));
            Properties props = new Properties();
            props.put(DUMMY_BUNDLE_1_NAME, "size=0|" + DownloadManager.DEPENDENCIES_PROPERTY + "=" + 
                    DUMMY_BUNDLE_2_NAME);
            props.put(DUMMY_BUNDLE_2_NAME, "size=0");
            props.store(out, null);
            in.close();
            out.close();

            InputStream jvmIn = SplitJRE.class.getResourceAsStream("/jvm.dll");
            File jvmPath = new File(j2reImageCopyPath, "bin" + File.separator + 
                                   "client" + File.separator + "jvm.dll");
            OutputStream jvmOut = new FileOutputStream(jvmPath);
            DownloadManager.send(jvmIn, jvmOut);
            jvmIn.close();
            jvmOut.close();
        }
        return j2reImageCopyPath;
    }
    
    
    private static void createJ2REImageCopyResources(File j2reImageCopyPath) 
            throws IOException {
        File resourcesJar = new File(j2reImageCopyPath, "lib" + 
                                     File.separator + "resources.jar");
        File originalPath =new File(getJ2REImagePath(), "lib" + File.separator + 
                                    "resources.jar");
        InputStream resourceIn = new FileInputStream(originalPath);
        JarInputStream in = new JarInputStream(
                new BufferedInputStream(resourceIn));
        OutputStream resourceOut = new FileOutputStream(resourcesJar);
        JarOutputStream out = new JarOutputStream(
                new BufferedOutputStream(resourceOut));
        for (;;) {
            JarEntry e = in.getNextJarEntry();
            if (e == null)
                break;
            String name = e.getName();
            if (name.equals(RESOURCE_MAP_PATH) ||
                    name.equals(FILE_MAP_PATH) ||
                    name.equals(BUNDLE_PROPERTIES_PATH) ||
                    name.equals(CHECK_VALUES_PATH))
                continue;
            out.putNextEntry(e);
            DownloadManager.send(in, out);
        }
        // Create the dummy bundles and get them into the image
        // copy resources.jar.  We will use -Dkernel.download.url=internal-resource/
        // to ensure that the files are "downloaded" by loading them from
        // resources.jar.
        InputStream din = SplitJRE.class.getResourceAsStream(
            DUMMY_PACK_1);
        File df1 = new File(j2reImageCopyPath.getPath() + 
            DUMMY_BUNDLE_1_PATH);
        File bundles = df1.getParentFile();
        if (bundles.exists()) {
            deleteAll(bundles);
        }
        bundles.mkdirs();

        ZipOutputStream dout = new ZipOutputStream( 
            new BufferedOutputStream(new FileOutputStream(df1)));
        dout.putNextEntry(new JarEntry("classes.pack"));
        DownloadManager.send(din, dout);
        din.close();
        dout.close();
        Bundle.compressAndSign(DUMMY_BUNDLE_1_NAME, df1);

        din = SplitJRE.class.getResourceAsStream(
            DUMMY_PACK_2);
        File df2 = new File(j2reImageCopyPath.getPath() + 
            DUMMY_BUNDLE_2_PATH);
        dout = new ZipOutputStream( 
            new BufferedOutputStream(new FileOutputStream(df2)));
        dout.putNextEntry(new JarEntry("classes.pack"));
        DownloadManager.send(din, dout);
        din.close();
        dout.close();
        Bundle.compressAndSign(DUMMY_BUNDLE_2_NAME, df2);

        // Add the dummy bundle check value properties to resources.jar
        out.putNextEntry(new JarEntry(CHECK_VALUES_PATH));
        BundleCheck.storeProperties(getKernelPath().getPath() +
            File.separator + CHECK_VALUES_PATH);
        FileInputStream cfin = new FileInputStream(getKernelPath().getPath() +
            File.separator + CHECK_VALUES_PATH);
        DownloadManager.send(cfin, out);
        cfin.close();

        in.close();
        out.close();

        mergeIntoJar(df1.getParentFile(), df1.getName(), resourcesJar);
        df1.delete();

        mergeIntoJar(df2.getParentFile(), df2.getName(), resourcesJar);
        df2.delete();
    }    
    
    private static boolean jarBinPathInitialized = false;
    private static File jarBinPath;

    /** Returns the path to the "bin" directory containing the "jar" command. */
    static File getBinPath() {
        if (!jarBinPathInitialized) {
            jarBinPathInitialized = true;
            String sep = File.separator;
            String exe = ".exe";
            File binDir = new File(getBuildPath(), "j2sdk-image" +
                                   sep + "bin");
            File tmp = new File(binDir, "jar" + exe);
            if (tmp.exists()) {
                jarBinPath = binDir;
            } else {
                jarBinPath = new File(getBuildPath(), "bin");
            }
        }

        return jarBinPath;
    }
    
    /** Returns the path to the tmp/kernel output directory. */
    static File getKernelPath() {
        return new File(getBuildPath(), "tmp" + File.separator + "kernel");
    }
    
    
    /** Returns the path to the kernel-image output directory. */
    private static File getStrippedImagePath() {
        return new File(getBuildPath(), "j2re-image-kernel");
    }
    

    /**
     * Recursively scans through a directory, adding the relative paths of all
     * files to the specified set.
     *
     *@param path the physical path to scan
     *@param name the current logical relative path, should be null for the root
     *@param result the output set
     */
    private static void doLoadFileNames(File path, String name, 
                                        Set/*<String>*/ paths) {
        if (path.isDirectory() && !path.getName().equals("bundles")) {
            File[] files = path.listFiles();
            for (int i = 0; i < files.length; i++)
                doLoadFileNames(files[i], 
                                name != null ? 
                                          name + "/" + files[i].getName() : 
                                          files[i].getName(),
                                paths);
        } else
            paths.add(name);
    }
    
    
    private static void loadEntryNames(JarFile jar, 
                                       Map/*<String, JarFile>*/ names) 
                                      throws IOException {
        Enumeration entries = jar.entries();
        while (entries.hasMoreElements()) {
            JarEntry entry = (JarEntry) entries.nextElement();
            if (!entry.isDirectory())
                names.put(entry.getName(), jar);
        }
    }
     
    
    /**
     * Recursively scans the resource directory (the "classes" directory under
     * the Java build path) and adds the relative paths of all files to the
     * the "resourceNames" map.
     */
    private static void loadResourceNames() throws IOException {
        File rt = new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "rt.jar");
        loadEntryNames(new JarFile(rt), resourceNames);
        File resources = new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "resources.jar");
        loadEntryNames(new JarFile(resources), resourceNames);
        loadEntryNames(new JarFile(new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "deploy.jar")), 
                         resourceNames);
        loadEntryNames(new JarFile(new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "plugin.jar")), 
                         resourceNames);
        loadEntryNames(new JarFile(new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "javaws.jar")), 
                         resourceNames);
        loadEntryNames(new JarFile(new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "jsse.jar")), 
                         resourceNames);
        loadEntryNames(new JarFile(new File(getJ2REImagePath(), "lib" + 
                                    File.separator + "jce.jar")), 
                         resourceNames);
        
        // create mappings for rt and resources so that they are not considered
        // unassigned
        Bundle core = Bundle.getBundle("core");
        fileMap.put("lib/rt.jar", core);
        fileMap.put("lib/resources.jar", core);
        resourceMap.put(RESOURCE_MAP_PATH, core);
        resourceMap.put(FILE_MAP_PATH, core);
        resourceMap.put(BUNDLE_PROPERTIES_PATH, core);
    }
     
    
    /**
     * Recursively scans the j2re-image directory and adds the relative paths 
     * of all files to the the "fileNames" set.
     */
    private static void loadFileNames() throws IOException {
        doLoadFileNames(getJ2REImagePath(), null, fileNames);
    }
     
    
    /**
     * Returns the directory in which the bundle archive files should be
     * created.
     */
    static File getBundleDirectory() throws IOException {
        if (bundleDirectory == null) {
            bundleDirectory = new File(getKernelPath(), "bundles");
            if (!bundleDirectory.exists())
                if (!bundleDirectory.mkdir())
                    throw new IOException("failed to create directory " + 
                                        bundleDirectory);
        }
        return bundleDirectory;
    }
    
    
    /**
     * Returns the path to the currently-executing SplitJRE.jar file.
     */
    private static File getSplitJREPath() throws IOException {
        URL splitURL = SplitJRE.class.getResource("SplitJRE.class"); 
        if (!splitURL.getProtocol().equals("jar")) 
            throw new RuntimeException("Error: expected SplitJRE's protocol " + 
                    "to be 'jar', but found " + splitURL); 
        String path = splitURL.getPath(); 
        path = path.substring(0, path.lastIndexOf("!")); 
        splitURL = new URL(path); // strip off "jar:" 
        if (!splitURL.getProtocol().equals("file")) 
            throw new RuntimeException("Error: expected SplitJRE's inner " + 
                "protocol to be 'file', but found " + splitURL); 
        try { 
            return new File(splitURL.toURI()); 
        }
        catch (URISyntaxException e) {
            throw new IOException(e);
        }
    }            
    
    /**
     * Returns a DOM Document with the contents of the bundles.xml file.
     */
    private static Document getBundleDefinitions() throws Exception {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = factory.newDocumentBuilder();
        URL bundlesXML = SplitJRE.class.getResource("bundles.xml");
        Document document = builder.parse(bundlesXML.openStream());
        return document;
    }
    
    
    static String getBundleProperty(String bundleName, String property) {
        Map/*<String, String>*/ map = (Map) bundleProperties.get(bundleName);
        return map != null ? (String) map.get(property) : null;
    }
    
    
    static void putBundleProperty(String bundleName, String property, 
            String value) {
        Map/*<String, String>*/ map = (Map) bundleProperties.get(bundleName);
        if (map == null) {
            map = new HashMap/*<String, String>*/() {
                public String toString() {
                    StringBuilder result = new StringBuilder();
                    Iterator i = entrySet().iterator();
                    while (i.hasNext()) {
                        Map.Entry e = (Map.Entry) i.next();
                        result.append(e.getKey());
                        result.append('=');
                        result.append(e.getValue());
                        if (i.hasNext())
                            result.append('|');
                    }
                    return result.toString();
                }
            };
            bundleProperties.put(bundleName, map);
        }
        map.put(property, value);
    }
    
    
    /**
     * Writes all classes or resources whose names are matched into a bundle.  
     * A match on any single matcher in the "classMatchers" list is sufficient 
     * to put a class into a bundle.  A given class can only appear in one 
     * bundle; additional matches in subsequent bundles are ignored.
     */
    private static void processClasses(List/*<Matcher>*/ classMatchers, 
                                       List/*<Matcher>*/ duplicateMatchers,
                                       Bundle bundle) 
                                      throws IOException {
        OutputStream rawOut = new FileOutputStream(bundle.getJarFile());
        JarOutputStream jarOut = new JarOutputStream(rawOut); 
        try {
            Iterator/*<String>*/ i = resourceNames.keySet().iterator();
            while (i.hasNext()) {
                String name = (String) i.next();

                boolean matched = false;                
                if (duplicateMatchers != null) {    
                    Iterator/*<Matcher>*/ j = duplicateMatchers.iterator();
                    while (j.hasNext()) {
                        Matcher matcher = (Matcher) j.next();
                        matcher.reset(name);
                        if (matcher.matches()) {
                            matched = true;
                            break;
                        }
                    }
                }
                
                if (!matched) {
                    if (resourceMap.keySet().contains(name))
                        continue;
                
                    Iterator/*<Matcher>*/ j = classMatchers.iterator();
                    while (j.hasNext()) {
                        Matcher matcher = (Matcher) j.next();
                        matcher.reset(name);
                        if (matcher.matches()) {
                            matched = true;
                            break;
                        }
                    }
                }
                
                if (matched) {
                    JarFile jar = (JarFile) resourceNames.get(name);
                    String jarName = jar.getName();
                    resourceMap.put(name, bundle);
                    JarEntry entry = new JarEntry(name);
                    entry.setTime(DownloadManager.KERNEL_STATIC_MODTIME);
                    jarOut.putNextEntry(entry);
                    InputStream in = jar.getInputStream(jar.getEntry(name));
                    DownloadManager.send(in, jarOut);
                    in.close();
                }
            }
        } finally {
            jarOut.close();
        }

        String jarPath = getBundleProperty(bundle.getName(), 
                DownloadManager.JAR_PATH_PROPERTY);
        if (jarPath != null) {
            // Overwrite the manufactured jar with the original for binary
            // identity.  We still go through the work of creating the jar
            // so that all of the right maps get initialized and such, but now
            // we can safely discard it in favor of the original.
            copyAll(new File(getJ2REImagePath(), jarPath), bundle.getJarFile());
        }

        
        if (bundle.shouldPackJar()) {
            bundle.packJar();
            JarEntry entry = new JarEntry("classes.pack");
            bundle.getOutputStream().putNextEntry(entry);
            InputStream in = new FileInputStream(bundle.getPackFile());
            DownloadManager.send(in, bundle.getOutputStream());
            in.close();
        } else {
            JarEntry entry = new JarEntry("classes.jar");
            bundle.getOutputStream().putNextEntry(entry);
            File jarPathsFile = new File(getJ2REImageCopyPath(), 
                    getBundleProperty(bundle.getName(), 
                    DownloadManager.JAR_PATH_PROPERTY));
            InputStream in = new FileInputStream(jarPathsFile);
            DownloadManager.send(in, bundle.getOutputStream());
            in.close();
        }
    }
    
    
    private static String getDownloadURL() {
        try {
            Class dm = DownloadManager.class;
            Method getDownloadURL = dm.getDeclaredMethod("getBaseDownloadURL", 
                                                         new Class[0]);
            getDownloadURL.setAccessible(true);
            return (String) getDownloadURL.invoke(null, new Object[0]);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }    
    
    
    // delete lib/bundles/tmp to prevent possible attempts to "reassemble"
    // j2re-image-copy
    private static void clearTmp() throws IOException {
        File tmp = new File(getJ2REImageCopyPath(), "lib" + File.separator + 
                "bundles" + File.separator + "tmp");
        if (tmp.exists())
            deleteAll(tmp);
    }
    
    
    public static void send(Reader in, Writer out) 
            throws IOException {    
        char[] buffer = new char[2048];
        int c;
        while ((c = in.read(buffer)) > 0)
            out.write(buffer, 0, c);
    }
    
    
    /**
     * Processes a &lt;neededToRun&gt; tag and produces a collection of all 
     * class names necessary to satisfy the tag.  This is implemented by using
     * Runtime.exec() to launch an external Java process with the -verbose
     * parameter and then parsing the output.
     */
    private static Set/*<String>*/ processNeededToRun(Element neededToRun) 
                                                     throws IOException {
     int attempt=0;
     while(true) {
        attempt++;
        clearTmp();
        File receipts = new File(getJ2REImageCopyPath(), "lib" + File.separator + 
                "bundles" + File.separator + "receipts");
        if (receipts.exists())
            receipts.delete();
        final Set/*<String>*/ result = new HashSet/*<String>*/();
        File launcher = new File(getJ2REImageCopyPath(), "bin" + 
                                    File.separator + "java");
        String filter = neededToRun.getAttribute("filter");
        if (filter.length() == 0)
            filter = ".*";
        else
            filter = escapePattern(filter);
        final Matcher filterMatcher = Pattern.compile(filter).matcher("");
        String args = neededToRun.getAttribute("args");
        String javaHome = getJ2REImageCopyPath().getPath().replaceAll("\\\\", "/");
        args = args.replaceAll("%JAVA_HOME%", '"' + javaHome + '"');
        File parent = getSplitJREPath().getParentFile();
                         //"-Dkernel.gzipcompression.enabled=true " +
        String command = '"' + launcher.getPath() + '"' + " -verbose " +
                         "-Dkernel.nomerge=true -Dkernel.debug=true " +
                         "-Dkernel.download.url=internal-resource/ " + 
                         "-D" + 
                         BackgroundDownloader.BACKGROUND_DOWNLOAD_PROPERTY + 
                         "=false ";
        if (debug) {
            command += "-D" + KERNEL_DEPLOY_DEBUG + "=true ";
        }
        command += args;
        if (debug) {
            System.out.println("NeededToRun attempt="+attempt+" Exec: " + command + 
                " in working directory: " + parent.getPath());
        }
        final Process java = Runtime.getRuntime().exec(command, 
                                                       null, 
                                                       parent);
        Thread outputReader = new Thread("outputReader") {
            public void run() {
                try {
                    Reader rawIn = new InputStreamReader(java.getInputStream());
                    BufferedReader in = new BufferedReader(rawIn);
                    String pattern = "\\[Loaded (.*) from .*rt.jar\\]";
                    Matcher lineMatcher = Pattern.compile(pattern).matcher("");
                    String line;
                    while ((line = in.readLine()) != null) {
                        lineMatcher.reset(line);
                        if (lineMatcher.matches()) {
                            String name = lineMatcher.group(1);
                            filterMatcher.reset(name);
                            if (filterMatcher.matches())
                                result.add(name);
                        }
                        else if (debug) {
			    System.out.println(line);
			}
}
                    send(in, new StringWriter());
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (Throwable te) {
                    te.printStackTrace();
                }
            }
        };
        outputReader.start();
        
        Thread errorReader = new Thread("errorReader") {
            public void run() {
                try {
                    InputStream in = java.getErrorStream();
                    DownloadManager.send(in, System.err);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        };
        errorReader.start();
        
        try {
            java.waitFor();                    
            if (java.exitValue() != 0) {
                System.err.println("Error: Java process exited with code " + 
                                   java.exitValue());
                System.exit(java.exitValue());      
            }
            outputReader.join();
            errorReader.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
            System.exit(192);
        }
        
        System.out.println("neededToRun: " + result.size() + " classes");
        if (result.isEmpty()) {
          if (attempt > 10)
            throw new RuntimeException("Error: neededToRun '" + 
                                       neededToRun.getAttribute("args") + 
                                       "' did not generate a list of classes");
       } else {
          return result;
       }
      }
    }
    
    
    /** 
     * Converts the simple wildcard patterns supported by bundles.xml (e.g.
     * "javax.swing.*") into proper regular expressions.
     */      
    private static String escapePattern(String pattern) {
        boolean endsWithClass = pattern.endsWith(CLASS_SUFFIX);
        if (endsWithClass)
            pattern = pattern.substring(0, pattern.length() - 
                                           CLASS_SUFFIX.length());
        pattern = pattern.replace("/", "[./]");
        pattern = pattern.replace("*", "[^/]*");
        pattern = pattern.replace("...", ".*");
        pattern = pattern.replace("$", "\\$") + "(:?\\$.*)?";
        if (endsWithClass)
            pattern += "\\.class";
        return pattern;
    }

    
    /**
     * Processes a &lt;classes&gt; tag from bundles.xml and adds all matched
     * classes to the bundle.
     */
    private static void processClasses(Element classes, Bundle bundle) 
                                                throws IOException {
        List/*<Matcher>*/ matchers = new ArrayList/*<Matcher>*/();
        NodeList children = classes.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) child;
                String tagName = e.getTagName();
                if (tagName.equals("class")) { // simple wildcard pattern
                    String className = e.getTextContent().trim();
                    if (className.indexOf(".") < className.lastIndexOf("/") &&
                            !className.endsWith("*"))
                        className += CLASS_SUFFIX;
                    Pattern pattern = Pattern.compile(escapePattern(className));
                    matchers.add(pattern.matcher(""));
                } else if (tagName.equals("neededToRun")) { // use java -verbose                                                         
                    Set/*<String>*/ classNames = processNeededToRun(e);
                    Iterator/*<String>*/ j = classNames.iterator();
                    while (j.hasNext()) {
                        String name = (String) j.next();
                        name = name.replace('.', '/') + CLASS_SUFFIX;
                        if (resourceMap.keySet().contains(name))
                            continue;
                        Pattern pattern = Pattern.compile(escapePattern(name));
                        matchers.add(pattern.matcher(""));
                    }
                }
            }
        }
        processClasses(matchers, null, bundle);
    }
    
    
    /** Adds a file from the j2re-image directory to the specified bundle. */
    static void addFileToBundle(String fileName, Bundle bundle, boolean optional) 
                                                        throws IOException {
        fileName = fileName.replace("\\", "/");
        bundle.getOutputStream().putNextEntry(new JarEntry(fileName));
        File file = new File(getJ2REImagePath(), 
                                fileName.replace('/', File.separatorChar));
        InputStream in;
        // TODO: this is temporary, until the new JVM is incorporated into the
        // core
        if (fileName.equals("bin/kernel/jvm.dll"))
            in = SplitJRE.class.getResourceAsStream("/jvm.dll");
        else if (fileName.equals("bin/kernel/Xusage.txt"))
            in = new FileInputStream(new File(getJ2REImagePath(), 
                                              "bin/client/Xusage.txt"));
        // FIXME: hardcoded reference to i386
        else if (fileName.equals("lib/i386/jvm.cfg")) 
            in = SplitJRE.class.getResourceAsStream("/jvm.cfg");
        else if (fileName.startsWith(WAIT_PREFIX))
            in = new FileInputStream(new File(getJ2REImagePath(), 
                             fileName.substring(WAIT_PREFIX.length())));
        else {
            try {
                in = new FileInputStream(file);
            } catch (FileNotFoundException e) {
                in = null;
            }
        }
        if (in != null) {
            DownloadManager.send(in, bundle.getOutputStream());
            fileMap.put(fileName, bundle);
            in.close();
        } else {
            if (!optional)
                throw new Error("could not locate stream for " + fileName);
        }
    }
    
    
    /** 
     * Processes a &lt;files&gt; tag from bundles.xml and adds all matched files
     * to the specified bundle.
     */
    private static void processFiles(Element files, Bundle bundle) 
                                                        throws IOException {
        NodeList children = files.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) child;
                String tagName = e.getTagName();
                if (tagName.equals("file")) { // file name pattern
                    String patternString = e.getTextContent().trim();
                    boolean wait = "true".equals(e.getAttribute("wait"));
                    boolean optional = "true".equals(e.getAttribute("optional"));
                    if (patternString.indexOf("*") == -1 && 
                            patternString.indexOf("...") == -1) {
                        // simple name, no wildcards
                        if (wait)
                            patternString = WAIT_PREFIX + patternString;
                        addFileToBundle(patternString, bundle, optional);
                    } else {
                        // perform wildcard matching
                        patternString = escapePattern(patternString);
                        Pattern pattern = Pattern.compile(patternString);
                        Matcher matcher = pattern.matcher("");
                        Iterator/*<String>*/ j = fileNames.iterator();
                        while (j.hasNext()) {
                            String fileName = (String) j.next();
                            if (!fileMap.containsKey(fileName)) {
                                matcher.reset(fileName);
                                if (matcher.matches() && 
                                        !fileMap.containsKey(fileName)) {
                                    if (wait)
                                        fileName = WAIT_PREFIX + fileName;
                                    addFileToBundle(fileName, bundle, optional);
                                }
                            }
                        }
                    }
                } else if (tagName.equals("dummyFile")) { // zero-byte file
                    String fileName = e.getTextContent().trim();
                    if (!fileMap.containsKey(fileName)) {
                        JarEntry entry = new JarEntry(fileName);
                        bundle.getOutputStream().putNextEntry(entry);
                    }
                } else if (tagName.equals("dummyJAR")) { // minimal jar
                    String fileName = e.getTextContent().trim();
                    if (!fileMap.containsKey(fileName)) {
                        JarEntry entry = new JarEntry(fileName);
                        bundle.getOutputStream().putNextEntry(entry);
                        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
                        JarOutputStream out = new JarOutputStream(buffer);
                        out.putNextEntry(new JarEntry("DUMMY"));
                        out.close(); 
                        DownloadManager.send(
                                new ByteArrayInputStream(buffer.toByteArray()), 
                                    bundle.getOutputStream());
                    }
                } else
                    throw new RuntimeException("unrecognized tag: " + tagName);
            }
        }
    }
    
    
    /** 
     * Processes a &lt;dependencis&gt; tag from bundles.xml and adds the
     * dependencies to the specified bundle.
     */
    private static void processDependencies(Element dependenciesTag, 
                                            Bundle bundle) 
                                           throws IOException {
        Set/*<String>*/ result = new HashSet/*<String>*/();
        NodeList children = dependenciesTag.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) child;
                String tagName = e.getTagName();
                if (tagName.equals("bundle")) { // static file name
                    String dependency = e.getAttribute("ref");
                    if (dependency.length() == 0)
                        throw new RuntimeException("missing 'ref' attribute " +
                                                   "on bundle tag");
                    result.add(dependency);
                } else
                    throw new RuntimeException("unrecognized tag: " + tagName);
            }
        }
        StringBuilder value = new StringBuilder();
        Iterator/*<String>*/ i = result.iterator();
        while (i.hasNext()) {
            if (value.length() > 0)
                value.append(',');
            value.append(i.next());
        }
        putBundleProperty(bundle.getName(), DownloadManager.DEPENDENCIES_PROPERTY,
                value.toString());
    }
    
    
    static void exec(String cmd, File dir) throws IOException, 
            InterruptedException {
        if (debug) {
            System.out.println("Exec: " + cmd + " in working dir: " +
                getStrippedImagePath());
        }
        Process p = Runtime.getRuntime().exec(cmd, null, 
            dir);
        p.waitFor();
        if (p.exitValue() != 0)
            throw new RuntimeException(cmd + " exited with code " + p.exitValue());
    }
        

    /**
     * Processes a bundle tag to create the corresponding bundle file.
     */
    public static void createBundle(Element bundleDefinition) 
                                                    throws IOException {
        Bundle bundle = Bundle.getBundle(bundleDefinition.getAttribute("name"));
        String install = bundleDefinition.getAttribute("install");
        if (install.length() > 0)
            putBundleProperty(bundle.getName(), "install", install);
        if ("false".equals(bundleDefinition.getAttribute("pack")))
            bundle.setShouldPackJar(false);
        String jarPath = bundleDefinition.getAttribute("jarPath");
        if (jarPath.length() > 0) { // bundle is based on a preexisting single jar
            putBundleProperty(bundle.getName(), DownloadManager.JAR_PATH_PROPERTY, jarPath);
            Map/*<String, JarFile>*/ resourceNamesBackup = resourceNames;
            resourceNames = new HashMap/*<String, JarFile>*/();
            File file = new File(getJ2REImagePath(), jarPath);
            loadEntryNames(new JarFile(file), resourceNames);
            List/*<Matcher>*/ matchers = new ArrayList/*<Matcher>*/();
            Iterator/*<String>*/ i = resourceNames.keySet().iterator();
            while (i.hasNext()) {
                String patternString = escapePattern((String) i.next());
                matchers.add(Pattern.compile(patternString).matcher(""));
            }
            processClasses(null, matchers, bundle);
            resourceNames = resourceNamesBackup;
            fileMap.put(jarPath, bundle);
        }
    
        NodeList children = bundleDefinition.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) child;
                String tagName = e.getTagName();
                if (tagName.equals("classes"))
                    processClasses(e, bundle);
                else if (tagName.equals("files"))
                    processFiles(e, bundle);
                else if (tagName.equals("dependencies"))
                    processDependencies(e, bundle);
                else
                    throw new RuntimeException("unrecognized tag: " + tagName);
            }
        }
        bundle.finish();

        if (bundle.getName().equals("junk"))
            bundle.getBundleFile().delete();
    }
    
    
    private static Set/*<String>*/ determineCoreBundles() throws IOException {
        Set/*<String>*/ coreBundles = new HashSet/*<String>*/();
        int coreSize = coreBundles.size();
        for (;;) {
            Iterator/*<String>*/ i = Bundle.bundles.keySet().iterator();
            while (i.hasNext()) {
                String bundleName = (String) i.next();
                if ("true".equals(getBundleProperty(bundleName, "install")))
                    coreBundles.add(bundleName);
                if (coreBundles.contains(bundleName)) {
                    String dependencies = getBundleProperty(bundleName,
                            DownloadManager.DEPENDENCIES_PROPERTY);
                    if (dependencies != null)
                        coreBundles.addAll(Arrays.asList(dependencies.split("\\s*,\\s*")));
                }
            }
            // continue until we don't find any more bundles to install
            if (coreBundles.size() == coreSize)
                break;
            coreSize = coreBundles.size();
        }
        
        Bundle core = Bundle.getBundle("core");
        Iterator/*<Map.Entry<String, Bundle>>*/ i = fileMap.entrySet().iterator();
        while (i.hasNext()) {
            Map.Entry/*<String, String>*/ e = (Map.Entry) i.next();
            if (coreBundles.contains(((Bundle) e.getValue()).getName()))
                e.setValue(core);
        }
        i = resourceMap.entrySet().iterator();
        while (i.hasNext()) {
            Map.Entry/*<String, String>*/ e = (Map.Entry) i.next();
            if (coreBundles.contains(((Bundle) e.getValue()).getName()))
                e.setValue(core);
        }
        return coreBundles;
    }


    private static void unpackCoreBundles() throws IOException {    
        new File(getStrippedImagePath(), 
                 "lib" + File.separator + "bundles").mkdirs();
        new File(getStrippedImagePath(), 
                 "lib" + File.separator + "ext").mkdirs();
        new File(getStrippedImagePath(), 
                 "lib" + File.separator + "applet").mkdirs();
        File rt = new File(getStrippedImagePath(), "lib" + 
                            File.separator + "rt.jar");

        Set/*<String>*/ core = determineCoreBundles();
        System.out.println("Core bundles: " + core);
        
        JarOutputStream out = new JarOutputStream(
            new BufferedOutputStream(new FileOutputStream(rt)));
        
        Iterator/*<String>*/ i = core.iterator();
        while (i.hasNext()) {
            Bundle bundle = Bundle.getBundle((String) i.next());
            putBundleProperty(bundle.getName(), "install", "true");
            try {
                String cmd = getBinPath() + File.separator + 
                            "jar xf " + bundle.getBundleFile();
                if (debug ) {
                    System.out.println("Exec: " + cmd + " in working dir: " +
                        getStrippedImagePath());
                    Process p = Runtime.getRuntime().exec(cmd, null, 
                        getStrippedImagePath());
                    dumpOutput(p);
                    p.waitFor();
                } else {
                    Runtime.getRuntime().exec(cmd, null, 
                        getStrippedImagePath()).waitFor();
                }
                
                // merge classes.jar into rt.jar
                new File(getStrippedImagePath(), "classes.pack").delete();
                String jarPath = getBundleProperty(bundle.getName(), 
                        DownloadManager.JAR_PATH_PROPERTY);
                if (jarPath == null) {
                    JarInputStream in = new JarInputStream(
                        new BufferedInputStream(new FileInputStream(bundle.getJarFile())));
                    for (;;) {
                        JarEntry e = in.getNextJarEntry();
                        if (e == null)
                            break;
                        out.putNextEntry(e);
                        DownloadManager.send(in, out);
                    }
                    in.close();
                }
                else {
                    bundle.getJarFile().renameTo(new File(getStrippedImagePath(),
                        jarPath.replace('/', File.separatorChar)));
                }
                bundle.getBundleFile().delete();
                File metaInf = new File(getStrippedImagePath(), "META-INF");
                if (metaInf.exists())
                    deleteAll(metaInf);
            } catch (InterruptedException e) {
                e.printStackTrace();
                System.exit(193);
            }
        }
        out.close();
    }

    /**
     * For classes which are not assigned to any bundle, creates a bundle which
     * contains all classes in the class' package.
     */
    private static void createDefaultBundle(String resourceName) 
                                                        throws IOException {
        String packageName = resourceName.substring(0, 
                               resourceName.lastIndexOf("/"));
        Bundle bundle = Bundle.getBundle(packageName.replace('/', '_'));
        List/*<Matcher>*/ matchers = new ArrayList/*<Matcher>*/();
        matchers.add(Pattern.compile(packageName + "/[^/]*").matcher(""));
        processClasses(matchers, null, bundle);

        bundle.finish();
    }
    
    
    /**
     * Recursively adds all files which are not currently in a bundle to the
     * specified bundle, starting from the specified relative path string (the
     * empty string, "", for the j2re-image root).
     */
    private static void addUnownedFilesToBundle(Bundle bundle, 
                                                String relativePath) 
                                                        throws IOException {
        if (relativePath.endsWith("rt.jar") || 
                relativePath.startsWith("lib/bundles"))
            return;
            
        File file = new File(getJ2REImagePath() + File.separator + 
                             relativePath.replace('/', File.separatorChar));
        if (file.isDirectory()) {
            File[] files = file.listFiles();
            for (int i = 0; i < files.length; i++)
                addUnownedFilesToBundle(bundle, relativePath + 
                                        (relativePath.length() > 0 ? "/" : "") + 
                                        files[i].getName());
        } else if (!fileMap.containsKey(relativePath) && 
                !fileMap.containsKey(WAIT_PREFIX + relativePath)) {
            System.out.println("Warning: file " + relativePath + " has not " + 
                                    "been placed into a bundle");
            addFileToBundle(relativePath, bundle, false);
        }
    }
    
    
    // Merges a file into the specified jar
    private static void mergeIntoJar(File base, String path, File dest) 
                                    throws IOException {
        try {
            String jar = new File(getBinPath(), "jar").getPath();
            Process p = Runtime.getRuntime().exec(jar + " uf " + dest + " -C " + 
                            base + ' ' + path);
            dumpOutput(p);
            p.waitFor();
            if (p.exitValue() != 0)
                throw new IOException("jar returned exit value: " + 
                                                p.exitValue());
        } catch (InterruptedException e) {
            e.printStackTrace();
            System.exit(194);
        }
    }
    
    
    // Merges the specified file into rt.jar
    private static void mergeIntoRtJar(File base, String path) throws IOException {
        mergeIntoJar(base, path, new File(getStrippedImagePath(), "lib" + 
                        File.separator + "rt.jar"));
    }


    /**
     * Creates the resource_map file and adds it to rt.jar.  The resource_map
     * is a compact representation of which resources live in which bundles, and
     * is logically structured as a tree.  Each resource is stored as a full
     * path under a bundle; e.g. the Object class is represented as
     * core/java/lang/Object.class.
     *
     * The tree is represented as a sequence of nodes, each of which begins with
     * a "level" byte (from 0-31) indicating its depth.  The CLASS_SUFFIX string
     * appears so frequently that it is special-cased to be represented by the
     * value 255.  The Object, String, and List classes could then be 
     * represented as 
     * <0>core<1>java<2>lang<3>Object<255><3>String<255><2>util<3>List<255>, 
     * with the values in  angle brackets all representing single bytes.  The 
     * (largely textual) contents of this structure then compress very well 
     * using normal compression algorithms, allowing us to store the location
     * of every class in the JRE using ~60KB of data.
     *
     * It is important to note that this algorithm is intended to work with
     * ASCII filenames only.  As of this writing, all core JRE resources have
     * ASCII names.
     *
     *@param entryName the name of the file to produce (e.g. "sun/jkernel/resource_map")
     *@param mapping the map of names and bundles to encode
     */
    private static void writeTreeMap(Map/*<String, Bundle>*/ mapping,
                                    OutputStream out) 
                            throws IOException {
        // create a sorted list of all of the keys to store, e.g.
        // core/java/lang/Object.class, java_awt/java/awt/Button.class
        SortedSet/*<String>*/ entries = new TreeSet/*<String>*/();
        Iterator/*<Map.Entry<String, Bundle>>*/ i = mapping.entrySet().iterator();
        while (i.hasNext()) {
            Map.Entry/*<String, Bundle>*/ e = 
                    (Map.Entry/*<String, Bundle>*/) i.next();
            String name = ((Bundle) e.getValue()).getName();
            if (!name.equals("junk")) {
                String entry = name + "/" + e.getKey();
                entries.add(entry);
            }
        }
        
        String last = null;
        // iterate through the list, writing only the tokens which differ from
        // the previous entry (i.e. if the last key written was 
        // core/java/lang/Object.class and we are now writing 
        // core/java/util/List.class, we will only write <2>util<3>List<255>
        Iterator/*<String>*/ j = entries.iterator();
        while (j.hasNext()) {
            String entry = (String) j.next();
            StringTokenizer stLast = last != null ? 
                    new StringTokenizer(last, "/") : 
                    null;
            StringTokenizer stNew = new StringTokenizer(entry, "/");
            byte level = 0; // byte indicating the current tree depth
            boolean writing = false;
            while (stNew.hasMoreTokens()) {
                String token = stNew.nextToken();
                // set "writing" to true once we hit a token we need to write
                if (!writing) {
                    if (stLast == null || !stLast.hasMoreTokens()) {
                        // no previous token
                        writing = true;
                    } else {
                        String lastToken = stLast.nextToken();
                        if (!lastToken.equals(token)) {
                            // new token differs from old
                            writing = true;
                        }
                    }
                }
                if (level >= 32)
                    throw new Error("Exceeded 32 levels of directory depth");
                if (writing) {
                    out.write(level);
                    if (token.endsWith(CLASS_SUFFIX)) {
                        // replace CLASS_SUFFIX with out-of-band value 255
                        token = token.substring(0, token.length() - 
                                                CLASS_SUFFIX.length()) + 
                                                (char) 255;
                    }
                    out.write(token.getBytes());
                }
                level++;
            }
            last = entry;
        }
    }
    
    
    private static void writeResourceMap() throws IOException {
        File path = new File(getKernelPath(), RESOURCE_MAP_PATH); 
        path.getParentFile().mkdirs();
        OutputStream out = new BufferedOutputStream(new FileOutputStream(path));
        writeTreeMap(resourceMap, out);
        out.close();
        mergeIntoRtJar(getKernelPath(), RESOURCE_MAP_PATH);
    }

    
    private static void writeFileMap() throws IOException {
        File path = new File(getKernelPath(), FILE_MAP_PATH); 
        path.getParentFile().mkdirs();
        OutputStream out = new BufferedOutputStream(new FileOutputStream(path));
        writeTreeMap(fileMap, out);
        out.close();
        mergeIntoRtJar(getKernelPath(), FILE_MAP_PATH);
    }


    private static void writeBundleProperties() throws IOException {
        putBundleProperty("merged", DownloadManager.JAR_PATH_PROPERTY, 
                WAIT_PREFIX + "merged-rt.jar");
        File path = new File(getKernelPath(), BUNDLE_PROPERTIES_PATH); 
        path.getParentFile().mkdirs();
        OutputStream out = new BufferedOutputStream(new FileOutputStream(path));
        Properties tmp = new Properties();
        Iterator i = bundleProperties.entrySet().iterator();
        while (i.hasNext()) {
            Map.Entry e = (Map.Entry) i.next();
            tmp.put(e.getKey(), e.getValue().toString());
        }
        tmp.store(out, null);
        out.close();
        mergeIntoRtJar(getKernelPath(), BUNDLE_PROPERTIES_PATH);
    }

    
    /**
     * Creates all bundles, both those specified in bundles.xml and default 
     * bundles for resources which have not been otherwise assigned.
     */
    public static void createBundles() throws Exception {
        File[] bundleFiles = getBundleDirectory().listFiles();
        for (int i = 0; i < bundleFiles.length; i++)
            bundleFiles[i].delete();
        
        loadResourceNames();
        loadFileNames();

        Document bundleDefinitions = getBundleDefinitions();
        Element bundles = bundleDefinitions.getDocumentElement();
        NodeList children = bundles.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE)
                createBundle((Element) child);
        }
        
        Iterator/*<String>*/ i = resourceNames.keySet().iterator();
        while (i.hasNext()) {
            String name = (String) i.next();
            if (resourceMap.get(name) == null && !name.startsWith("META-INF")
                  && name.indexOf("/") > -1)
                createDefaultBundle(name);
        }
        
        Bundle misc = Bundle.getBundle("misc");
        addUnownedFilesToBundle(misc, "");
        misc.finish();
        unpackCoreBundles();
        writeResourceMap();
        writeFileMap();
    }
    
    
    static void dumpOutput(final Process p) {
        Thread outputReader = new Thread("outputReader") {
            public void run() {
                try {
                    InputStream in = p.getInputStream();
                    DownloadManager.send(in, System.out);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        };
        outputReader.start();
        Thread errorReader = new Thread("errorReader") {
            public void run() {
                try {
                    InputStream in = p.getErrorStream();
                    DownloadManager.send(in, System.err);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        };
        errorReader.start();
    }
    
    
    /** Delete a file tree. */
    private static void deleteAll(File path) throws IOException {
        File[] children = path.listFiles();
        if (children != null) {
            for (int i = 0; i < children.length; i++)
                deleteAll(children[i]);
        }
        if (!path.delete())
            throw new IOException("Unable to delete " + path);
        
    }
    

    /** Copies a file tree. */
    private static void copyAll(File src, File dest) throws IOException {
        copyAll(src, dest, null);
    } 


    /** Copy a file tree, excluding certain named files. */
    private static void copyAll(File src, File dest, Set/*<String>*/ excludes) 
                            throws IOException {
        if (excludes == null || !excludes.contains(src.getName())) {
            if (src.isDirectory()) {
                dest.mkdirs();
                File[] children = src.listFiles();
                if (children != null) {
                    for (int i = 0; i < children.length; i++)
                        copyAll(children[i], 
                                new File(dest, children[i].getName()), 
                                excludes);
                }
            } else {
                dest.getParentFile().mkdirs();
                FileInputStream in = new FileInputStream(src);
                FileOutputStream out = new FileOutputStream(dest);
                DownloadManager.send(in, out);
                in.close();
                out.close();
            }
        }
    }
    
    
    /** 
     * Create a bundle for the "merged" (original) rt.jar and resources.jar.
     */
    public static void createMergedBundle() throws Exception {
        Bundle merged = Bundle.getBundle("merged");
        JarEntry entry = new JarEntry("classes.pack");
        merged.getOutputStream().putNextEntry(entry);
        File rtPack = new File(buildPath, "pack" + 
                File.separator + "pack-jre-jars" + 
                File.separator + "lib" + File.separator + "rt.pack");
        merged.setPackFile(rtPack);
        InputStream in = new FileInputStream(rtPack);
        DownloadManager.send(in, merged.getOutputStream());
        in.close();
        entry = new JarEntry(WAIT_PREFIX + "merged-resources.jar");
        merged.getOutputStream().putNextEntry(entry);
        in = new FileInputStream(new File(getJ2REImagePath(), "lib" + 
                File.separator + "resources.jar"));
        DownloadManager.send(in, merged.getOutputStream());
        in.close();
        merged.finish();
        fileMap.put("lib/bundles/tmp/merged-resources.jar", merged);
    }
    
    
    private static void createDummyIndex() throws IOException {
        Set/*<String>*/ bundleNames = Bundle.bundles.keySet();
        PrintWriter dummyIndex = new PrintWriter(new FileWriter(
                new File(getBundleDirectory(), "index.html")));
        Iterator/*<String>*/ i = bundleNames.iterator();
        while (i.hasNext()) {
            String name = (String) i.next();
            dummyIndex.println(name + ".zip = http://127.0.0.1/" + name + ".zip");
        }
        dummyIndex.close();
    }
        

    static void compressAndSignAllBundles() throws IOException {
        Set bundleNames = Bundle.bundles.keySet();
        Iterator i = bundleNames.iterator();
        while (i.hasNext()) {
            String name = (String) i.next();
            if (!"true".equals(getBundleProperty(name, 
                    DownloadManager.INSTALL_PROPERTY)))
                Bundle.compressAndSign(name, new File(getBundleDirectory(), 
                    name + ".zip"));
        }
        BundleCheck.storeProperties(getKernelPath().getPath() +
            File.separator + CHECK_VALUES_PATH);
        mergeIntoRtJar(getKernelPath(), CHECK_VALUES_PATH);
    }


    // called by writeKernelMap to write out the parent directories of the path
    // which have not yet appeared in the output
    private static void writeParentDirectories(String path, Bundle bundle, 
            Set dirs, PrintWriter out) throws IOException {
        String parent = "";
        for (;;) {
            int index = path.indexOf(File.separatorChar, parent.length());
            if (index == -1)
                break;
            parent = path.substring(0, index + 1);
            if (!dirs.contains(parent)) {
                dirs.add(parent);
                out.print(bundle.getName() + "*" + parent + "*0\n");
            }
        }
    }

    
    private static void writeKernelMap() throws IOException {
        PrintWriter out = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(new File(getStrippedImagePath(), "lib" + 
                File.separator + "kernel.map")), "utf-8"));
        Set dirs = new HashSet();
        Iterator i = fileMap.entrySet().iterator();
        while (i.hasNext()) {
            Map.Entry e = (Map.Entry) i.next();
            Bundle bundle = (Bundle) e.getValue();
            String path = (String) e.getKey();
            String effectivePath;
            if (path.startsWith(WAIT_PREFIX))
                effectivePath = path.substring(WAIT_PREFIX.length());
            else
                effectivePath = path;
            File file;
            if (path.endsWith("merged-resources.jar")) {
                path = "lib/bundles/tmp/merged-resources.jar";
                file = new File(getJ2REImagePath(), "lib" + File.separator + 
                        "resources.jar");
            }
            else if (path.endsWith("jvm.cfg")) {
                file = new File(getJ2REImagePath(), "lib" + File.separator + 
                        "i386" + File.separator + "jvm.cfg");
            }
            else {
                file = new File(getStrippedImagePath(), effectivePath);
                if (!file.exists())
                    file = new File(getJ2REImagePath(), effectivePath);
            }
            
            path = DownloadManager.getKernelJREDir() + bundle.getName() + 
                    File.separatorChar + path.replace('/', File.separatorChar);
            writeParentDirectories(path, bundle, dirs, out);
            BundleCheck check = BundleCheck.getInstance(file);
            out.print(bundle.getName() + "*" + path + "*" + check.toString() + '\n');
        }
        i = Bundle.bundles.values().iterator();
        while (i.hasNext()) {
            Bundle bundle = (Bundle) i.next();
            String jarPath = getBundleProperty(bundle.getName(), 
                    DownloadManager.JAR_PATH_PROPERTY);
            if (jarPath != null)
                jarPath = DownloadManager.getKernelJREDir() + bundle.getName() + 
                            File.separator + jarPath.replace('/',
                            File.separatorChar);
            else
                jarPath = DownloadManager.getKernelJREDir() + bundle.getName() + 
                        File.separator + "lib" + File.separator + "bundles" + 
                        File.separator + bundle.getName() + ".jar";
            writeParentDirectories(jarPath, bundle, dirs, out);
            if (bundle.getPackFile().exists()) {
                sun.jkernel.Bundle.unpack(bundle.getPackFile(), bundle.getJarFile());
                BundleCheck check = BundleCheck.getInstance(bundle.getJarFile());
                out.print(bundle.getName() + "*" + jarPath + "*" + 
                        check.toString() + '\n');
            }
        }
        out.print("temp*" + DownloadManager.getKernelJREDir() + "-bundles\\receipts*0\n");
        out.print("temp*lib\\bundles\\tmp\\finished*0\n");
        out.close();
    }


    public static void main(String[] arg) throws Exception {
        System.setProperty("kernel.download.url", DownloadManager.RESOURCE_URL);
        if (arg.length == 1 && arg[0].equals("exit")) {
            // used as a "JRE startup" test in bundles.xml
            System.exit(0);
        }
        if (arg.length != 2) {
            System.out.println(
                "Usage: SplitJRE <Java build path> <pack.properties> | exit | ");
            System.exit(195);
        }
        buildPath = new File(arg[0]).getCanonicalFile();
        System.out.println("SplitJRE build path: " + buildPath);
        packProperties = new File(arg[1]);
        File bundles = new File(getJ2REImageCopyPath(), 
                                "lib" + File.separator + "bundles");

        new File(getStrippedImagePath(), 
                 "lib" + File.separator + "bundles").mkdirs();
        deleteAll(getStrippedImagePath());
        createBundles();
        createMergedBundle();
        createDummyIndex();
        compressAndSignAllBundles();
        writeBundleProperties();
        writeKernelMap();
        if (debug) {
            System.out.println(
                "total milliseconds spent in " +
                "Bundle.compressFile(String,File): " + 
                Bundle.totalMillisForCompress);
        }
    }
}
