/*
 * @(#)Bundle.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.kernel;

import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;
import sun.jkernel.BundleCheck;
import sun.jkernel.DownloadManager;
import java.util.zip.GZIPOutputStream;

/**
 * Represents a single bundle.  The end-goal of the split process is to 
 * create the file pointed to by the bundleFile field, which is a zip file
 * containing all of the bundle's files and resources.
 */
class Bundle {
     static final Map/*<String, Bundle>*/ bundles = new HashMap/*<String, Bundle>*/();
    
     public static final int BUFFER_SIZE = 8192;

    /** The name of this bundle, e.g. "javax_swing". */
    private String name;
    
    /** 
     * The temporary jar file into which all classes and resources are being
     * placed.
     */
    private File jarFile;
    
    /**
     * The temporary .pack file generated from the temporary jar file.
     */
    private File packFile;
    
    /**
     * The final zip file containing the pack file and all other bundle 
     * files.
     */
    private File bundleFile;

    /**
     * True if the classes.jar file should be packed.  This must be false for
     * our signed JAR files, which are (at least in the current build) not 
     * built to survive packing and unpacking with signatures intact.
     */
    private boolean shouldPackJar = true;
    
    /** A ZipOutputStream which outputs to bundleFile. */
    private ZipOutputStream bundleOutputStream;

    /** Use extra compression for bundles when true */
    private static final boolean extraCompressionEnabled = 
        ! "false".equals(System.getProperty("kernel.extra.compression"));

    public static Bundle getBundle(String name) throws IOException {
        Bundle result = (Bundle) bundles.get(name);
        if (result == null) {
            result = new Bundle(name);
            result.create();
            bundles.put(name, result);
        }
        return result;
    }
    
    
    /** Creates a new bundle with the specified name. */
    public Bundle(String name) {
        this.name = name;
    }
    
    
    public String getName() {
        return name;
    }
    
    
    public File getJarFile() {
        return jarFile;
    }


    public File getPackFile() {
        return packFile;
    }
    
    
    public void setPackFile(File packFile) {
        this.packFile = packFile;
    }
    
    
    public File getBundleFile() {
        return bundleFile;
    }
    
    
    public boolean shouldPackJar() {
        return shouldPackJar;
    }
    
    
    public void setShouldPackJar(boolean shouldPackJar) {
        this.shouldPackJar = shouldPackJar;
    }
    
    
    public ZipOutputStream getOutputStream() {
        return bundleOutputStream;
    }

    
    private void create() throws IOException {
        System.out.println("Creating " + name + "...");
        File packDir = new File(SplitJRE.getKernelPath(), "pack");
        packDir.mkdirs();
        this.jarFile = new File(packDir, name + ".jar");
        this.packFile = new File(packDir, name + ".pack");
        this.bundleFile = new File(SplitJRE.getBundleDirectory(), 
                                   name + ".zip");
        if (bundleFile.exists())
            throw new IOException("bundle file " + bundleFile + 
                                  " already exists");
        OutputStream bundleOut = new FileOutputStream(bundleFile);
        this.bundleOutputStream = new ZipOutputStream(bundleOut);
    }
    
    
    /** Packs the temporary jar to create the temporary .pack file. */
    void packJar() throws IOException {
        System.out.println("Packing " + jarFile + "...");
        String cmd = SplitJRE.getBinPath() + File.separator + "pack200 " +
                "-J-mx256m --no-gzip -f " + SplitJRE.getPackProperties() + " " +
                packFile + " " + jarFile;
        System.out.println("Exec: " + cmd);
        Process p = Runtime.getRuntime().exec(cmd);
        try {
            sun.jkernel.DownloadManager.dumpOutput(p);
            p.waitFor();
            if (p.exitValue() != 0)
                throw new IOException("pack200 exited with code " + p.exitValue());
        }
        catch (InterruptedException e) {
            throw new RuntimeException(e.toString());
        }
    }
    
    /**
     * Convert a bundle to a jar file with zero compression and zero mod
     * time. This is made necessary because we're not in the position to
     * know the compressed size of a bundle and its CRC32 value "up front" at 
     * the time we have to provide it to a new JarEntry before 
     * writing the entry's stream data. That is, since all bundle data would 
     * have to be compressed to a separate file before the JarEntry creation,
     *  then read back to be put into an uncompressed entry (i.e. a "STORE"
     * JarEntry) anyway, it is a wash between doing that (where the copy
     * opeations are all over the place and there is a high and risk of a
     * future code change bracking the accurate tracking of all the stream 
     * bytes) vs just converting the jar here, where we can't easily screw up 
     * the size or CRC. Simple, right? Put differently, we're delaying 
     * build time optimization.
     */


