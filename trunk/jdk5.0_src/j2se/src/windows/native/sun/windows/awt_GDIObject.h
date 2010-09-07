/*
 * @(#)awt_GDIObject.h	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_GDIOBJECT_H
#define AWT_GDIOBJECT_H

#include "awt.h"
#include "Hashtable.h"

#define MEMORY_OVER_SPEED 1

/*
 * An AwtGDIObject is a cached, color-based GDI object, such as a pen or
 * brush.
 */
class AwtGDIObject {
public:
    INLINE COLORREF GetColor() { return m_color; }
    INLINE void SetColor(COLORREF color) { m_color = color; }

    INLINE HGDIOBJ GetHandle() { return m_handle; }
    INLINE void SetHandle(HGDIOBJ handle) { m_handle = handle; }

    /*
     * NOTE: we don't syncronize access to the reference counter.
     * Currently it is changed only when we are already synchronized
     * on the global BatchDestructionManager lock.
     */
    INLINE int GetRefCount() { return m_refCount; }
    INLINE int IncrRefCount() { return ++m_refCount; }
    INLINE int DecrRefCount() { return --m_refCount; }

    /*
     * Decrement the reference count of a cached GDI object.  When it hits 
     * zero, notify the cache that the object can be safely removed.
     * The cache will eventually delete the GDI object and this wrapper.
     */
    INLINE void Release() {
#if MEMORY_OVER_SPEED
        ReleaseInCache();
#endif
    }

protected:
    /*
     * Get a GDI object from its respective cache.  If it doesn't exist
     * it gets created, otherwise its reference count gets bumped.
     */
    static AwtGDIObject* Get(COLORREF color);

    virtual void ReleaseInCache() = 0;

    INLINE AwtGDIObject() { 
        m_handle = NULL;
        m_refCount = 0;
    }

    virtual ~AwtGDIObject() {
        if (m_handle != NULL) {
            ::DeleteObject(m_handle);
        }
    }

private:
    COLORREF m_color;
    HGDIOBJ  m_handle;
    int      m_refCount;
};

#endif // AWT_GDIOBJECT_H
