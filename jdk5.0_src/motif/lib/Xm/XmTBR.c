/*
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident	"@(#)XmTBR.c	1.5 02/06/13 SMI"

/******************************************************************************
  This file is a implementation of the word break support in motif

  The implementation approach used below is to wrap a locale specific
  word break functions around a standard word break routines in motif.
  The XmTBR methods table contains pointers to methods in this file, which
  merely wrap calls to the m_strscanfor and m_wcsscanfor() functions around
  calls to the methods in motif (basically Text and TextF) .

  See PSARC/1999/593 for more detail.
*******************************************************************************/

#ifdef SUN_TBR

#include <sys/isa_defs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "XmRenderTI.h"
#include <X11/Intrinsic.h>
#include <dlfcn.h>
#include "XmTBR.h"


#ifdef  _LP64

#if defined(__sparcv9)
#define MACH_ARCH     "sparcv9"
#elif defined(__ia64)
#define MACH_ARCH     "ia64"
#else
#error "unknown architecture"
#endif /* __sparcv9 */

#else /* _LP64 */
#define MACH_ARCH ""
#endif /* end of _LP64 */


XmTBR
XmCreateXmTBR()
{
	XmTBR xm_tbr;
	char *locale_name;
	char so_path[PATH_MAX];
	struct stat st;
	void *so;

	locale_name = setlocale(LC_CTYPE, (const char *)NULL);
	if (!locale_name)
		return ((XmTBR)NULL);

#ifdef	_LP64 
	sprintf(so_path, "/usr/lib/locale/%s/LC_CTYPE/%s/textboundary.so.%d",
		locale_name, MACH_ARCH, TBR_MODULE_VERSION);
#else
	sprintf(so_path, "/usr/lib/locale/%s/LC_CTYPE/textboundary.so.%d",
		locale_name, TBR_MODULE_VERSION);
#endif

	/*
	 * The textboundary.so should exist and must be a regular file.
	 * Otherwise, we return (XmTBR)NULL to save time for non-TBR locales.
	 */
	if (stat(so_path, &st) != 0 || (!(st.st_mode & S_IFREG)))
		return ((XmTBR)NULL);
		
	/*
	 * Open the locale-specific TBR module and collect all four function
	 * pointers.
	 */
	so = dlopen(so_path, RTLD_LAZY);
	if (so == (void *)NULL)
		return ((XmTBR)NULL);

	xm_tbr = (XmTBR)calloc(1, sizeof(XmTBRRec));
	if (xm_tbr == NULL) {
		dlclose(so);
		return ((XmTBR)NULL);
	}

	xm_tbr->m_create_tbr = (TBRObject(*)())dlsym(so, "__m_scanfor_open");
	xm_tbr->m_destroy_tbr = (size_t(*)())dlsym(so, "__m_scanfor_close");
	xm_tbr->m_strscanfor = (size_t(*)())dlsym(so, "__m_strscanfor");
	xm_tbr->m_wcsscanfor = (size_t(*)())dlsym(so, "__m_wcsscanfor");
	if (xm_tbr->m_create_tbr == NULL || xm_tbr->m_destroy_tbr == NULL ||
	    xm_tbr->m_strscanfor == NULL || xm_tbr->m_wcsscanfor == NULL) {
		dlclose(so);
		free((void *)xm_tbr);
		return ((XmTBR)NULL);
	}

	/* Create the TBR object handle for this session. */
	xm_tbr->tbr_object = (TBRObject)(*xm_tbr->m_create_tbr)(locale_name);
	if (xm_tbr->tbr_object == (TBRObject)-1) {
		dlclose(so);
		free((void *)xm_tbr);
		return ((XmTBR)NULL);
	}

	xm_tbr->so = so;

	return ((XmTBR)xm_tbr);
}


void
XmDestroyXmTBR(XmTBR xm_tbr)
{ 
	/*
	 * We assume if xm_tbr is not NULL, we have all data fields at
	 * the xm_tbr are intact and valid. This means whoever is going to
	 * maintain this file should make sure that XmCreateXmTBR and
	 * XmDestroyXmTBR are the only functions in the Motif that will
	 * touch the inside of the XmTBRObject (from XmRendition).
	 */
	if (xm_tbr) {
		(*xm_tbr->m_destroy_tbr)(xm_tbr->tbr_object);
		dlclose(xm_tbr->so);
		free((void *)xm_tbr);
	}
}


size_t
XmStrScanForTB(XmTBR xm_tbr, char *inbuf_str, size_t in_num_chars,
		Boolean is_wchar, XmTextPosition position,
		XmTextScanDirection dir, TBRScanCondition condition,
		Boolean inverse)
{
	if (is_wchar) {
		return ((*xm_tbr->m_wcsscanfor)(xm_tbr->tbr_object, 
					(wchar_t *)inbuf_str, in_num_chars, 
					(size_t)position,
					(dir == XmsdLeft) ? TBR_Back : TBR_Forw,
					condition, inverse));
	}

	return ((*xm_tbr->m_strscanfor)(xm_tbr->tbr_object,
				inbuf_str, in_num_chars, (size_t)position,
				(dir == XmsdLeft) ? TBR_Back :TBR_Forw,
				condition, inverse));
}


#endif /*SUN_TBR*/
