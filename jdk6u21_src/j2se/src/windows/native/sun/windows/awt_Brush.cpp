/*
 * @(#)awt_Brush.cpp	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Brush.h"

GDIHashtable AwtBrush::cache("Brush cache", DeleteAwtBrush);

AwtBrush::AwtBrush(COLORREF color) {
    if (!EnsureGDIObjectAvailability()) {
        // If we've run out of GDI objects, don't try to create
        // a new one
        return;
    }
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
    if (brush == NULL) {
        // We've already incremented the counter: decrement if
        // creation failed
        Decrement();
    }
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
