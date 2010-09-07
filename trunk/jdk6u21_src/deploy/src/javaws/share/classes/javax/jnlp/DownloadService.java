/*
 * @(#)DownloadService.java	1.17 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.net.URL;
import java.io.IOException;

/**
 * <code>DownloadService</code> service allows an application
 * to control how its own resources are cached, to determine
 * which of its resources are currently cached, to force resources
 * to be cached, and to remove resources from the cache.  The JNLP
 * Client is responsible for providing a specific implementation of
 * this service.
 *
 * @since 1.0
 */

public interface DownloadService {
    
    /**
     *  Returns <code>true</code> if the resource referred to by the
     *  given URL and version is cached, and that resource is either
     *  mentioned in the calling applications JNLP file, is within the 
     *  codebase of the calling applications JNLP file, or the
     *  calling application has been granted all-permissions.
     *  .
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @return         <code>true</code> if the above conditions are
     *                  met, and <code>false</code> otherwise.
     */
    public boolean isResourceCached(URL ref, String version);
    
    /**
     *  Returns <code>true</code> if the part referred to by the
     *  given string is cached, and that part is
     *  mentioned in the JNLP file for the application.
     *
     *  @param part     The name of the part.
     *
     *  @return         <code>true</code> if the above conditions are
     *                  met, and <code>false</code> otherwise.
     */
    public boolean isPartCached(String part);
    
    /**
     *  Returns <code>true</code> if the parts referred to by the
     *  given array are cached, and those parts are
     *  mentioned in the JNLP file for the application.
     *
     *  @param parts     An array of part names.
     *
     *  @return         <code>true</code> if the above conditions are
     *                  met, and <code>false</code> otherwise.
     */
    public boolean isPartCached(String [] parts);
    
    /**
     *  Returns <code>true</code> if the given part of the given
     *  extension is cached, and the extension and part are
     *  mentioned in the JNLP file for the application.
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param part     The name of the part.
     *
     *  @return         <code>true</code> if the above conditions are
     *                  met, and <code>false</code> otherwise.
     */
    public boolean isExtensionPartCached(URL ref, String version, String part);
    
    /**
     *  Returns <code>true</code> if the given parts of the given
     *  extension are cached, and the extension and parts are
     *  mentioned in the JNLP file for the application.
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param parts     An array of part names.
     *
     *  @return         <code>true</code> if the above conditions are
     *                  met, and <code>false</code> otherwise.
     */
    public boolean isExtensionPartCached(URL ref, String version, String [] parts);
    
    /**
     *  Downloads the given resource, if the resource is either mentioned
     *  in the calling applications JNLP file, is within the codebase 
     *  of the calling applications JNLP file, or if the calling application 
     *  has been granted all-permissions.  This method will block
     *  until the download is completed or an exception occurs.
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param progress Download progress callback object.
     */
    public void loadResource(URL ref, String version, DownloadServiceListener progress)  throws IOException;
    
    /**
     *  Downloads the given part, if the part is mentioned in
     *  the JNLP file for the application.  This method will block
     *  until the download is completed or an exception occurs.
     *
     *  @param part     The name of the part.
     *
     *  @param progress Download progress callback object.
     */
    public void loadPart(String part, DownloadServiceListener progress)  throws IOException;
    
    /**
     *  Downloads the given parts, if the parts are mentioned in
     *  the JNLP file for the application.  This method will block
     *  until the download is completed or an exception occurs.
     *
     *  @param parts     An array of part names.
     *
     *  @param progress  Download progress callback object.
     */
    public void loadPart(String [] parts, DownloadServiceListener progress)  throws IOException;
    
    /**
     *  Downloads the given part of the given extension, if the part
     *  and the extension are mentioned in the JNLP file for the
     *  application.  This method will block until the download is
     *  completed or an exception occurs.
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param part     The name of the part.
     *
     *  @param progress  Download progress callback object.
     */
    public void loadExtensionPart(URL ref, String version, String part, DownloadServiceListener progress)
        throws IOException;
    
    /**
     *  Downloads the given parts of the given extension, if the parts
     *  and the extension are mentioned in the JNLP file for the
     *  application.  This method will block until the download is
     *  completed or an exception occurs.
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param parts    An array of part names to load.
     *
     *  @param progress  Download progress callback object.
     */
    public void loadExtensionPart(URL ref, String version, String [] parts, DownloadServiceListener progress)
        throws IOException;
    
    /**
     *  Removes the given resource from the cache, if the resource
     *  is either mentioned in the calling applications JNLP file, 
     *  is within the codebase of the calling applications JNLP file, 
     *  or if the calling application has been granted all-permissions.
     *
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     */
    public void removeResource(URL ref, String version) throws IOException;
    
    /**
     *  Removes the given part from the cache, if the part
     *  is mentioned in the JNLP file for the application.
     *
     *  @param part     The name of the part.
     */
    public void removePart(String part) throws IOException;
    
    /**
     *  Removes the given parts from the cache, if the parts
     *  are mentioned in the JNLP file for the application.
     *
     *  @param parts     An array of part names.
     */
    public void removePart(String [] parts) throws IOException;
    
    /**
     *  Removes the given part of the given extension from the cache,
     *  if the part and the extension are mentioned in the JNLP file
     *  for the application.
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param part     The name of the part.
     */
    public void removeExtensionPart(URL ref, String version, String part) throws IOException;
    
    /**
     *  Removes the given parts of the given extension from the cache,
     *  if the parts and the extension are mentioned in the JNLP file
     *  for the application.
     *  @param ref      The URL for the resource.
     *
     *  @param version  The version string, or <code>null</code> for
     *                  no version.
     *
     *  @param parts     An array of part names.
     */
    public void removeExtensionPart(URL ref, String version, String [] parts) throws IOException;
    
    /**
     *  Return a default {@link DownloadServiceListener} implementation which, when passed to
     *  a <code>load</code> method, should pop up and update a progress window as the load
     *  progresses.
     *
     *  @return A <code>DownloadServiceListener</code> object representing a download progress
     *           listener.
     */
    public DownloadServiceListener getDefaultProgressWindow();
    
}
