/*
 * @(#)X11SurfaceData.h	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "SurfaceData.h"

#include "awt_p.h"
#include "awt_GraphicsEnv.h"

#include <jdga.h>

/**
 * This include file contains support declarations for loops using the
 * X11 extended SurfaceData interface to talk to an X11 drawable from
 * native code.
 */

#ifdef HEADLESS
#define X11SDOps void
#else /* HEADLESS */
typedef struct _X11SDOps X11SDOps;

/*
 * This function retrieves an X11 GC for rendering to the destination
 * managed by the indicated X11SDOps structure.
 *
 * The env parameter should be the JNIEnv of the surrounding JNI context.
 *
 * The ops parameter should be a pointer to the ops object upon which
 * this function is being invoked.
 *
 * The clip parameter should be a pointer to a rectangle indicating the
 * desired clip.
 *
 * The comp parameter should be a pointer to a Composite object, or NULL
 * which means the Src (default) compositing rule will be used.
 *
 * The pixel parameter should be the pixel value for the color that will
 * be used for rendering.
 *
 * The ReleaseGC function should be called to release the lock on the GC
 * after a given atomic set of rendering operations is complete.
 *
 * Note to callers:
 *	This function may use JNI methods so it is important that the
 *	caller not have any outstanding GetPrimitiveArrayCritical or
 *	GetStringCritical locks which have not been released.
 *
 *	The caller may continue to use JNI methods after this method
 *	is called since this function will not leave any outstanding
 *	JNI Critical locks unreleased.
 */
typedef GC GetGCFunc(JNIEnv *env,
		     X11SDOps *xsdo,
		     jobject clip,
		     jobject comp,
		     jint pixel);

/*
 * This function releases an X11 GC that was retrieved from the GetGC
 * function of the indicated X11SDOps structure.
 *
 * The env parameter should be the JNIEnv of the surrounding JNI context.
 *
 * The ops parameter should be a pointer to the ops object upon which
 * this function is being invoked.
 *
 * The xgc parameter should be the handle to the GC object that was
 * returned from the GetGC function.
 *
 * Note to callers:
 *	This function may use JNI methods so it is important that the
 *	caller not have any outstanding GetPrimitiveArrayCritical or
 *	GetStringCritical locks which have not been released.
 *
 *	The caller may continue to use JNI methods after this method
 *	is called since this function will not leave any outstanding
 *	JNI Critical locks unreleased.
 */
typedef void ReleaseGCFunc(JNIEnv *env,
			   X11SDOps *xsdo,
			   GC xgc);

/*
 * This function returns an X11 Drawable which transparent pixels
 * (if there are any) were set to the specified color.
 *
 * The env parameter should be the JNIEnv of the surrounding JNI context.
 *
 * The xsdo parameter should be a pointer to the ops object upon which
 * this function is being invoked.
 *
 * The pixel parameter should be a color to which the transparent
 * pixels of the image should be se set to.
 */
typedef Drawable GetPixmapBgFunc(JNIEnv *env,
				 X11SDOps *xsdo,
				 jint pixel);

/*
 * This function releases the lock set by GetPixmapBg
 * function of the indicated X11SDOps structure.
 *
 * The env parameter should be the JNIEnv of the surrounding JNI context.
 *
 * The ops parameter should be a pointer to the ops object upon which
 * this function is being invoked.
 */
typedef void ReleasePixmapBgFunc(JNIEnv *env,
				 X11SDOps *xsdo);


#ifdef MITSHM
typedef struct {
    XShmSegmentInfo     *shmSegInfo;    /* Shared Memory Segment Info */
    jint                bytesPerLine;   /* needed for ShMem lock */
    jboolean            xRequestSent;   /* true if x request is sent w/o XSync */
    jint                pmSize;

    jboolean		usingShmPixmap;
    Drawable		pixmap;
    Drawable		shmPixmap;
    jint		numBltsSinceRead;
    jint		pixelsReadSinceBlt;
    jint		pixelsReadThreshold;
    jint		numBltsThreshold;
} ShmPixmapData;
#endif /* MITSHM */

struct _X11SDOps {
    SurfaceDataOps	sdOps;
    GetGCFunc		*GetGC;
    ReleaseGCFunc	*ReleaseGC;
    GetPixmapBgFunc     *GetPixmapWithBg;
    ReleasePixmapBgFunc	*ReleasePixmapWithBg;
    jboolean		invalid;
    jboolean		isPixmap;
    jobject		peer;
    Drawable		drawable;
    Widget              widget;
    GC			gc;
    jint		lastpixel;
    jobject		lastclip;
    jobject             lastcomp;
    jint                lastxorpixel;
    jint		depth;
    jint		pixelmask;
    JDgaSurfaceInfo	surfInfo;
    AwtGraphicsConfigData *configData;
    ColorData		*cData;
    jboolean		dgaAvailable;
    void		*dgaDev;
    Pixmap		bitmask;
    jint		bgPixel;       /* bg pixel for the pixmap */
    jint                pmWidth;       /* width, height of the */
    jint                pmHeight;      /* pixmap */
#ifdef MITSHM
    ShmPixmapData       shmPMData;     /* data for switching between shm/nonshm pixmaps*/
#endif /* MITSHM */
};

#define X11SD_LOCK_UNLOCKED	0	/* surface is not locked */
#define X11SD_LOCK_BY_NULL	1	/* surface locked for NOP */
#define X11SD_LOCK_BY_XIMAGE	2	/* surface locked by Get/PutImage */
#define X11SD_LOCK_BY_DGA	3	/* surface locked by DGA */
#define X11SD_LOCK_BY_SHMEM	4	/* surface locked by ShMemExt */

#ifdef MITSHM
XImage * X11SD_GetSharedImage	    (X11SDOps *xsdo, jint width, jint height, jboolean readBits);
XImage * X11SD_CreateSharedImage    (X11SDOps *xsdo, jint width, jint height);
Drawable X11SD_CreateSharedPixmap   (X11SDOps *xsdo);
void	 X11SD_DropSharedSegment    (XShmSegmentInfo *shminfo);
void	 X11SD_PuntPixmap	    (X11SDOps *xsdo, jint width, jint height);
void	 X11SD_UnPuntPixmap	    (X11SDOps *xsdo);
jboolean X11SD_CachedXImageFits     (jint width, jint height, jint depth, jboolean readBits);
XImage * X11SD_GetCachedXImage      (jint width, jint height, jboolean readBits);
#endif /* MITSHM */
void     X11SD_DisposeOrCacheXImage (XImage * image);
void     X11SD_DisposeXImage(XImage * image);
void     X11SD_DirectRenderNotify(JNIEnv *env, X11SDOps *xsdo);
#endif /* !HEADLESS */

/*
 * This function returns a pointer to a native X11SDOps structure
 * for accessing the indicated X11 SurfaceData Java object.  It
 * verifies that the indicated SurfaceData object is an instance
 * of X11SurfaceData before returning and will return NULL if the
 * wrong SurfaceData object is being accessed.  This function will
 * throw the appropriate Java exception if it returns NULL so that
 * the caller can simply return.
 *
 * Note to callers:
 *	This function uses JNI methods so it is important that the
 *	caller not have any outstanding GetPrimitiveArrayCritical or
 *	GetStringCritical locks which have not been released.
 *
 *	The caller may continue to use JNI methods after this method
 *	is called since this function will not leave any outstanding
 *	JNI Critical locks unreleased.
 */
JNIEXPORT X11SDOps * JNICALL
X11SurfaceData_GetOps(JNIEnv *env, jobject sData);