    private static void zeroCopyJar(String srcPath, String dstPath) throws 
        IOException {

        JarInputStream in = new JarInputStream(
            new BufferedInputStream(new FileInputStream(srcPath), BUFFER_SIZE));
        JarOutputStream out = new JarOutputStream(new BufferedOutputStream(
            new FileOutputStream(dstPath), BUFFER_SIZE));
        for (;;) {
            JarEntry e = in.getNextJarEntry();
            if (e == null)
                break;
            if (e.isDirectory()) {
                out.putNextEntry(e);
                DownloadManager.send(in, out);
            } else {
                File tmpFile = File.createTempFile("SplitJRE", ".tmp");
                BufferedOutputStream bout = new BufferedOutputStream(
                    new FileOutputStream(tmpFile), BUFFER_SIZE);

                long size = 0;
                CRC32 crc = new CRC32();
                byte[] buf = new byte[BUFFER_SIZE];
                int readCount;
                while((readCount = in.read(buf)) > 0) {
                    size += readCount;
                    crc.update(buf,0,readCount);
                    bout.write(buf,0,readCount);
                } 
                bout.close();
                JarEntry newEntry = new JarEntry(e);
                newEntry.setMethod(ZipEntry.STORED);
                newEntry.setCompressedSize(size);
                newEntry.setSize(size);
                newEntry.setCrc(crc.getValue());
                newEntry.setTime(DownloadManager.KERNEL_STATIC_MODTIME); 
                out.putNextEntry(newEntry);
                FileInputStream fin = new FileInputStream(tmpFile);
                DownloadManager.send(fin, out);
                fin.close();
                tmpFile.delete();
            }
        }
        in.close();
        out.close();
    }

    // Pulls this after finishing timing tests
    static long totalMillisForCompress;

    /**
     * Do "extra" (e.g. 7-Zip) file compression if enabled or else
     * use a GZIP stream (slightly better than jar/zip compression).
     */
    static void compressFile(String uncompressedPath, 
        String compressedPath) throws IOException {
        long tmpTime=0;
        if (SplitJRE.debug) {
            tmpTime = System.currentTimeMillis();
        }

        try {
            if (extraCompressionEnabled) {
                if (! sun.jkernel.Bundle.extraCompress(uncompressedPath, 
                    compressedPath)) {
                    if (SplitJRE.debug) {
                        System.out.println("compressing " + uncompressedPath +
                            " using LZMA");
                    }
                }
            } else {
                if (SplitJRE.debug) {
                    System.out.println("compressing " + uncompressedPath +
                        " using GZIP stream");
                }
                BufferedInputStream in = new BufferedInputStream(new
                    FileInputStream(uncompressedPath), BUFFER_SIZE);
                GZIPOutputStream out = new GZIPOutputStream(new 
                    BufferedOutputStream(new FileOutputStream(
                    compressedPath),BUFFER_SIZE));
                DownloadManager.send(in,out);
                in.close();
                out.close();
            }
        } catch (Exception e) {
                throw new RuntimeException("Bundle.compressFile threw: " + e);
        }
        if (SplitJRE.debug) {
            totalMillisForCompress += (System.currentTimeMillis() - tmpTime);
        }
    }

    /** Close the bundle. */
    void finish() throws IOException {
        // TODO: remove this
        if (name.equals("core")) {
            SplitJRE.addFileToBundle("bin/kernel/jvm.dll", this, false);
            SplitJRE.addFileToBundle("bin/kernel/Xusage.txt", this, false);
        }
        
        bundleOutputStream.close();

    }

    /** Compress and sign the bundle **/
    static void compressAndSign(String name, File bundleFile) 
        throws IOException {

        // ignore bundles "core" and "junk"
        if (name.equals("core") || name.equals("junk")) {
            return;
        }


        File tmpFile = new File(bundleFile.getParent(), name + ".jar0");
        zeroCopyJar(bundleFile.getPath(), tmpFile.getPath());
        compressFile(tmpFile.getPath(), bundleFile.getPath());
        tmpFile.delete();
        BundleCheck.addProperty(name, bundleFile);
        SplitJRE.putBundleProperty(name, DownloadManager.SIZE_PROPERTY,
                String.valueOf(bundleFile.length()));
    }

    public String toString() {
        return "Bundle[" + name + "]";
    }
}
