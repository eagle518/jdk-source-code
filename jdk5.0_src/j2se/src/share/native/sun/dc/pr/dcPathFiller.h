/*
 * @(#)dcPathFiller.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathFiller.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 *	
 *	Methods used in this code are protected by U.S. Patent 5,438,656.
 *	Foreign patents are pending.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_FILLER_H
#define _DC_PATH_FILLER_H

#include "dcPathStore.h"
#include "dcLLFiller.h"

#ifdef	__cplusplus
extern "C" {
#endif

/*	fill mode values (setFillMode)	*/
#define	dcPathFiller_EOFILL	dcLLFiller_EOFILL
#define dcPathFiller_NZFILL	dcLLFiller_NZFILL

/*	getTileState returns		*/
#define dcPathFiller_TILE_IS_ALL_0	0
#define dcPathFiller_TILE_IS_ALL_1	1
#define dcPathFiller_TILE_IS_GENERAL	2

/* LIMITS AND SIZES */
/* Tile size */
extern ixx dcPathFiller_tileSizeL2S;
extern ixx dcPathFiller_tileSize;
extern f32 dcPathFiller_tileSizeF;

/* Coordinate limits */
#define dcPathFiller_maxPathF		1000000.0F
#define dcPathFiller_validLoCoord(c)	((c) >= -dcPathFiller_maxPathF)
#define dcPathFiller_validHiCoord(c)	((c) <=  dcPathFiller_maxPathF)

typedef struct dcPathFillerFace_** dcPathFiller;
typedef struct dcPathFillerFace_ {
    dcPathStoreFace		mu;
    void	(*setFillMode)	(doeE, dcPathFiller,	ixx fillmode);
    void	(*getAlphaBox)	(doeE, dcPathFiller,	i32* box);
    void	(*setOutputArea)(doeE, dcPathFiller,	f32 x0, f32 y0,
							i32 w, i32 y);
    ixx		(*getTileState)	(doeE, dcPathFiller);
    void	(*writeAlpha8)	(doeE, dcPathFiller,	u8*  alpha,
							i32 xstride, i32 ystride,
							i32 pix0offset);
    void	(*writeAlpha16)	(doeE, dcPathFiller,	u16* alpha,
				 			i32 xstride, i32 ystride,
				 			i32 pix0offset);
    void	(*nextTile)	(doeE, dcPathFiller);
    void	(*reset)	(doeE, dcPathFiller);
} dcPathFillerFace;

extern dcPathFiller	dcPathFiller_create(doeE);

extern void		dcPathFiller_staticInitialize(doeE);
extern void		dcPathFiller_staticFinalize  (doeE);

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_FILLER_H */


