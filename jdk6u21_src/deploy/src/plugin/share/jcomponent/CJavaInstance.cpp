/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include "CJavaInstance.h"
#include <stdio.h>


void CJavaInstance::start() {
    m_vm->start(m_jid);
}

void CJavaInstance::stop() {
    m_vm->stop(m_jid);
}

void CJavaInstance::destroy() {
    m_vm->destroy(m_jid);
}

void CJavaInstance::window(int win, int width, int height, int x, int y) {
    m_vm->window(m_jid,win,width,height,x,y);
}

void CJavaInstance::javascriptReply(const char * reply) {
    if(reply != NULL) {
        m_vm->javaScriptReply(m_jid,reply);
    } else {
        m_vm->javaScriptEnd(m_jid);
    }
}

void CJavaInstance::docbase(const char * db) {
    m_vm->docbase(m_jid, db);
}

CJavaInstance::CJavaInstance(IPluginVM * vm, 
                             int argc, const char** argn,const char **argv,
                             IJavaInstanceCB * jicb) {
    m_refcount = 0;
    m_vm = vm;
    m_jicb = jicb;
    m_jicb->add();

    m_vm->newx(IPluginVM::aPPLET, argc, argn, argv, (IPluginVMCB *) this, &m_jid);
    m_vm->start(m_jid);
}

CJavaInstance::~CJavaInstance() {
    fprintf(stderr,"In CJavaInstance::~CJavaInstance()\n");
    m_vm->invalidateCB(m_jid);
    m_jicb->release();
}

// These are the things the VM can ask of us.
void CJavaInstance::showStatus(char * mess) {

    if (m_jicb != NULL) {
        m_jicb->showStatus(mess);
    }
}

void CJavaInstance::showDocument(char * url, char * target) {

    if (m_jicb != NULL) {
        m_jicb->showDocument(url,target);
    }
}

int CJavaInstance::findProxy(char * url, char ** proxy) {

    if (m_jicb != NULL) {
        m_jicb->findProxy(url,proxy);
    } else return -1;

    return 0;
}

void CJavaInstance::findCookie(char * url, char ** cookie) {

    if (m_jicb != NULL) {
        m_jicb->findCookie(url, cookie);
    }
}

int CJavaInstance::javascriptRequest(char * value) {

    if (m_jicb != NULL) {
        m_jicb->javascriptRequest(value);
    } else {
        return -1;
    }
    return 0;
}

void CJavaInstance::setCookie(char * url, char *cookie) {

    if (m_jicb != NULL) {
        m_jicb->setCookie(url,cookie);
    }
}

//  For ease of location, let these function always be last

JRESULT CJavaInstance::QI(const IID& iid, void ** ppv) {
    if (iid == IEgo_IID || iid == IJavaInstance_IID) {
        *ppv = static_cast<IJavaInstance *>(this);
    } else if (iid == IPluginVMCB_IID) {
      *ppv = static_cast<IPluginVMCB *>(this);
    } else {
        *ppv = NULL;
        return J_NOINTERFACE;
    }

    reinterpret_cast<IEgo *>(*ppv)->add();
    return J_NOERROR;
}

unsigned long CJavaInstance::add() {

    return ++m_refcount;
}

unsigned long CJavaInstance::release() {

    if (--m_refcount == 0) {
        delete this;
        return 0;
    }
    return m_refcount;
}

