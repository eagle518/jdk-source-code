/*
 * @(#)msgString.c	1.13 05/05/20
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include "system.h" 
#include "propertyParser.h" 
#include "util.h" 
  
static PropertyFileEntry* MsgFileHead = NULL; 

/*
 * Message localization routine: getMsgString()
 */

msgEntry messages[] = {
  {0, KEY_BADMSG, STR_BADMSG},               /* no zero id */
  {MSG_BADINST_NOCFG, KEY_BADINST_NOCFG, STR_BADINST_NOCFG},
  {MSG_BADINST_NOJRE, KEY_BADINST_NOJRE, STR_BADINST_NOJRE},
  {MSG_BADINST_EXECV, KEY_BADINST_EXECV, STR_BADINST_EXECV},
  {MSG_BADINST_SYSEXE, KEY_BADINST_SYSEXE, STR_BADINST_SYSEXE},
  {MSG_LISTENER_FAILED, KEY_LISTENER_FAILED, STR_LISTENER_FAILED},
  {MSG_ACCEPT_FAILED, KEY_ACCEPT_FAILED, STR_ACCEPT_FAILED},
  {MSG_RECV_FAILED, KEY_RECV_FAILED, STR_RECV_FAILED},
  {MSG_INVALID_PORT, KEY_INVALID_PORT, STR_INVALID_PORT},
  {MSG_READ_ERROR, KEY_READ_ERROR, STR_READ_ERROR},
  {MSG_XMLPARSE_ERROR, KEY_XMLPARSE_ERROR, STR_XMLPARSE_ERROR},
  {MSG_SPLASH_EXIT, KEY_SPLASH_EXIT, STR_SPLASH_EXIT},
  {MSG_WSA_ERROR, KEY_WSA_ERROR, STR_WSA_ERROR},
  {MSG_WSA_LOAD, KEY_WSA_LOAD, STR_WSA_LOAD},
  {MSG_WSA_START, KEY_WSA_START, STR_WSA_START},
  {MSG_BADINST_NOHOME, KEY_BADINST_NOHOME, STR_BADINST_NOHOME},
  {MSG_SPLASH_NOIMAGE, KEY_SPLASH_NOIMAGE, STR_SPLASH_NOIMAGE},
  {MSG_SPLASH_SOCKET, KEY_SPLASH_SOCKET, STR_SPLASH_SOCKET},
  {MSG_SPLASH_CMND, KEY_SPLASH_CMND, STR_SPLASH_CMND},
  {MSG_SPLASH_PORT, KEY_SPLASH_PORT, STR_SPLASH_PORT},
  {MSG_SPLASH_SEND, KEY_SPLASH_SEND, STR_SPLASH_SEND},
  {MSG_SPLASH_TIMER, KEY_SPLASH_TIMER, STR_SPLASH_TIMER},
  {MSG_SPLASH_X11_OPEN, KEY_SPLASH_X11_OPEN, STR_SPLASH_X11_OPEN},
  {MSG_SPLASH_X11_CONNECT, KEY_SPLASH_X11_CONNECT, STR_SPLASH_X11_CONNECT},
  {MSG_JAVAWS_USAGE,KEY_JAVAWS_USAGE,STR_JAVAWS_USAGE}
};

void initializeMessages(char *localeString) {
    char msgFileName[MAXPATHLEN];

    /*  construct name of localized message property file, such as:
     *  C:\Program Files\Java Web Start\resources\messages_en_US.properties
     */
    sysStrNPrintF(msgFileName, sizeof(msgFileName), "%s%c%s%s%s",
	sysGetJavawsResourcesLib(),
	FILE_SEPARATOR,
	"messages_",
        localeString,
	".properties");

    MsgFileHead = parsePropertyFile(msgFileName, NULL);

    if (MsgFileHead == NULL) {
      char *p = 0;
      if ((p = strrchr(localeString, '_')) != NULL) {
	*p=0;
      }
      sysStrNPrintF(msgFileName, sizeof(msgFileName), "%s%c%s%s%s",
	      sysGetJavawsResourcesLib(),	   
	      FILE_SEPARATOR,
	      "messages_",
	      localeString,
	      ".properties");
    }
      
    MsgFileHead = parsePropertyFile(msgFileName, NULL);

    if (MsgFileHead == NULL) {
        /* OK - no localized file for this local, try non-localized:
         *      C:\Program Files\Java Web Start\resources\messages.properties
          *
         */
        sysStrNPrintF(msgFileName, sizeof(msgFileName), "%s%c%s",
            sysGetJavawsResourcesLib(),   
            FILE_SEPARATOR, 
            "messages.properties");

        MsgFileHead = parsePropertyFile(msgFileName, NULL);
    }
    if (MsgFileHead == NULL) {
	/* now we have classic - double fault - fatal error generating error msg
         * we have to abort w/o translated message
	 */
	
	Abort("Can not find message file");
    }
}

static int msgs_initialized = FALSE;
static int msgs_inProgress = FALSE;

size_t DecodeMessage(char *value, twchar_t *wVal) {
  char *p = value;
  int index = 0;
  unsigned int hex = 0;
  while(*p) {
    if (*p!='\\') {
      wVal[index++] = (twchar_t)*p++;
    } else {
      p++;p++;
      sscanf(p, "%4x", &hex);
      p+=4;
      wVal[index++] = (twchar_t)hex;
    }
  }
  wVal[index] = 0; wVal[index+1] = 0;
  return index;
}

char *getMsgString(int messageID) {
    char *value, *v2 = NULL;
    char *key = messages[0].key;
    char *defaultMessage = messages[0].message;
    int i, len, len2;
    twchar_t wArray[MAXSTRINGLEN];

    len = sizeof(messages)/sizeof(messages[0]);
    for (i=0; i<len; i++) {
	if (messages[i].id == messageID) {
	    key = messages[i].key;
	    defaultMessage = messages[i].message;
            break;
        }
    }

    if (msgs_inProgress) {
	/* double fault - error generating error msg return default*/
	return defaultMessage;
    }
    msgs_inProgress = TRUE;

    if (!msgs_initialized) {
	initializeMessages(sysGetLocaleStr());
	msgs_initialized = TRUE;
    }

    value = GetPropertyValue(MsgFileHead, key);
    if (value == NULL) {
	value = defaultMessage;
    }
    len2 = DecodeMessage(value, wArray);
    v2 = sysWideCharToMBCS(wArray, len2);

	msgs_inProgress = FALSE;

    if (v2 == NULL) {
      return defaultMessage;
    } else {
      return v2;
    }
}
