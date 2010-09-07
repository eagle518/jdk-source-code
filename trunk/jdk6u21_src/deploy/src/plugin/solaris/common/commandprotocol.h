/*
 * @(#)commandprotocol.h	1.18 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Message codes for commands from the plug-in to the Java VM process:
 */
#define JAVA_PLUGIN_NEW				0xFA0001
#define JAVA_PLUGIN_DESTROY			0xFA0002
#define JAVA_PLUGIN_WINDOW 	 		0xFA0003
#define JAVA_PLUGIN_SHUTDOWN  			0xFA0004
#define JAVA_PLUGIN_DOCBASE  			0xFA0005
#define JAVA_PLUGIN_PROXY_MAPPING 		0xFA0007
#define JAVA_PLUGIN_COOKIE             		0xFA0008
#define JAVA_PLUGIN_JAVASCRIPT_REPLY   		0xFA000A
#define JAVA_PLUGIN_JAVASCRIPT_END  		0xFA000B
#define JAVA_PLUGIN_START			0xFA0011
#define JAVA_PLUGIN_STOP			0xFA0012
#define JAVA_PLUGIN_ATTACH_THREAD     		0xFA0013
#define JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED 0xFA0014
#define JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT    0xFA0015
#define JAVA_PLUGIN_PRINT              		0xFA0016
#define JAVA_PLUGIN_CONSOLE_SHOW		0xFA0019
#define JAVA_PLUGIN_CONSOLE_HIDE		0xFA001A
#define JAVA_PLUGIN_QUERY_XEMBED		0xFA001B

