/*
 * @(#)HttpRequest.java	1.6 03/12/19
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net;
import java.net.URL;
import java.io.IOException;


/** The HttpRequest interface defines the lowest-level network primtives
 *
 *  They are simple HTTP HEAD and GET requsts
 */
public interface HttpRequest {
    // MIME types
    public static final String JNLP_MIME_TYPE     	= "application/x-java-jnlp-file";
    public static final String ERROR_MIME_TYPE    	= "application/x-java-jnlp-error";
    public static final String JAR_MIME_TYPE      	= "application/x-java-archive";
    // tomcat by default return JAR with java-archive encoding
    public static final String JAR_MIME_TYPE_EX      	= "application/java-archive";
    public static final String PACK200_MIME_TYPE  	= "application/x-java-pack200";
    public static final String JARDIFF_MIME_TYPE  	= "application/x-java-archive-diff";
    public static final String GIF_MIME_TYPE      	= "image/gif";
    public static final String JPEG_MIME_TYPE     	= "image/jpeg";
    
    // HTTP Compression RFC 2616
    public static final String GZIP_ENCODING      		= "gzip";
    public static final String PACK200_GZIP_ENCODING 	= "pack200-gzip";
    public static final String CONTENT_ENCODING         = "content-encoding";
    public static final String ACCEPT_ENCODING          = "accept-encoding";
    public static final String CONTENT_TYPE             = "content-type";
    
    public static final String DEPLOY_REQUEST_CONTENT_TYPE  = 
            "deploy-request-content-type";
    
    // Low-level interface
    HttpResponse doGetRequestEX(URL url, long lastModified) throws IOException;
    HttpResponse doGetRequestEX(URL url, String[] headerKeys, 
            String[] headerValues, long lastModified) throws IOException;
    
    HttpResponse doHeadRequest(URL url) throws IOException;
    HttpResponse doGetRequest (URL url) throws IOException;
    
    HttpResponse doHeadRequest(URL url, boolean httpCompression) throws IOException;
    HttpResponse doGetRequest (URL url, boolean httpCompression) throws IOException;
    
    HttpResponse doHeadRequest(URL url, String[] headerKeys, String[] headerValues) throws IOException;
    HttpResponse doGetRequest (URL url, String[] headerKeys, String[] headerValues) throws IOException;
    
    HttpResponse doHeadRequest(URL url, String[] headerKeys, String[] headerValues, boolean httpCompresion) throws IOException;
    HttpResponse doGetRequest (URL url, String[] headerKeys, String[] headerValues, boolean httpCompresion) throws IOException;
}


