/*
 * @(#)msgString.h	1.7 05/05/18
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _MSGSTRING_H
#define _MSGSTRING_H

#define MSG_BADMSG 0
#define KEY_BADMSG "error.internal.badmsg"
#define STR_BADMSG "internal error, unknown message"

#define MSG_BADINST_NOCFG 1
#define KEY_BADINST_NOCFG "error.badinst.nocfg"
#define STR_BADINST_NOCFG "Bad installation. Could not located javaws.cfg file"

#define MSG_BADINST_NOJRE 2
#define KEY_BADINST_NOJRE "error.badinst.nojre"
#define STR_BADINST_NOJRE "Bad installation. No JRE found in configuration file"

#define MSG_BADINST_EXECV 3
#define KEY_BADINST_EXECV "error.badinst.execv"
#define STR_BADINST_EXECV "Bad installation. Error invoking Java VM (execv)"

#define MSG_BADINST_SYSEXE 4
#define KEY_BADINST_SYSEXE "error.badinst.sysexec"
#define STR_BADINST_SYSEXE "Bad installation. Error invoking Java VM (SysExec)"

#define MSG_LISTENER_FAILED 5
#define KEY_LISTENER_FAILED "error.listener.failed"
#define STR_LISTENER_FAILED "Splash: sysCreateListenerSocket failed"

#define MSG_ACCEPT_FAILED 6
#define KEY_ACCEPT_FAILED "error.accept.failed"
#define STR_ACCEPT_FAILED "Splash: accept failed"

#define MSG_RECV_FAILED 7
#define KEY_RECV_FAILED "error.recv.failed"
#define STR_RECV_FAILED "Splash: recv failed"

#define MSG_INVALID_PORT 8
#define KEY_INVALID_PORT "error.invalid.port"
#define STR_INVALID_PORT "Splash: didn't revive a valid port"

#define MSG_READ_ERROR 9
#define KEY_READ_ERROR "error.read"
#define STR_READ_ERROR "Read past end of buffer"

#define MSG_XMLPARSE_ERROR 10
#define KEY_XMLPARSE_ERROR "error.xmlparsing"
#define STR_XMLPARSE_ERROR "XML Parsing error: wrong kind of token found"

/* from win32 system_md.c: */
#define MSG_SPLASH_EXIT 11
#define KEY_SPLASH_EXIT "error.splash.exit"
#define STR_SPLASH_EXIT "Java Web Start splash screen process exiting ...\n"

#define MSG_WSA_ERROR 12
#define KEY_WSA_ERROR "error.winsock"
#define STR_WSA_ERROR "\tLast WinSock Error: "

#define MSG_WSA_LOAD 13
#define KEY_WSA_LOAD "error.winsock.load"
#define STR_WSA_LOAD "Couldn't load winsock.dll"

#define MSG_WSA_START 14
#define KEY_WSA_START "error.winsock.star"
#define STR_WSA_START "WSAStartup failed"

/* from unix system_md.c: */
#define MSG_BADINST_NOHOME 15
#define KEY_BADINST_NOHOME "error.badinst.nohome"
#define STR_BADINST_NOHOME "Bad installation: JAVAWS_HOME not set" 

/* from splash.c: */
#define MSG_SPLASH_NOIMAGE 16
#define KEY_SPLASH_NOIMAGE "error.splash.noimage"
#define STR_SPLASH_NOIMAGE "Splash: couldn't load splash screen image"

#define MSG_SPLASH_SOCKET 17
#define KEY_SPLASH_SOCKET "error.splash.socket"
#define STR_SPLASH_SOCKET "Splash: server socket failed"

#define MSG_SPLASH_CMND 18
#define KEY_SPLASH_CMND "error.splash.cmnd"
#define STR_SPLASH_CMND "Splash: unrecognized command"

#define MSG_SPLASH_PORT 19
#define KEY_SPLASH_PORT "error.splash.port"
#define STR_SPLASH_PORT "Splash: port not specified"

#define MSG_SPLASH_SEND 20
#define KEY_SPLASH_SEND "error.splash.send"
#define STR_SPLASH_SEND "Splash: send failed"

#define MSG_SPLASH_TIMER 21
#define KEY_SPLASH_TIMER "error.splash.timer"
#define STR_SPLASH_TIMER "Splash: couldn't create shutdown timer"

#define MSG_SPLASH_X11_OPEN 22
#define KEY_SPLASH_X11_OPEN "error.splash.x11.open"
#define STR_SPLASH_X11_OPEN "Splash: Can't open X11 display"

#define MSG_SPLASH_X11_CONNECT 23
#define KEY_SPLASH_X11_CONNECT "error.splash.x11.connect"
#define STR_SPLASH_X11_CONNECT "Splash: X11 connection failed"

#define MSG_JAVAWS_USAGE 24
#define KEY_JAVAWS_USAGE "message.javaws.usage"
#define STR_JAVAWS_USAGE \
"\nUsage:\tjavaws [run-options] <jnlp-file>	"\
"\n      \tjavaws [control-options]"\
"\n	"\
"\nwhere run-options include:"\
"\n  -verbose       \tdisplay additional output"\
"\n  -offline       \trun the application in offline mode"\
"\n  -system        \trun the application from the system cache only"\
"\n  -Xnosplash     \trun without showing a splash screen"\
"\n  -J <options>   \tsupply options to the vm"\
"\n  -wait          \tstart java process and wait for its exit"\
"\n	"\
"\ncontrol-options include:"\
"\n  -viewer        \tshow the cache viewer in the java control panel"\
"\n  -uninstall     \tremove all applications from the cache"\
"\n  -uninstall <jnlp-file>              \tremove the application from the cache"\
"\n  -import [import-options] <jnlp-file>\timport the application to the cache"\
"\n	"\
"\nimport-options include:"\
"\n  -silent        \timport silently (with no user interface)"\
"\n  -system        \timport application into the system cache"\
"\n  -codebase <url>\tretrieve resources from the given codebase"\
"\n  -shortcut      \tinstall shortcuts as if user allowed prompt"\
"\n  -association   \tinstall associations as if user allowed prompt"\
"\n\n"

typedef struct _msgEntry {
  int id;
  char *key;
  char *message;
} msgEntry;

extern char *getMsgString(int messageID);

#endif

