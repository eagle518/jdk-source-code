/*
 * @(#)orion.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * ORION.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#ifndef __T2K_ORION__
#define __T2K_ORION__
#include "scoder.h"
#include "truetype.h"
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/* ----- ----- ----- ----- ----- */
#define ORION_MAJOR_VERSION 1
#define ORION_MINOR_VERSION 0

#define NUM_ECOPYX (8)
#define NUM_EX (NUM_ECOPYX+8+1)

#define ORION_STATE_0 0

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	/* public */
	int OrionState;
	int num_eb1, num_e;
	int num_eb2;	/* == num_e - num_eb1 - 1 */
	short *dx; 		/* [t->num_eb1 * 256] */
	short *dy; 		/* [t->num_eb1 * 256] */
	char *onCurve;	/* [t->num_eb1 * 256] */
	SCODER **copy; /* SCODER *copy[ t->num_eb1 ] */
	SCODER **literal; /* SCODER * literal[ t->num_e ] */
	SCODER *control;
	SCODER *ep;
} OrionModelClass;


#ifdef ENABLE_WRITE
OrionModelClass *New_OrionModelClass( sfntClass *font );
void Save_OrionModelClass( OrionModelClass *t, OutputStream *out );
int KnownVectorInOrionModel( OrionModelClass *t, short dx, short dy, char onCurve );
#endif
/*
 * The standard OrionModel constructor.
 */
OrionModelClass *New_OrionModelClassFromStream( tsiMemObject *mem, InputStream *in );

/*
 * Updates the orion state.
 */
void Set_OrionState( OrionModelClass *t, int dx, int dy, char onCurve );

/*
 * The Destructor.
 */
void Delete_OrionModelClass( OrionModelClass *t );


/* ----- ----- ----- ----- ----- */
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_ORION__ */
