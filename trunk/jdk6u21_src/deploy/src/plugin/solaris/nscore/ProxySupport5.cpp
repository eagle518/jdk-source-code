/*
 * @(#)ProxySupport5.cpp	1.24 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * This module provides support for querying for proxy URLs from the browser.
 * In the intialization phase the auto proxy URL is first found from the
 * netscape preferences file. state->proxy_config_url.
 * A whole bunch of javascript functions along with any responses from
 * asking for the autoproxy url from the host are stored in 
 *  state->proxy_script, which is then used together with the javascript
 * command javascript:FindProxyForURL to get the proxy of a URL.
 * The response comes in via ProxmapReply.
 */
#include "commonhdr.h"
#include "IPluginManager.h"
#include "IPluginInstance.h"
#include "JavaPluginFactory5.h"
#include "JavaVM5.h"
#include "CWriteBuffer.h"
#include "commandprotocol.h"
#include "ProxySupport5.h"

extern "C" {
#ifdef __linux__
extern int gethostname(char *, size_t);
#else
extern int gethostname(char *, int);
#endif
}

ProxySupport5::ProxySupport5(JavaVM5* javaVM_arg) {
    javaVM = javaVM_arg;
}

/*
 * This function will eventually provide the proxy for a given URL
 * This sends a JAVA_PLUGIN_PROXMAP_QUERY to the peer with a javascript
 * command 
 *     javascript:FindProxyForURL(<url>,<host>)
 * The GetURL will eventually return via a JavaStream5 which,  calls
 * ProxmapReply.
 * It is invoked by the Worker thread via the JavaVM5, through the 
 * JAVA_PLUGIN_FIND_PROXY command. 
 */
void ProxySupport5::ProxmapFindProxy(IPluginInstance *inst,
				     char * url,
				     char * host) {
  /* First issue (or reissue) the script definition.  Then issue the
   * specific mapping request * We'll get an answer later through a
   * stream write * redircted through to proxmap_reply
   */
  
  UNUSED(host);

  char *proxyResult;
  
  IPluginManager *mgr = javaVM->GetPluginFactory()->GetPluginManager();
  
  if (mgr->FindProxyForURL(url , &proxyResult) != JD_OK)
     { 
       // If fails to retrieve proxy, default to DIRECT 
       // 
       proxyResult = (char *) malloc(sizeof(char) * 7); 
       sprintf(proxyResult, "DIRECT"); 
     }

  ProxmapReply(url, strlen(proxyResult), proxyResult);

  free((void *)proxyResult);

  UNUSED(inst);

}

/* 
 * Called by the browser (via a stream) with the response of a proxy request
 * from a worker thread
 * The chain of events is:
 *   Worker thread -> JAVA_PLUGIN_FIND_PROXY to plugin
 *   JavaVM5::doWork  -> ProxySupport5->ProxmapFindProxy
 *   ProxmapFindProxy -> GetURL javascript:findproxyforurl...
 *                        -> Browser  JAVA_PLUGIN_PROXMAP_QUERY
 *.  Browser -> JavaPluginInstance new Stream
 *   Browser -> Stream write    JAVA_PLUGIN_PROXMAP_QUERY
 *   New stream write -> ProxySupport5.ProxmapReply
 */
void ProxySupport5::ProxmapReply(const char *stream_url, int len, 
				 void *buffer)
{
    CWriteBuffer wb;

    /*
     * Figure out the URL that was being resolved.
     */
    trace("Proxmap reply for an original JAVA_PLUGIN_PROXY_REQUEST");
    trace("Mapping for proxy : %s ", stream_url);
    wb.putInt(JAVA_PLUGIN_PROXY_MAPPING);
    wb.putString(stream_url);
    wb.putString((const char *) buffer,len);
    javaVM->SendRequest(wb, FALSE);
}

