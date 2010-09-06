#include <stdio.h>

#include "CFDMonitor.h"
#include <gdk/gdk.h>

class cbdata {
public:
    int fd;
    void (*f)(void *);
    void * data;

    cbdata::cbdata(int p1, void (*p2)(void *), void * p3) {
        fd = p1;
         f = p2;
        data = p3;
    }
};

extern "C" void fdInputHandler(gpointer data, 
                               gint source, 
                               GdkInputCondition condition) {

    cbdata * foo = (cbdata * ) data;
    foo->f(foo->data);
}

void CFDMonitor::connectFD(int fd, void (*f)(void *), void * data) {

    cbdata * foo = new cbdata(fd,f,data);
    gint m_ID = gdk_input_add_full(fd,
                                   GDK_INPUT_READ,
                                   fdInputHandler,
                                   foo,
                                   NULL);
}

void CFDMonitor::disconnectFD(void * data) {

    cbdata * foo = (cbdata *) data;
    delete foo;
}

CFDMonitor::CFDMonitor() {
}


//  For ease of location, let these function always be last

JRESULT CFDMonitor::QI(const IID& iid, void ** ppv) {
    if (iid == IEgo_IID || iid == IFDMonitor_IID) {
        *ppv = static_cast<IFDMonitor *>(this);
        *ppv = NULL;
        return J_NOINTERFACE;
    }

    reinterpret_cast<IEgo *>(*ppv)->add();
    return J_NOERROR;
}

unsigned long CFDMonitor::add() {

    return ++m_refcount;
}

unsigned long CFDMonitor::release() {

    if (--m_refcount == 0) {
        delete this;
        return 0;
    }
    return m_refcount;
}

