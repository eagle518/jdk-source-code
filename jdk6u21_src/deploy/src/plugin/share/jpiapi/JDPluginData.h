/*
 * @(#)JDPluginData.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  JDPluginData.h  by X.Lu
 * 
 *
 * Definetion of various plug-in data type and macros 
 */

#ifndef _JDPLUGINDATA_H_
#define _JDPLUGINDATA_H_

#include "JDData.h"

#ifdef XP_UNIX
#include <X11/Xlib.h>
#endif
/* Plugin specific data structures imported from nsplugindefs.h	*/
enum JDPluginWindowType {
	JDPluginWindowType_Window = 1,
	JDPluginWindowType_Drawable
};

enum JDPluginVariable {
     JDPluginVariable_NameString                      = 1,
     JDPluginVariable_DescriptionString               = 2
};

enum JDPluginManagerVariable {
     JDPluginManagerVariable_XDisplay                 = 1,
     JDPluginManagerVariable_XtAppContext             = 2,
     JDPluginManagerVariable_SupportsXEmbed           = 14 
};

struct JDPluginRect {
    JDUint16	top;
    JDUint16	left;
    JDUint16	bottom;
    JDUint16	right;
};

struct JDPluginPort;
struct JDPluginWindow {
    JDPluginPort* window; 
    JDint32		  x;
    JDint32		  y;
    JDUint32	  width;
    JDUint32	  height;
    JDPluginRect  clipRect;
#ifdef XP_UNIX
    void*         ws_info;
#endif
    JDPluginWindowType type;
};

#ifdef XP_UNIX
#ifndef NO_X11
struct JDPluginSetWindowCallbackStruct {
     JDint32     type;
     Display*    display;
     Visual*     visual;
     Colormap    colormap;
     JDUint32    depth;
};
#else
struct JDPluginSetWindowCallbackStruct {
     JDint32     type;
};
#endif

#include <fcntl.h>
#include <stdio.h>

struct JDPluginPrintCallbackStruct {
  JDint32     type;
  FILE*       fp;
};
#endif

struct JDPluginFullPrint {
	JDBool 	pluginPrinted;
	JDBool  printOne;

	void*   platformPrint;
};

struct JDPluginEmbedPrint {
	JDPluginWindow	window;
	void*			platformPrint;
};

struct JDPluginPrint {
	JDUint16		mode;
	union
	{
		JDPluginFullPrint	fullPrint;
		JDPluginEmbedPrint	embedPrint;
	}print;
};


enum JDPluginMode {
    JDPluginMode_Embedded = 1,
    JDPluginMode_Full
};


typedef const char*		JDPluginMimeType;

enum JDPluginReason {
     JDPluginReason_Base = 0,
     JDPluginReason_Done = 0,
     JDPluginReason_NetworkErr,
     JDPluginReason_UserBreak,
     JDPluginReason_NoReason
};

struct JDPluginEvent {
  JDUint32 event;
  JDUint32 wParam;
  JDUint32 lParam;
};

enum JDPluginStreamType {
    JDPluginStreamType_Normal = 1,
    JDPluginStreamType_Seek,
    JDPluginStreamType_AsFile,
    JDPluginStreamType_AsFileOnly
};

#endif /*_JDPLUGINDATA_H_*/
