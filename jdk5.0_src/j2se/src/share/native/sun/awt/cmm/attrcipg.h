/*
 * @(#)attrcipg.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*

	File:		attrcipg.h	"@(#)attrcipg.h	1.14 03/01/99"

	Contains:
			This module is the include file for the private CIPG
			KCMS attribute interface.  It must be included in any
			source file using KCMS attributes.  There are
			definitions for all enums needed for use with
			attributes.

	Written by:	Drivin' Team

	Copyright:	(c) 1994-1999 by Eastman Kodak Company, all rights
			reserved.

 */


#ifndef _ATTRCIPG_H_
#define _ATTRCIPG_H_

/* The enumeration of all currently supported private CIPG attributes.
   All attribute values are returned as strings. The meaning of these
   strings is indicated in the comment within the ()'s. Hence, if a
   value is, for example an enum, use the atoi() call to convert to an int. */

#define	KCM_CHAIN_GRID				16384	/* chaining grid type */
#define	KCM_IN_CHAIN_CLASS			16385	/* input composition ID (1-3 only valid IDs)*/
#define	KCM_OUT_CHAIN_CLASS			16386	/* output composition ID (1-3 only valid IDs)*/
#define	KCM_REF_DEVICE_UNIT			16387	/* unit or s/n of reference device */
#define	KCM_RAW_RGB					16388	/* File with raw device data(scanners) */
#define	KCM_TARGET_TYPE				16389	/* (string) calibration target type */
#define	KCM_TARGET_BASELINE			16390	/* (string) file with baseline meas */
#define	KCM_FIT_TYPE				16391	/* (string) type of modeling */
#define	KCM_GRID_SIZE				16392	/* (3-tuple) grid dimensions */
#define	KCM_GEN_SOFTWARE			16393	/* (string) generation software */
#define	KCM_GEN_HARDWARE			16394	/* (string) system used to generate */
#define	KCM_CAL_SUMMARY				16395	/* (string) file with cal. summary */
#define	KCM_VER_SUMMARY				16396	/* (string) file with ver. summary */
#define	KCM_PERCENT_DOT				16397	/* (string) file with cmded % dots */
#define	KCM_IN_SOURCE				16398	/* (string) file with injected src */
#define	KCM_OUT_SOURCE				16399	/* (string) file with injected src */
#define	KCM_LINEARIZATION_FILE		16400	/* (string) file with linearization data */
#define	KCM_KCP_VERSION				16401	/* (string) Color Processor version */
#define	KCM_INKFUT_PATH				16402	/* (string) Path to table directory */
#define	KCM_OUT_LIN_SMOOTH			16403	/* (4 integers) output linearization smoothing */
#define	KCM_GAMMA_RED_IN			16404	/* (4 floats max.) red input gamma data */
#define	KCM_GAMMA_GREEN_IN			16405	/* (4 floats max.) green input gamma data */
#define	KCM_GAMMA_BLUE_IN			16406	/* (4 floats max.) blue input gamma data */
#define	KCM_GAMMA_RED_OUT			16407	/* (4 floats max.) red output gamma data */
#define	KCM_GAMMA_GREEN_OUT			16408	/* (4 floats max.) green output gamma data */
#define	KCM_GAMMA_BLUE_OUT			16409	/* (4 floats max.) blue output gamma data */
#define	KCM_DENSITY_FILTER 16410	/* string to specify filter type in densitometer*/
#define KCM_25_DOTGAIN 16411 			/* 4 integers of CMYK dot gain at 25% nominal dot */
#define KCM_50_DOTGAIN 16412			/* 4 integers of CMYK dot gain at 50% nominal dot */
#define KCM_75_DOTGAIN 16413			/* 4 integers of CMYK dot gain at 75% nominal dot */
#define KCM_AIM_FILE_ID 16414			/* instrument aim file id */
#define KCM_CP_RULES_DIR			16415	/* path to the color processor rules directory */

#define	KCM_KCP_INPUT_WT_UPVP		16441	/* u'v' representation of att #41 */
#define	KCM_KCP_OUTPUT_WT_UPVP		16442	/* u'v' representation of att #42 */

#define	KCM_IN_CHAIN_CLASS_2		16485	/* input composition ID (1-6 only valid IDs)*/
#define	KCM_OUT_CHAIN_CLASS_2		16486	/* output composition ID (1-6 only valid IDs)*/

/* Chainning Classes */
#define KCM_CHAIN_MIN		1	/* Min Chain Value */
#define	KCM_CHAIN_CLASS_RCS			1		/* RCS, (simulate, effects), Photo CD in */
#define	KCM_CHAIN_CLASS_MON_RGB		2		/* Monitor RGB */
#define	KCM_CHAIN_CLASS_UVL			3		/* uvL */
#define	KCM_CHAIN_CLASS_CMYK		4		/* Printer/proofer CMYK */
#define	KCM_CHAIN_CLASS_CMY			5		/* Printer CMY (or RGB) */
#define	KCM_CHAIN_CLASS_CIELAB1		6		/* CIELAB (for PhotoShop) (ICC Definition) */
#define	KCM_CHAIN_CLASS_CIELAB2		7		/* CIELAB (IT8 definition) */
#define	KCM_CHAIN_CLASS_CIELAB3		8		/* CIELAB (ColorFax definition) */
#define	KCM_CHAIN_CLASS_YCC			9		/* ycc */
#define	KCM_CHAIN_CLASS_XYZ			10		/* xyz */
#define	KCM_CHAIN_CLASS_HIFI		11		/* HiFi */
#define	KCM_CHAIN_CLASS_SCN_RGB		12		/* Scanner RGB */

#define KCM_CHAIN_MAX				12		/* Change when new added */

#endif		/* _ATTRCIPG_H_ */
