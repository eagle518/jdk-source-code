/*
 * @(#)awt_Brush.cpp	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Brush.h"

GDIHashtable AwtBrush::cache("Brush cache", DeleteAwtBrush);

AwtBrush::AwtBrush(COLORREF color) {
    SetColor(color);
    HBRUSH brush = ::CreateSolidBrush(color);
    /*
     * Fix for BugTraq ID 4191297.
     * If GDI resource creation failed flush all GDIHashtables
     * to destroy unreferenced GDI resources.
     */
    if (brush == NULL) {
        cache.flushAll();
        brush = ::CreateSolidBrush(color);
    }
    DASSERT(brush != NULL);
    SetHandle(brush);
}

AwtBrush* AwtBrush::Get(COLORREF color) {

    CriticalSection::Lock l(cache.getManagerLock());

    AwtBrush* obj = static_cast<AwtBrush*>(cache.get(
        reinterpret_cast<void*>(static_cast<INT_PTR>(color))));
    if (obj == NULL) {
        obj = new AwtBrush(color);
        VERIFY(cache.put(reinterpret_cast<void*>(
            static_cast<INT_PTR>(color)), obj) == NULL);
    }
    obj->IncrRefCount();
    return obj;
}

void AwtBrush::ReleaseInCache() {

    CriticalSection::Lock l(cache.getManagerLock());

    if (DecrRefCount() == 0) {
        cache.release(reinterpret_cast<void*>(
            static_cast<INT_PTR>(GetColor())));
    }
}

void AwtBrush::DeleteAwtBrush(void* pBrush) {
    delete (AwtBrush*)pBrush;
}
