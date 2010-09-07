/*
 * @(#)CookieSupport.cpp	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include "commonhdr.h"
#include "ICookieStorage.h"
#include "CookieSupport.h"
#include "commandprotocol.h"
#include "JavaVM5.h"
#include "JavaPluginInstance5.h"
#include "JavaPluginFactory5.h"
#include "CWriteBuffer.h"

CookieSupport::CookieSupport(JavaVM5 *jvm) : mJvm(jvm)
{
}

/*
 * Gets the cookies from nsICookieStorage and sends it to the requester
 */
void CookieSupport::FindCookieForURL(JavaPluginInstance5 *inst, 
				     const char *url){

  JDUint32 cookieSize =  MAX_COOKIE;
  char cookieResult[MAX_COOKIE];
  ICookieStorage *cookieStorage=mJvm->GetPluginFactory()->GetCookieStorage();
  if (cookieStorage->GetCookie(url, 
			       (void*) cookieResult, 
			       cookieSize ) == JD_OK)
    {
      ReplyCookie(cookieResult, cookieSize, inst->GetPluginNumber());
    }
    else
    {
      /* The applet is waiting on the pipe for a reply.  We must provide one!! */
      ReplyCookie(" ",1,inst->GetPluginNumber());
    }
  return;
}

/*
* Sets the cookies in nsICookieStorage from to the requester
*/
void CookieSupport::SetCookieForURL(const char *url, const char* cookie)
{
    ICookieStorage *cookieStorage=mJvm->GetPluginFactory()->GetCookieStorage();

    // Set the cookie in the browser
    cookieStorage->SetCookie(url, (void*) cookie, strlen(cookie));

    return;
}

/*
 * Replies to cookie requestor message
 */
void CookieSupport::ReplyCookie(const char *cookie, int len, int pnum){

    CWriteBuffer wb;
    trace("CookieSupport reply for an original JAVA_PLUGIN_COOKIE_REQUEST\n");
    wb.putInt(JAVA_PLUGIN_COOKIE);
    wb.putInt(pnum);
    wb.putString(cookie);
    mJvm->SendRequest(wb, FALSE);
}
