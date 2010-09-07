/*
 * @(#)awt_Pen.cpp	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Pen.h"

GDIHashtable AwtPen::cache("Pen cache", DeleteAwtPen);

AwtPen::AwtPen(COLORREF color) {
    SetColor(color);
    HPEN pen = ::CreatePen(PS_SOLID, 1, color);
    /*
     * Fix for BugTraq ID 4191297.
     * If GDI resource creation failed flush all GDIHashtables
     * to destroy unreferenced GDI resources.
     */
    if (pen == NULL) {
        cache.flushAll();
        pen = ::CreatePen(PS_SOLID, 1, color);
    }
    DASSERT(pen != NULL);
    SetHandle(pen);
}

AwtPen* AwtPen::Get(COLORREF color) {

    CriticalSection::Lock l(cache.getManagerLock());

    AwtPen* obj = static_cast<AwtPen*>(cache.get(
        reinterpret_cast<void*>(static_cast<INT_PTR>(color))));
    if (obj == NULL) {
        obj = new AwtPen(color);
        VERIFY(cache.put(
            reinterpret_cast<void*>(static_cast<INT_PTR>(color)),
            obj) == NULL);
    }
    obj->IncrRefCount();
    return obj;
}

void AwtPen::ReleaseInCache() {

    CriticalSection::Lock l(cache.getManagerLock());

    if (DecrRefCount() == 0) {
        cache.release(
            reinterpret_cast<void*>(static_cast<INT_PTR>(GetColor())));
    }
}

void AwtPen::DeleteAwtPen(void* pPen) {
    delete (AwtPen*)pPen;
}
