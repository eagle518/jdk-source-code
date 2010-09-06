/*
 * @(#)FileType.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import sun.plugin.resources.ResourceHandler;

/**
 * This class encapsulates all the file types that is supported
 * in caching.
 */
public class FileType
{
    // Unknown file type
    public static final int UNKNOWN = 0x0000;

    // JAR file
    public static final int JAR = 0x0001;

    // JAR JAR file
    public static final int JARJAR = 0x0002;

    // Non JAR file
    public static final int NONJAR = 0x0003;

    // CLASS file
    public static final int CLASS = 0x0011;

    // GIF file
    public static final int GIF_IMAGE = 0x0021;

    // JPEG file
    public static final int JPEG_IMAGE = 0x0022;

    // AU file
    public static final int AU_SOUND = 0x0041;

    // AU file
    public static final int WAV_SOUND = 0x0042;


    public static int getType(String ext) {
	int type = UNKNOWN;
	if(ext != null ) {
	    if(ext.equalsIgnoreCase(".jar"))
		type = JAR;
	    else 
	    if(ext.equalsIgnoreCase(".class"))
		type = CLASS;
	    else 
	    if(ext.equalsIgnoreCase(".gif"))
		type = GIF_IMAGE;
	    else
	    if(ext.equalsIgnoreCase(".jpg"))
		type = JPEG_IMAGE;
	    else
	    if(ext.equalsIgnoreCase(".au"))
		type = AU_SOUND;
	    else
	    if(ext.equalsIgnoreCase(".wav"))
		type = WAV_SOUND;
	}
	return type;
    }

    public static String getFileDescription(String ext) {
	String descr = "unknown";
	if(ext != null ) {
	    String msgStr = "cache_viewer.type";
	    msgStr = msgStr + ext.toLowerCase();
	    descr = ResourceHandler.getMessage(msgStr);
	}
	return descr;
    }
}




