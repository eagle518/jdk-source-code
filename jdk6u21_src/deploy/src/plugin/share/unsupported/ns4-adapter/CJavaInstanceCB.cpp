#include <stdio.h>

#include "CJavaInstanceCB.h"
#include "ns4common.h"

void CJavaInstanceCB::showStatus(char * mess) {
        NPN_Status(m_npp,mess);
}

void CJavaInstanceCB::showDocument(char *url, char *target) {
}

void CJavaInstanceCB::findProxy(char * url, char ** proxy) {
}

void CJavaInstanceCB::findCookie(char * url, char ** cookie) {
}

void CJavaInstanceCB::javascriptRequest(char * buff) {
    NPN_GetURLNotify(m_npp, buff, NULL, (void*) JA_JSR);
}

void CJavaInstanceCB::setCookie(char * url, char * cookie) {
}

CJavaInstanceCB::CJavaInstanceCB(NPP npp) {
    m_npp = npp;
    m_refcount = 0;
}

CJavaInstanceCB::~CJavaInstanceCB() {
    fprintf(stderr,"In CJavaInstanceCB::~CJavaInstanceCB\n");
}

//  For ease of location, let these function always be last

JRESULT CJavaInstanceCB::QI(const IID& iid, void ** ppv) {
    if (iid == IEgo_IID || iid == IJavaInstanceCB_IID) {
        *ppv = static_cast<IJavaInstanceCB *>(this);
        *ppv = NULL;
        return J_NOINTERFACE;
    }

    reinterpret_cast<IEgo *>(*ppv)->add();
    return J_NOERROR;
}

unsigned long CJavaInstanceCB::add() {

    return ++m_refcount;
}

unsigned long CJavaInstanceCB::release() {

    if (--m_refcount == 0) {
        delete this;
        return 0;
    }
    return m_refcount;
}

