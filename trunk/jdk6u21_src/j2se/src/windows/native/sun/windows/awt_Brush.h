/*
 * @(#)awt_Brush.h	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_BRUSH_H
#define AWT_BRUSH_H

#include "awt_GDIObject.h"
#include "GDIHashtable.h"

/*
 * An AwtBrush is a cached Windows brush.
 */
class AwtBrush : public AwtGDIObject {
public:
    /*
     * Get a GDI object from its respective cache.  If it doesn't exist
     * it gets created, otherwise its reference count gets bumped.
     */
    static AwtBrush* Get(COLORREF color);

    // Delete an AwtBrush, called by Hashtable.clear().
    static void DeleteAwtBrush(void* pBrush);

protected:
    /*
     * Decrement the reference count of a cached GDI object.  When it hits 
     * zero, notify the cache that the object can be safely removed.
     * The cache will eventually delete the GDI object and this wrapper.
     */
    virtual void ReleaseInCache();

private:
    AwtBrush(COLORREF color);
    ~AwtBrush() {}

    static GDIHashtable cache;
};

#endif // AWT_BRUSH_H
