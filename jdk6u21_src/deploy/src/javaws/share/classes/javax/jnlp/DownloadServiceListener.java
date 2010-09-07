/*
 * @(#)DownloadServiceListener.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.net.URL;

/**
 * The <code>DownloadServiceListener</code> provides an interface for a
 * callback object implementation, which may be used by a DownloadService
 * implementation.  The <code>DownloadServiceListener</code> implementation's
 * methods should be invoked by the <code>DownloadService</code> implementation
 * at various stages of the download, allowing an application that uses the
 * JNLP API to display a progress bar during a <code>DownloadService</code> download.
 *
 * @since 1.0
 * @see DownloadService
 */

public interface DownloadServiceListener {
    /**
     * A JNLP client's <code>DownloadService</code> implementation should call this method
     * several times during a download.  A <code>DownloadServiceListener</code> implementation
     * may display a progress bar and / or update information based on the parameters.
     *
     * @param url            The URL representing the resource being downloaded.
     *
     * @param version        The version of the resource being downloaded.
     *
     * @param readSoFar      The number of bytes downloaded so far.
     *
     * @param total          The total number of bytes to be downloaded, or -1 if the
     *                       number is unknown.
     *
     * @param overallPercent The percentage of the overall update operation that is complete,
     *                       or -1 if the percentage is unknown.
     */
    public void progress(URL url, String version, long readSoFar, long total, int overallPercent);
    
    /**
     * A JNLP client's <code>DownloadService</code> implementation should call this method
     * at least several times during validation of a download.  Validation often includes
     * ensuring that downloaded resources are authentic (appropriately signed).  A
     * <code>DownloadServiceListener</code> implementation may display a progress bar and / or
     * update information based on the parameters.
     *
     * @param url            The URL representing the resource being validated.
     *
     * @param version        The version of the resource being validated.
     *
     * @param entry          The number of JAR entries validated so far.
     *
     * @param total          The total number of entries to be validated.
     *
     * @param overallPercent The percentage of the overall update operation that is complete,
     *                       or -1 if the percentage is unknown.
     */
    public void validating(URL url, String version, long entry, long total, int overallPercent);
    
    /**
     * A JNLP client's <code>DownloadService</code> implementation should call this method
     * at least several times when applying an incremental update to an in-cache resource.  A
     * <code>DownloadServiceListener</code> implementation may display a progress bar and / or
     * update information based on the parameters.
     *
     * @param url            The URL representing the resource being patched.
     *
     * @param version        The version of the resource being patched.
     *
     * @param patchPercent   The percentage of the patch operation that is complete, or -1
     *                       if the percentage is unknown.
     *
     * @param overallPercent The percentage of the overall update operation that is complete,
     *                       or -1 if the percentage is unknown.
     */
    public void upgradingArchive(URL url, String version, int patchPercent, int overallPercent);
    
    /**
     * A JNLP client's <code>DownloadService</code> implementation should call this method
     * if a download fails or aborts unexpectedly.  In response, a
     * <code>DownloadServiceListener implementation may display update information to the user
     * to reflect this.
     *
     * @param url            The URL representing the resource for which the download failed.
     *
     * @param version        The version of the resource for which the download failed.
     */
    public void downloadFailed(URL url, String version);
}


