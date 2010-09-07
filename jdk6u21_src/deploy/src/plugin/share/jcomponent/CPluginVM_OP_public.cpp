/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "CPluginVM_OP.h"
#include "CWriteBuffer.h"
#include "CReadBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void CPluginVM_OP::newx(JavaType jt, int argc, 
                        const char**argn, const char** argv,
                        IPluginVMCB * cb,
                        JavaID * jid) {
    fprintf(stderr,"In newx: CB = %X\n",cb);
    if (startVM()) {
    fprintf(stderr,"In newx: after startVM\n");
        if (jid != NULL) {
            while(m_pairsTable.find(m_ltstate->instance_count) != NULL) {
                m_ltstate->instance_count++;
                if (m_ltstate->instance_count == 0x00EF) {
                    m_ltstate->instance_count = 0;
                }
            }
            *jid = m_ltstate->instance_count++;
            if (m_ltstate->instance_count == 0x00EF) {
                m_ltstate->instance_count = 0;
            }
            m_pairsTable.add(*jid,cb);
            CWriteBuffer wb;
            wb.putInt(JAVA_PLUGIN_NEW);
 fprintf(stderr,"jid points at %d\n",*jid);
            wb.putInt(*jid);
            wb.putInt(jt);
            wb.putInt(argc);
            int i;
            for(i=0;i<argc;i++) {
                wb.putString(argn[i]);
                wb.putString(argv[i]);
            }
            wb.send(m_ltstate->command_pipe);    
       
            int ok;
            CReadBuffer rb(m_ltstate->command_pipe);
            rb.getInt(&ok);
            if (ok != JAVA_PLUGIN_OK) {
            }
        }
    }
}

void CPluginVM_OP::start(JavaID jid) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_START);
        wb.putInt(jid);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::stop(JavaID jid) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_STOP);
        wb.putInt(jid);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::destroy(JavaID jid) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_DESTROY);
        wb.putInt(jid);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::window(JavaID jid, int window, int width,
                          int height, int x, int y) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_WINDOW);
        wb.putInt(jid);
        wb.putInt(window);
        wb.putInt(width);
        wb.putInt(height);
        wb.putInt(x);
        wb.putInt(y);
        wb.send(m_ltstate->command_pipe);
        
        int ok;
        CReadBuffer rb(m_ltstate->command_pipe);
        rb.getInt(&ok);
        if (ok == JAVA_PLUGIN_OK) {
        }
    }
} 

void CPluginVM_OP::print(JavaID jid) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_PRINT);
    }
}

void CPluginVM_OP::docbase(JavaID jid, const char * docbase) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_DOCBASE);
        wb.putInt(jid);
        wb.putString(docbase);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::proxyMapping(char * url, char * proxy) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_PROXY_MAPPING);
        wb.putString(url);
        wb.putString(proxy);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::cookie(JavaID instance, char * c) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_COOKIE);
        wb.putInt(instance);
        wb.putString(c);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::javaScriptReply(JavaID jid, const char * reply) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_JAVASCRIPT_REPLY);
        wb.putInt(jid);
        wb.putString(reply);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::javaScriptEnd(JavaID jid) {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_JAVASCRIPT_END);
        wb.putInt(jid);
        wb.send(m_ltstate->command_pipe);
    }
}

void CPluginVM_OP::attachThread() {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_ATTACH_THREAD);
    }
}

void CPluginVM_OP::getInstanceJavaObject() {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT);
    }
}

void CPluginVM_OP::consoleShow() {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_CONSOLE_SHOW);
    }
}

void CPluginVM_OP::consoleHide() {
    if (startVM()) {
        CWriteBuffer wb;
        wb.putInt(JAVA_PLUGIN_CONSOLE_HIDE);
    }
}

void CPluginVM_OP::invalidateCB(JavaID jid) {
fprintf(stderr,"CPluginVM_OP::invalidateCB JavaID=%d\n",jid);
    m_pairsTable.remove(jid);
}

//  For ease of location, let these function always be last

JRESULT CPluginVM_OP::QI(const IID& iid, void ** ppv) {

    if (iid == IEgo_IID || iid == IPluginVM_IID) {
        *ppv = static_cast<IPluginVM *>(this);
    } else {
        *ppv = NULL;
        return J_NOINTERFACE;
    }

    reinterpret_cast<IEgo *>(*ppv)->add();
    return J_NOERROR;
}

unsigned long CPluginVM_OP::add() {

    return ++m_refcount;
}

unsigned long CPluginVM_OP::release() {

    if (--m_refcount == 0) {
        delete this;
        return 0;
    }
    return m_refcount;
}

