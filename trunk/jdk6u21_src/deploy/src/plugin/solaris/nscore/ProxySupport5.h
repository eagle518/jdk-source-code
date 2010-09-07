/*
 * @(#)ProxySupport5.h	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef PROXYSUPPORT_H
#define PROXYSUPPORT_H

#include "JavaVM5.h"
#include "IPluginInstance.h"

class ProxySupport5 {
public:
    ProxySupport5(JavaVM5 *javaVM);
    void ProxmapReply(const char *stream_url, int len, void *buffer);
    void ProxmapFindProxy(IPluginInstance *inst, char * url,
			  char * host);
protected:
    struct LongTermState* state;
    JavaVM5*              javaVM;
};

#endif

