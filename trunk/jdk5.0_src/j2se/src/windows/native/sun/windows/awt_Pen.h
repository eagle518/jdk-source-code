/*
 * @(#)awt_Pen.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_PEN_H
#define AWT_PEN_H

#include "awt_GDIObject.h"
#include "GDIHashtable.h"

/*
 * An AwtPen is a cached Windows pen.
 */
class AwtPen : public AwtGDIObject {
public:
    /*
     * Get a GDI object from its respective cache.  If it doesn't exist
     * it gets created, otherwise its reference count gets bumped.
     */
    static AwtPen* Get(COLORREF color);

    // Delete an AwtPen, called by Hashtable.clear().
    static void DeleteAwtPen(void* pPen);

protected:
    /*
     * Decrement the reference count of a cached GDI object.  When it hits 
     * zero, notify the cache that the object can be safely removed.
     * The cache will eventually delete the GDI object and this wrapper.
     */
    virtual void ReleaseInCache();

private:
    AwtPen(COLORREF color);
    ~AwtPen() {}

    static GDIHashtable cache;
};

#endif // AWT_PEN_H
