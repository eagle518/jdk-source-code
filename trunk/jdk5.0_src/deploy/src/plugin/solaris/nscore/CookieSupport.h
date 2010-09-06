/*
 * @(#)CookieSupport.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef COOKIESUPPORT_H
#define COOKIESUPPORT_H

#define MAX_COOKIE 8192

class JavaVM5;
class JavaPluginInstance5;

class CookieSupport {
 public:
    
  CookieSupport(JavaVM5 *jvm);
  /*
   * Gets the cookies from nsICookieStorage and sends it to the requester
   */
  void FindCookieForURL(JavaPluginInstance5 *inst, const char *url);

  /*
   * Sets the cookies in nsICookieStorage from to the requester
   */
  void SetCookieForURL(const char *url, const char* cookie);

  /*
   * Replies to the requestor 
   */
  void ReplyCookie(const char *cookie, int len, int plugin_number);
  
 protected:
  
  JavaVM5* mJvm;
    
};
#endif //COOKIESUPPORT_H


