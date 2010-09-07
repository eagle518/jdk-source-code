/*
 * @(#)workerprotocol.h	1.18 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Message code for work requests from the Java VM process to the plug-in.
 */
#define JAVA_PLUGIN_SHOW_STATUS		0xF60001
#define JAVA_PLUGIN_SHOW_DOCUMENT	0xF60002
#define JAVA_PLUGIN_FIND_PROXY   	0xF60003
#define JAVA_PLUGIN_FIND_COOKIE         0xF60004
#define JAVA_PLUGIN_JAVASCRIPT_REQUEST  0xF60006
#define JAVA_PLUGIN_SET_COOKIE          0xF60009
#define JAVA_PLUGIN_STATUS_CHANGE       0xF6000A

/*
 * Message codes for replies
 */
#define JAVA_PLUGIN_OK			0xFB0001
#define JAVA_PLUGIN_XEMBED_TRUE		0xFB0002
#define JAVA_PLUGIN_XEMBED_FALSE	0xFB0003


