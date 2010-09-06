/*
 * @(#)HttpRequest.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.net.URL;
import java.io.IOException;


/** The HttpRequest interface defines the lowest-level
 *  network primtives for Java Web Start.
 *
 *  They are simple HTTP HEAD and GET requsts
 */
public interface HttpRequest {
    // MIME types
    public static final String JNLP_MIME_TYPE     	= "application/x-java-jnlp-file";
    public static final String ERROR_MIME_TYPE    	= "application/x-java-jnlp-error";
    public static final String JAR_MIME_TYPE      	= "application/x-java-archive";
    public static final String PACK200_MIME_TYPE  	= "application/x-java-pack200";
    public static final String JARDIFF_MIME_TYPE  	= "application/x-java-archive-diff";
    public static final String GIF_MIME_TYPE      	= "image/gif";
    public static final String JPEG_MIME_TYPE     	= "image/jpeg";
    
    // HTTP Compression RFC 2616
    public static final String GZIP_ENCODING      	= "gzip";
    public static final String PACK200_GZIP_ENCODING 	= "pack200-gzip";
    public static final String CONTENT_ENCODING         = "content-encoding";
    public static final String ACCEPT_ENCODING          = "accept-encoding";
    public static final String CONTENT_TYPE             = "content-type";
    
    // Low-level interface
    HttpResponse doHeadRequest(URL url) throws IOException;
    HttpResponse doGetRequest (URL url) throws IOException;
    
    HttpResponse doHeadRequest(URL url, boolean httpCompression) throws IOException;
    HttpResponse doGetRequest (URL url, boolean httpCompression) throws IOException;
    
    HttpResponse doHeadRequest(URL url, String[] headerKeys, String[] headerValues) throws IOException;
    HttpResponse doGetRequest (URL url, String[] headerKeys, String[] headerValues) throws IOException;
    
    HttpResponse doHeadRequest(URL url, String[] headerKeys, String[] headerValues, boolean httpCompresion) throws IOException;
    HttpResponse doGetRequest (URL url, String[] headerKeys, String[] headerValues, boolean httpCompresion) throws IOException;
}


