/*
 * @(#)DownloadService2.java	1.5 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import com.sun.jnlp.DownloadService2Impl;
import java.io.IOException;

/**
 * Provides cache query services to JNLP applications. Together
 * with methods in {@link DownloadService}, this allows for advanced
 * programmatic cache management.
 *
 * @since 6.0.18
 */
public interface DownloadService2 {

    /**
     * Matches all resources in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int ALL = 0;

    /**
     * Matches applications in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int APPLICATION = 1;

    /**
     * Matches applets in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int APPLET = 2;

    /**
     * Matches extensions in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int EXTENSION = 3;

    /**
     * Matches JARs in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int JAR = 4;

    /**
     * Matches image files in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int IMAGE = 5;

    /**
     * Matches class files in
     * {@link #getCachedResources} and {@link #getUpdateAvailableResources}.
     */
    public final static int CLASS = 6;

    /**
     * Specifies patterns for resource queries as arguments and holds
     * results in {@link #getCachedResources} and
     * {@link #getUpdateAvailableResources}. <br>
     *
     * For the {@code url} and {@code version} properties, standard regular
     * expressions as documented in {code java.util.regex} are supported.
     */
    public class ResourceSpec {

        static {
            DownloadService2Impl.setResourceSpecAccess(
                new DownloadService2Impl.ResourceSpecAccess() {
                public void setSize(ResourceSpec spec, long size) {
                    spec.size = size;
                }
                public void setLastModified(ResourceSpec spec, long lm) {
                    spec.lastModified = lm;
                }
                public void setExpirationDate(ResourceSpec spec, long ed) {
                    spec.expirationDate = ed;
                }
            });
        }

        private String url;

        private String version;

        private int type;

        private long size;

        private long lastModified;

        private long expirationDate;

        /**
         * Creates a new ResourceSpec instance.
         *
         * @param url the URL pattern
         * @param version the version pattern
         * @param type the resource type. 
         *        This should be one of the following constants defined in 
         *        DownloadService2: ALL, APPLICATION, APPLET, EXTENSION,
         *        JAR, IMAGE, or CLASS.
         */
        public ResourceSpec(String url, String version, int type) {
            this.url = url;
            this.version = version;
            this.type = type;
            size = -1;
        }

        /**
         * Returns the URL of this resource.
         *
         * @return the URL of this resource
         */
        public String getUrl() {
            return url;
        }

        /**
         * Returns the version of this resource.
         *
         * @return the version of this resource
         */
        public String getVersion() {
            return version;
        }

        /**
         * Returns the type of this resource.
         *
         * @return the type of this resource
         */
        public int getType() {
            return type;
        }

        /**
         * Returns the size of a resource.
         *
         * This is only useful for ResourceSpecs that have been returned
         * as a result of
         * {@link #getCachedResources} or {@link #getUpdateAvailableResources}.
         *
         * @return the size of a resource
         */
        public long getSize() {
            return size;
        }

        /**
         * Returns the time of last modification of the resource. <br>
         * The returned value has the same semantics as the return value of
         * System.currentTimeMillis(). <br>
         * A value of <code>0</code> means unknown.
         *
         * @return the time of last modification of the resource
         */
        public long getLastModified() {
            return lastModified;
        }

        /**
         * Returns the time of expiration of the resource. <br>
         * The returned value has the same semantics as the return value of
         * System.currentTimeMillis(). <br>
         * A value of <code>0</code> means unknown.
         *
         * @return the time of expiration of the resource
         */
        public long getExpirationDate() {
            return expirationDate;
        }
    }

    /**
     * Returns all resources in the cache that match one of the specified
     * resource specs. <br>
     *
     * For supported patterns in the query arguments, see
     * {@link ResourceSpec}. The returned {@code ResourceSpec} objects
     * have specific URL and version properties (i.e. no patterns). <br>
     *
     * @param spec the spec to match resources against <br>
     *
     * @return all resources that match one of the specs
     * @throws IllegalArgumentException <br> 
     *     if the ResourceSpec is null, or  <br> 
     *     if the ResourceSpec contains a null or empty URL string, or <br> 
     *     if the ResourceSpec contains invalid regular expressions. <br> 
     *     if the ResourceSpec contains a type that is not one of: <br>
     *     ALL, APPLICATION, APPLET, EXTENSION, JAR, IMAGE, or CLASS.
     */
    ResourceSpec[] getCachedResources(ResourceSpec spec);

    /**
     * Returns all resources in the cache that match one of the specified
     * resource specs AND have an update available from their server. <br>
     * 
     * For supported patterns in the query arguments, see
     * {@link ResourceSpec}. The returned {@code ResourceSpec} objects
     * have specific URL and version properties (i.e. no patterns). <br>
     * 
     * NOTE: This call may attempt HTTP GET request to check for update.
     * 
     * @param specs the spec to match resources against
     *
     * @return all resources for which an update is available that match one of
     *         the specs
     *
     * @throws IOException if something went wrong during update checks
     * @throws IllegalArgumentException <br> 
     *     if the ResourceSpec is null, or  <br> 
     *     if the ResourceSpec contains a null or empty URL string, or <br> 
     *     if the ResourceSpec contains invalid regular expressions. <br> 
     *     if the ResourceSpec contains a type that is not one of: <br>
     *     ALL, APPLICATION, APPLET, EXTENSION, JAR, IMAGE, or CLASS.
     */
    ResourceSpec[] getUpdateAvailableResources(ResourceSpec spec)
            throws IOException;

}
