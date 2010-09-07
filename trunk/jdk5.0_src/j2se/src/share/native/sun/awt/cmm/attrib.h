/*
 * @(#)attrib.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)attrib.h	1.26 98/09/09

	Contains:
	    This module is the include file for the KCMS attribute interface.
	    It must be included in any source file using KCMS attributes.  There
	    are definitions for all enums needed for use with attributes.

	Written by:	Drivin' Team

****************************************************************
  COPYRIGHT (c) Eastman Kodak Compnay, 1991-1998 
  As  an  unpublished  work pursuant to Title 17 of the United
  States Code.  All rights reserved.
*****************************************************************

 */

#ifndef _ATTRIB_H_
#define _ATTRIB_H_

/*
 * Get all public include files
 */
#include "kcmsos.h"

/* define attribute ranges */
#define KCMS_ATTRIBUTE_START		1L
#define KCMS_ATTRIBUTE_END			16383L
#define CIPG_ATTRIBUTE_START		16384L
#define CIPG_ATTRIBUTE_END			32767L
#define KODAK_ATTRIBUTE_START		32768L
#define KODAK_ATTRIBUTE_END			65535L
#define KCM_ATTRIBUTE_USER_START	65536L
#define KCM_ATTRIBUTE_USER_END		2147483646L

#define KCM_MAX_ATTRIB_VALUE_LENGTH 255

typedef KpInt32_t KcmAttribute;


/* The definition of all currently supported Kcm Toolkit ATTRIBUTES.	*/

#define	KCM_UNIQUE_ID					1	/* (int) A unique ID, assigned by the KCM toolkit */
#define	KCM_TECH_VERSION				2	/* (int) technology version of this PT */
#define	KCM_TECH_TYPE					3	/* (regVal Technology) */
#define	KCM_SPACE_IN					4	/* (regVal ColorSpace) */
#define	KCM_SPACE_OUT					5	/* (regVal ColorSpace) */
#define	KCM_NUM_VAR_OUT					6	/* (int) # of output functions in this PT */
#define	KCM_NUM_VAR_1_IN				7	/* (int) # of input variables for output function 1 */
#define	KCM_NUM_VAR_2_IN				8	/* (int) # of input variables for output function 2 */
#define	KCM_NUM_VAR_3_IN				9	/* (int) # of input variables for output function 3 */
#define	KCM_NUM_VAR_4_IN				10	/* (int) # of input variables for output function 4 */
#define	KCM_NUM_VAR_5_IN				11	/* (int) # of input variables for output function 5 */
#define	KCM_NUM_VAR_6_IN				12	/* (int) # of input variables for output function 6 */
#define	KCM_NUM_VAR_7_IN				13	/* (int) # of input variables for output function 7 */
#define	KCM_NUM_VAR_8_IN				14	/* (int) # of input variables for output function 8 */
#define	KCM_COPYRIGHT					15	/* (string) Copyright information */
#define	KCM_PT_VERSION					16	/* (structStr) PT version number */
#define	KCM_CREATE_TIME					17	/* (datetime string) Time stamp when created */
#define	KCM_MEDIUM_IN					18	/* (#enum KcmMedium) medium for which the PT was designed, input */
#define	KCM_DEVICE_MFG_IN				19	/* (string) Brand name of input device */
#define	KCM_DEVICE_MODEL_IN				20	/* (string) Model of input device */
#define	KCM_DEVICE_UNIT_IN				21	/* (string) Unit or serial number of input device */
#define	KCM_DEVICE_SETTINGS_IN			22	/* (string) settings on input device when calibrated */
#define	KCM_MONITOR_WT_PT				23	/* (3 floats) xyY values of monitor @R=G=B=255 */
#define	KCM_DESCRIPTION					24	/* (string) description of the transform */
#define	KCM_CLASS						25	/* (#enum KcmClass) type of PT */
#define	KCM_EFFECT_TYPE					26	/* (regVal Effect) Effect Type */
#define	KCM_MEDIUM_OUT					27	/* (#enum KcmMedium) Medium of output */
#define	KCM_MEDIUM_DESC_IN				28	/* (string) description of input medium */
#define	KCM_MEDIUM_DESC_OUT				29	/* (string) description of output medium */
#define	KCM_MEDIUM_SENSE_IN				30	/* (#enum KcmMediumSense) input medium +/- */
#define	KCM_MEDIUM_SENSE_OUT			31	/* (#enum KcmMediumSense) output medium +/- */
#define	KCM_MEDIUM_PRODUCT_IN			32	/* (regVal MediumProduct) Input medium product */
#define	KCM_MEDIUM_PRODUCT_OUT			33	/* (regVal MediumProduct) Output medium product */
#define	KCM_PRT_UCR						34	/* (1 float) percent UCR */
#define	KCM_PRT_GCR						35	/* (1 float) amount GCR */
#define	KCM_ILLUM_TYPE_IN				39	/* (regVal Illumination) Illum. type and temp */
#define	KCM_ILLUM_TYPE_OUT				40	/* (regVal Illumination) Illum. type and temp */
#define	KCM_WHITE_POINT_IN				41	/* (2 floats) xy of white point */
#define	KCM_WHITE_POINT_OUT				42	/* (2 floats) xy of white point */
#define	KCM_DEVICE_PHOSPHOR_IN			43	/* (regVal Phosphor) */
#define	KCM_DEVICE_PHOSPHOR_OUT			44	/* (regVal Phosphor) */
#define	KCM_PRT_BLACK_SHAPE				45	/* (#enum KcmBlackShape) */
#define	KCM_PRT_BLACKSTART_DELAY		46	/* (#enum KcmBlackDelay) */
#define	KCM_CREATOR						47	/* (string) software used with device */
#define	KCM_LINEARIZATION_TYPE			49	/* (string) method used for linearization */
#define	KCM_PRT_LINE_RULINGS			50	/* (4 floats) screen rulings(C,M,Y,K) */
#define	KCM_PRT_SCREEN_ANGLES			51	/* (4 floats) screen angles(C,M,Y,K) */
#define	KCM_DMAX_OUT					52	/* (4 floats) max. density(C,M,Y,K) */
#define	KCM_DEVICE_LINEARIZED_IN		53	/* (#enum KcmLinearized) Input linearized? */
#define	KCM_DEVICE_LINEARIZED_OUT		54	/* (#enum KcmLinearized) Output linearized? */
#define	KCM_DEVICE_MFG_OUT				55	/* (string) Brand name of output device */
#define	KCM_DEVICE_MODEL_OUT			56	/* (string) Model of output device */
#define	KCM_DEVICE_UNIT_OUT				57	/* (string) Unit or serial number of output device */
#define	KCM_DEVICE_SETTINGS_OUT			58	/* (string) settings on output device when calibrated */
#define	KCM_COMPOSITION_STATE			59	/* (#enum KcmXFormState) raw or connected PT */

#define	KCM_BITS_IN						60	/* (int) # bits/component, input data */
#define	KCM_BITS_OUT					61	/* (int) # bits/component, output data */
#define	KCM_DATATYPE_IN					62	/* (#enum KcmSampleType) input data type */
#define	KCM_DATATYPE_OUT				63	/* (#enum KcmSampleType) output data type */
#define	KCM_INTERNATIONALIZATION		64	/* (regVal CharacterSet) character set */
#define	KCM_LASTSAVE_TIME				65	/* (datetime string) time of last save to file system */
#define	KCM_PRIMARIES_1_IN				66	/* (2 floats) input primary #1 */
#define	KCM_PRIMARIES_2_IN				67	/* (2 floats) input primary #2 */
#define	KCM_PRIMARIES_3_IN				68	/* (2 floats) input primary #3 */
#define	KCM_PRIMARIES_4_IN				69	/* (2 floats) input primary #4 */
#define	KCM_PRIMARIES_5_IN				70	/* (2 floats) input primary #5 */
#define	KCM_PRIMARIES_6_IN				71	/* (2 floats) input primary #6 */
#define	KCM_PRIMARIES_7_IN				72	/* (2 floats) input primary #7 */
#define	KCM_PRIMARIES_8_IN				73	/* (2 floats) input primary #8 */
#define	KCM_PRIMARIES_1_OUT				74	/* (2 floats) output primary #1 */
#define	KCM_PRIMARIES_2_OUT				75	/* (2 floats) output primary #2 */
#define	KCM_PRIMARIES_3_OUT				76	/* (2 floats) output primary #3 */
#define	KCM_PRIMARIES_4_OUT				77	/* (2 floats) output primary #4 */
#define	KCM_PRIMARIES_5_OUT				78	/* (2 floats) output primary #5 */
#define	KCM_PRIMARIES_6_OUT				79	/* (2 floats) output primary #6 */
#define	KCM_PRIMARIES_7_OUT				80	/* (2 floats) output primary #7 */
#define	KCM_PRIMARIES_8_OUT				81	/* (2 floats) output primary #8 */
#define	KCM_SEQUENCE_LIST				82	/* (structStr) PTs used to create this PT; UIDs in Hex */
#define	KCM_CHARACTERIZATION_ID			83	/* (structStr) characterization type used */
#define	KCM_DEVICE_SERIAL_NUMBER_IN		84	/* (string) device serial number, input */
#define	KCM_DEVICE_SERIAL_NUMBER_OUT	85	/* (string) device serial number, output */
#define	KCM_DEVICE_TYPE_IN				86	/* (regVal DeviceType) device type, input */
#define	KCM_DEVICE_TYPE_OUT				87	/* (regVal DeviceType) device type, output */
#define	KCM_DMAX_IN						88	/* (4 floats) max. density, input */
#define	KCM_DEVICE_CAL_DATA				89	/* (string) calibration data */
#define	KCM_INTERPRETATION_IN			90	/* (regVal Interpretation) data interpretation, input */
#define	KCM_INTERPRETATION_OUT			91	/* (regVal Interpretation) data interpretation, input */
#define	KCM_DEVICE_LT_AMBIENT_LVL_IN	92	/* (float) ambient light level, input in foot-lamberts */
#define	KCM_DEVICE_LT_AMBIENT_LVL_OUT	93	/* (float) ambient light level, output in foot-lamberts */
#define	KCM_NUM_ATTRIBUTES				94	/* (int) total number of attributes */

#define KCM_COMPRESSION_OUT				95	/* (#enum KcmCompression) type of gamut compression */

#define	KCM_SIM_MEDIUM_OUT				96	/* (#enum KcmMedium) Medium of simulation */
#define	KCM_SIM_MEDIUM_DESC_OUT			97	/* (string) description of simulated medium */
#define	KCM_SIM_MEDIUM_SENSE_OUT		98	/* (#enum KcmMediumSense) siumlated medium +/- */
#define	KCM_SIM_MEDIUM_PRODUCT_OUT		99	/* (regVal MediumProduct) simulated medium product */
#define	KCM_SIM_UCR						100	/* (1 float) percent UCR */
#define	KCM_SIM_GCR						101	/* (1 float) percent GCR */
#define	KCM_SIM_ILLUM_TYPE_IN			102	/* (regVal Illumination) Illum. type and temp */
#define	KCM_SIM_ILLUM_TYPE_OUT			103	/* (regVal Illumination) Illum. type and temp */
#define	KCM_SIM_WHITE_POINT_OUT			104	/* (2 floats) xy of white point */
#define	KCM_SIM_BLACK_SHAPE				105	/* (#enum KcmBlackShape) */
#define	KCM_SIM_BLACKSTART_DELAY		106	/* (#enum KcmBlackDelay) */
#define	KCM_SIM_LINE_RULINGS			107	/* (4 floats) screen rulings(C,M,Y,K) */
#define	KCM_SIM_SCREEN_ANGLE			108	/* (4 floats) screen angles(C,M,Y,K) */
#define	KCM_SIM_DMAX_OUT				109	/* (4 floats) max. density(C,M,Y,K) */
#define	KCM_SIM_DEVICE_MFG_OUT			110	/* (string) Brand name of simulated output device */
#define	KCM_SIM_DEVICE_MODEL_OUT		111	/* (string) Model of simulated output device */
#define	KCM_SIM_DEVICE_UNIT_OUT			112 /* (string) Unit or serial number of simulated output device */
#define	KCM_SIM_DEVICE_SETTINGS_OUT		113	/* (string) settings on simulated output device when calibrated */
#define	KCM_SIM_PRIMARIES_1_OUT			114	/* (2 floats) simulated output primary #1 */
#define	KCM_SIM_PRIMARIES_2_OUT			115	/* (2 floats) simulated output primary #2 */
#define	KCM_SIM_PRIMARIES_3_OUT			116	/* (2 floats) simulated output primary #3 */
#define	KCM_SIM_PRIMARIES_4_OUT			117	/* (2 floats) simulated output primary #4 */
#define	KCM_SIM_PRIMARIES_5_OUT			118	/* (2 floats) simulated output primary #5 */
#define	KCM_SIM_PRIMARIES_6_OUT			119	/* (2 floats) simulated output primary #6 */
#define	KCM_SIM_PRIMARIES_7_OUT			120	/* (2 floats) simulated output primary #7 */
#define	KCM_SIM_PRIMARIES_8_OUT			121	/* (2 floats) simulated output primary #8 */
#define	KCM_SIM_COMPRESSION_OUT			122	/* (#enum KcmCompression) type of gamut compression */

#define	KCM_CRC							123	/* (KcmInt) CRC of transform data NOT attributes */
#define	KCM_CRC_SEQUENCE				124	/* (structStr) CRCs of PTs used to create this PT in Hex */

#define	KCM_MONITOR_RED		KCM_PRIMARIES_1_OUT	/* (3 floats) colors of red phosphor */
#define	KCM_MONITOR_GREEN	KCM_PRIMARIES_2_OUT	/* (3 floats) colors of green phosphor */
#define	KCM_MONITOR_BLUE	KCM_PRIMARIES_3_OUT	/* (3 floats) colors of blue phosphor */

/* Monitor Print Agreement attributes */

#define KCM_MPA_MONITOR_UID				125	/* (int) The unique ID, assigned by the KCM toolkit, of the
													monitor PT being tuned */
#define KCM_MPA_MONITOR_WT_PT			126	/* (3 floats) xyY values of MPA monitor @R=G=B=255 */
#define KCM_MPA_MONITOR_WHITE_POINT		127	/* (2 floats) xy of MPA monitor white point */
#define KCM_MPA_MONITOR_PHOSPHOR		128	/* (regVal Phosphor) */
#define KCM_MPA_MONITOR_MFG				129	/* (string) Brand name of MPA monitor */
#define	KCM_MPA_MONITOR_MODEL			130	/* (string) Model of MPA monitor */
#define	KCM_MPA_MONITOR_UNIT			131	/* (string) Unit or serial number of MPA monitor */
#define	KCM_MPA_MONITOR_SETTINGS		132	/* (string) settings on MPA monitor when calibrated */
#define	KCM_MPA_MONITOR_PRIMARIES_1		133	/* (2 floats) input primary #1 */
#define	KCM_MPA_MONITOR_PRIMARIES_2		134	/* (2 floats) input primary #2 */
#define	KCM_MPA_MONITOR_PRIMARIES_3		135	/* (2 floats) input primary #3 */
#define	KCM_MPA_SIM_CLASS				136	/* (#enum KcmClass) type of output PT being tuned */
#define	KCM_MPA_SIM_UID					137	/* (int) The unique ID, assigned by the KCM toolkit, of the
													output PT being tuned */
#define	KCM_MPA_SIM_WHITE_POINT			138	/* (2 floats) xy of MPA output white point */
#define KCM_MPA_SIM_MFG					139	/* (string) Brand name of MPA ouput device being simulated */
#define	KCM_MPA_SIM_MODEL				140	/* (string) Model of MPA ouput device being simulated */
#define	KCM_MPA_SIM_UNIT				141	/* (string) Unit or serial number of MPA ouput device being simulated */
#define	KCM_MPA_SIM_SETTINGS			142	/* (string) settings on MPA ouput device being simulated when calibrated */
#define	KCM_MPA_SIM_PRIMARIES_1			143	/* (2 floats) input primary #1 */
#define	KCM_MPA_SIM_PRIMARIES_2			144	/* (2 floats) input primary #2 */
#define	KCM_MPA_SIM_PRIMARIES_3			145	/* (2 floats) input primary #3 */
#define	KCM_MPA_SIM_PRIMARIES_4			146	/* (2 floats) input primary #4 */
#define	KCM_MPA_SIM_PRIMARIES_5			147	/* (2 floats) input primary #5 */
#define	KCM_MPA_SIM_PRIMARIES_6			148	/* (2 floats) input primary #6 */
#define	KCM_MPA_SIM_PRIMARIES_7			149	/* (2 floats) input primary #7 */
#define	KCM_MPA_SIM_PRIMARIES_8			150	/* (2 floats) input primary #8 */
#define	KCM_MPA_SIM_COMPRESSION			151	/* (regVal KcmCompression) type of gamut compression of
													MPA ouput device being simulated */
#define	KCM_MPA_SIM_MEDIUM				152	/* (#enum KcmMedium) Medium of MPA ouput device being simulated */
#define	KCM_MPA_SIM_MEDIUM_DESC			153	/* (string) description of simulated medium of MPA ouput 
													device being simulated */
#define	KCM_MPA_SIM_MEDIUM_SENSE		154	/* (#enum KcmMediumSense) medium +/- of MPA ouput
													device being simulated */
#define	KCM_MPA_SIM_MEDIUM_PRODUCT		155	/* (regVal MediumProduct) medium product of 
													MPA ouput device being simulated */
#define	KCM_MPA_SIM_UCR					156	/* (1 float) percent UCR of MPA ouput device being simulated */
#define	KCM_MPA_SIM_GCR					157	/* (1 float) percent GCR of MPA ouput device being simulated */
#define	KCM_MPA_SIM_ILLUM_TYPE_IN		158	/* (regVal Illumination) Illum. type and temp of
													MPA ouput device being simulated */
#define	KCM_MPA_SIM_ILLUM_TYPE_OUT		159	/* (regVal Illumination) Illum. type and temp of 
													MPA ouput device being simulated */
#define	KCM_MPA_SIM_WHITE_PT			160	/* (2 floats) xy of white point of MPA ouput device being simulated */
#define	KCM_MPA_SIM_BLACK_SHAPE			161	/* (#enum KcmBlackShape) of MPA ouput device being simulated */
#define	KCM_MPA_SIM_BLACKSTART_DELAY	162	/* (#enum KcmBlackDelay) of MPA ouput device being simulated */
#define KCM_MPA_SIM_WT_PT				163	/* (3 floats) xyY values of MPA ouput device being simulated @R=G=B=255 */
#define KCM_MPA_SIM_PHOSPHOR			164	/* (regVal Phosphor) of MPA ouput device being simulated */

/* Input Effect attributes */

#define KCM_INPUT_EFF_UID				165	/* (int) The unique ID, assigned by the KCM toolkit, of the
													input PT to be used with this effect */
#define	KCM_INPUT_EFF_CLASS				166	/* (#enum KcmClass) type of input PT to be used with this effect */
#define KCM_INPUT_EFF_DEVICE_TYPE		167	/* (regVal DeviceType) device type of input PT to 
													be used with this effect */
#define KCM_INPUT_EFF_MFG				168	/* (string) Brand name from input PT to be used with this effect */
#define	KCM_INPUT_EFF_MODEL				169	/* (string) Model from input PT to be used with this effect */
#define	KCM_INPUT_EFF_UNIT				170	/* (string) Unit or serial number from input PT to be used with this effect */
#define	KCM_INPUT_EFF_SETTINGS			171	/* (string) settings from input PT to be used 
													with this effect when calibrated */
#define KCM_INPUT_EFF_WHITE_POINT		172	/* (2 floats) xy of white point from input PT to be used with this effect */
#define KCM_INPUT_EFF_MONITOR_WT_PT		173	/* (3 floats) xyY values of input if monitor @R=G=B=255 */
#define KCM_INPUT_EFF_PHOSPHOR			174	/* (regVal Phosphor) of input if monitor */
#define	KCM_INPUT_EFF_PRIMARIES_1		175	/* (2 floats) input primary #1 */
#define	KCM_INPUT_EFF_PRIMARIES_2		176	/* (2 floats) input primary #2 */
#define	KCM_INPUT_EFF_PRIMARIES_3		177	/* (2 floats) input primary #3 */
#define	KCM_INPUT_EFF_PRIMARIES_4		178	/* (2 floats) input primary #4 */
#define	KCM_INPUT_EFF_MEDIUM			179	/* (#enum KcmMedium) Medium from input PT to 
													be used with this effect */
#define	KCM_INPUT_EFF_MEDIUM_SENSE		180	/* (#enum KcmMediumSense) medium +/- from input PT 
													to be used with this effect */
#define	KCM_INPUT_EFF_MEDIUM_PRODUCT	181	/* (regVal MediumProduct) medium product from 
													input PT to be used with this effect */
#define	KCM_INPUT_EFF_ILLUM_TYPE		182	/* (regVal Illumination) Illum. type and temp 
													from input PT to be used with this effect */
#define KCM_CONFIGURATION_NAME			183	/* (string) Name of configuration this PT belongs to */
#define KCM_PHOTO_CD_FILM_TERMS			184	/* (string) Film terms from the Photo CD workstation */
#define KCM_SENSE_INVERTIBLE_IN			185	/* (enum/int) possible to invert input data? */
#define KCM_SENSE_INVERTIBLE_OUT		186	/* (enum/int) possible to invert output data? */

#define KCM_CHAN_NAME_1_OUT				187	/* (string) name of 1st output channel */
#define KCM_CHAN_NAME_2_OUT				188	/* (string) name of 2nd output channel */
#define KCM_CHAN_NAME_3_OUT				189	/* (string) name of 3rd output channel */
#define KCM_CHAN_NAME_4_OUT				190	/* (string) name of 4th output channel */
#define KCM_CHAN_NAME_5_OUT				191	/* (string) name of 5th output channel */
#define KCM_CHAN_NAME_6_OUT				192	/* (string) name of 6th output channel */
#define KCM_CHAN_NAME_7_OUT				193	/* (string) name of 7th output channel */
#define KCM_CHAN_NAME_8_OUT				194	/* (string) name of 8th output channel */
#define KCM_CHAN_NAME_1_IN				195	/* (string) name of 1st input channel */
#define KCM_CHAN_NAME_2_IN				196	/* (string) name of 2nd input channel */
#define KCM_CHAN_NAME_3_IN				197	/* (string) name of 3rd input channel */
#define KCM_CHAN_NAME_4_IN				198	/* (string) name of 4th input channel */
#define KCM_CHAN_NAME_5_IN				199	/* (string) name of 5th input channel */
#define KCM_CHAN_NAME_6_IN				200	/* (string) name of 6th input channel */
#define KCM_CHAN_NAME_7_IN				201	/* (string) name of 7th input channel */
#define KCM_CHAN_NAME_8_IN				202	/* (string) name of 8th input channel */
#define KCM_SIM_CHAN_NAME_1_OUT			203	/* (string) name of 1st simulated output channel */
#define KCM_SIM_CHAN_NAME_2_OUT			204	/* (string) name of 2nd simulated output channel */
#define KCM_SIM_CHAN_NAME_3_OUT			205	/* (string) name of 3rd simulated output channel */
#define KCM_SIM_CHAN_NAME_4_OUT			206	/* (string) name of 4th simulated output channel */
#define KCM_SIM_CHAN_NAME_5_OUT			207	/* (string) name of 5th simulated output channel */
#define KCM_SIM_CHAN_NAME_6_OUT			208	/* (string) name of 6th simulated output channel */
#define KCM_SIM_CHAN_NAME_7_OUT			209	/* (string) name of 7th simulated output channel */
#define KCM_SIM_CHAN_NAME_8_OUT			210	/* (string) name of 8th simulated output channel */
#define KCM_VISUAL_DENS_1_OUT			211	/* (float) visual density for 1st output channel */
#define KCM_VISUAL_DENS_2_OUT			212	/* (float) visual density for 2nd output channel */
#define KCM_VISUAL_DENS_3_OUT			213	/* (float) visual density for 3rd output channel */
#define KCM_VISUAL_DENS_4_OUT			214	/* (float) visual density for 4th output channel */
#define KCM_VISUAL_DENS_5_OUT			215	/* (float) visual density for 5th output channel */
#define KCM_VISUAL_DENS_6_OUT			216	/* (float) visual density for 6th output channel */
#define KCM_VISUAL_DENS_7_OUT			217	/* (float) visual density for 7th output channel */
#define KCM_VISUAL_DENS_8_OUT			218	/* (float) visual density for 8th output channel */
#define KCM_SIM_VISUAL_DENS_1_OUT		219	/* (float) visual density for 1st output channel */
#define KCM_SIM_VISUAL_DENS_2_OUT		220	/* (float) visual density for 2nd output channel */
#define KCM_SIM_VISUAL_DENS_3_OUT		221	/* (float) visual density for 3rd output channel */
#define KCM_SIM_VISUAL_DENS_4_OUT		222	/* (float) visual density for 4th output channel */
#define KCM_SIM_VISUAL_DENS_5_OUT		223	/* (float) visual density for 5th output channel */
#define KCM_SIM_VISUAL_DENS_6_OUT		224	/* (float) visual density for 6th output channel */
#define KCM_SIM_VISUAL_DENS_7_OUT		225	/* (float) visual density for 7th output channel */
#define KCM_SIM_VISUAL_DENS_8_OUT		226	/* (float) visual density for 8th output channel */
#define KCM_ICC_COLORSPACE_IN			227	/* (string) ICC Signature for Input Color SPace*/
#define KCM_ICC_COLORSPACE_OUT			228	/* (string) ICC Signature for Output Color SPace*/

#define KCM_ICC_PROFILE_TYPE			229	/* (enum/int) type of data in ICC mft's */


/* End of attribute list (increment KCM_LAST_VALID_ATTRIBUTE when adding a new entry) */

#define KCM_LAST_VALID_ATTRIBUTE		230	/* Placeholder for last valid KCMS attribute */

/* Values for Attributes */
#define	KCM_UNKNOWN 0

#define	KCM_UNKNOWN_S "0"

/* regVal Technology */
#define	KCM_FUT 1
#define	KCM_SERIAL_FUT 2

#define KCM_FUT_S "1"
#define KCM_SERIAL_FUT_S "2"

/* regVal ColorSpace */
#define KCM_RGB 2
#define KCM_PHOTO_CD_YCC 3
#define KCM_UVLSTAR 4
#define KCM_CMY 5
#define KCM_CMYK 6
#define KCM_RCS 7
#define KCM_CIE_XYZ 8
#define KCM_CIE_LAB 9
#define KCM_CIE_LUV 10
#define KCM_HI_FI_5_COLOR 11
#define KCM_HI_FI_6_COLOR 12
#define KCM_HI_FI_7_COLOR 13
#define KCM_HI_FI_8_COLOR 14
#define KCM_CIE_YXY 15
#define KCM_CIE_GRAY 16
#define KCM_CIE_HSV 17
#define KCM_CIE_HLS 18
#define KCM_MONO 19
#define KCM_IMAGE_LAB 35
#define KCM_IMAGE_XYZ 36
#define KCM_ICC_LAB8 37
#define KCM_ICC_LAB16 38
#define KCM_ICC_XYZ 39
#define KCM_ADOBE_LAB 40

  /* generic n channel color spaces */
#define KCM_2_COLOR 33
#define KCM_3_COLOR 20
#define KCM_4_COLOR 21
#define KCM_5_COLOR 22
#define KCM_6_COLOR 23
#define KCM_7_COLOR 24
#define KCM_8_COLOR 25
#define KCM_9_COLOR 26
#define KCM_A_COLOR 27
#define KCM_B_COLOR 28
#define KCM_C_COLOR 29
#define KCM_D_COLOR 30
#define KCM_E_COLOR 31
#define KCM_F_COLOR 32
/* Unknown color space from ICC profile */
#define KCM_ICC_UNKNOWN 34

#define KCM_RCS_S "7"

/* regVal DeviceType */
#define KCM_CAPTURE 1
#define KCM_DISPLAY 2
#define KCM_SCANNER 3
#define KCM_FILM_SCANNER 4
#define KCM_REFLECTIVE_SCANNER 5
#define KCM_PRINTER 6
#define KCM_INK_JET_PRINTER 7
#define KCM_WAX_TRANSFER_PRINTER 8
#define KCM_ELECTROPHOTOGRAPHIC_PRINTER 9
#define KCM_ELECTROSTATIC_PRINTER 10
#define KCM_DYE_SUBLIMATION_PRINTER 11
#define KCM_PHOTOGRAPHIC_PAPER_PRINTER 12
#define KCM_FILM_WRITER 13
#define KCM_VIDEO_MONITOR 14
#define KCM_VIDEO_CAMERA 15
#define KCM_PHOTO_CD 16
#define KCM_PROOFING_SYSTEM 17
#define KCM_PHOTOTYPESETTER 18
#define KCM_PROJECTION_TV 19
#define KCM_LCD_DISPLAY 20

/* regVal MediumProduct */
#define KCM_KODACHROME 1
#define KCM_EKTACOLOR 2
#define KCM_EKTACHROME 3
#define KCM_FUJICHROME 4
#define KCM_EKTATHERM 5
#define KCM_PAPER 6
#define KCM_PHOTO_PRINT 7
#define KCM_TRANSPARENCY 8
#define KCM_COLOR_NEGATIVE_FILM	9

/* regVal Illumination */
#define KCM_D50_FLOURESCENT 1
#define KCM_D65_FLOURESCENT 2
#define KCM_TUNGSTEN 3
#define KCM_D50 4
#define KCM_D65 5
#define KCM_D93 6
#define KCM_OFFICE_FLOURESCENT 7
#define KCM_INCANDESCENT 8

/* regVal Phosphor */
#define KCM_P22 1
#define KCM_EBU 2
#define KCM_SMPTE_C 3

/* regVal Effect */
#define KCM_EFFECT_INPUT 1
#define KCM_EFFECT_OUTPUT 2
#define KCM_EFFECT_ADAPTATION 3
#define KCM_EFFECT_EDITING_TONE 4
#define KCM_EFFECT_EDITING_GRAY_BALANCE 5
#define KCM_PICTURE_EFFECT 9
#define KCM_MPA_EFFECT 10

/* shorter names were for Microsoft compiler, it used to complain */
#define KCM_EFFECT_EDITING_SELECTIVE_CO 6
#define KCM_EFFECT_EDITING_NEGATIVE_TO_ 7

#define KCM_EFFECT_EDITING_SELECTIVE_COLOR 6
#define KCM_EFFECT_EDITING_NEGATIVE_TO_POSITIVE 7


#define KCM_EFFECT_MULTIPLE 8

/* regVal Interpretation */
#define KCM_COLORIMETRIC_INTERPRETATION 1
#define KCM_PHOTO_CD_INTERPRETATION 2

/* regVal CharacterSet */
#define KCM_7_BIT_ASCII 1
#define KCM_8_BIT_ASCII 2
#define KCM_EUC 3
#define KCM_EBCDIC 4
#define KCM_ISO_646 5
#define KCM_ISO_6937 6
#define KCM_ISO_8859_1 7
#define KCM_ISO_8859_2 8
#define KCM_ISO_8859_3 9
#define KCM_ISO_8859_4 10
#define KCM_ISO_8859_5 11
#define KCM_ISO_8859_6 12
#define KCM_ISO_8859_7 13
#define KCM_ISO_8859_8 14
#define KCM_ISO_8859_9 15
#define KCM_ISO_10646 16
#define KCM_UNICODE 17
#define KCM_GB_2312 18
#define KCM_JIS 19
#define KCM_SHIFT_JIS 20
#define KCM_CCCII 21
#define KCM_MCS 22

/* #enum KcmClass */
#define KCM_INPUT_CLASS 1
#define KCM_OUTPUT_CLASS 2
#define KCM_EFFECT_CLASS 3
#define KCM_DISPLAY_CLASS 5
#define KCM_OUTPUT_SIMULATE_CLASS 6
#define KCM_INPUT_LINEARIZATION_CLASS 7
#define KCM_OUTPUT_LINEARIZATION_CLASS 8
#define KCM_INPUT_TO_SYSTEM_CLASS 9
#define KCM_SYSTEM_TO_OUTPUT_CLASS 10
#define KCM_SYSTEM_TO_MONITOR_CLASS 11
#define KCM_MONITOR_TO_SYSTEM_CLASS 12
#define KCM_INPUT_TO_OUTPUT_CLASS 13
#define KCM_COMPOSED_CLASS 14
#define KCM_EXPORT_CLASS 15
#define KCM_IMPORT_CLASS 16
#define KCM_INTERCHANGE_TO_SYSTEM_CLASS 17
#define KCM_SYSTEM_TO_INTERCHANGE_CLASS 18
#define	KCM_ALARM_CLASS 25
#define	KCM_AUX_RULE_CLASS 26

#define KCM_AUX_RULE_CLASS_S "26"

/* #enum KcmXFormState */
#define KCM_SINGLE 1
#define KCM_COMPOSED 2
#define KCM_CONNECTED 3

/* #enum KcmMedium */
#define KCM_REFLECTIVE 1
#define KCM_TRANSMISSIVE 2

/* #enum KcmMediumSense */
#define KCM_POSITIVE 1
#define KCM_NEGATIVE 2

/* #enum KcmBlackShape */
#define KCM_AGRESSIVE_BLACK_SHAPE 1
#define KCM_NORMAL_BLACK_SHAPE 2

/* #enum KcmBlackDelay */
#define KCM_SHORT_BLACK_DELAY 1
#define KCM_MEDIUM_BLACK_DELAY 2
#define KCM_LONG_BLACK_DELAY 3

/* #enum KcmLinearized */
#define KCM_IS_LINEARIZED 1
#define KCM_IS_NOT_LINEARIZED 2

/* #enum KcmSampleType */
#define KCM_BIT 1
#define KCM_BYTE 2
#define KCM_UBYTE 3
#define KCM_SHORT 4
#define KCM_USHORT 5
#define KCM_LONG 6
#define KCM_ULONG 7
#define KCM_FLOAT 8
#define KCM_DOUBLE 9
#define KCM_USHORT_12 10	/* two bytes per component with 12 bits of actual data */
#define KCM_USHORT_555 11	/* 14-10 (RED) 9-5 (GREEN) 4-0 (BLUE) */
#define KCM_USHORT_565 12	/* 15-11 (RED) 10-5 (GREEN) 4-0 (BLUE) */
#define KCM_R10G10B10 13	/* 29-20 (RED) 19-10 (GREEN) 9-0 (BLUE) */

/* regVal KcmCompression */
#define KCM_PERCEPTUAL 1
#define KCM_COLORIMETRIC 2
#define KCM_SATURATION_MATCH 3
#define KCM_REFLECTION_COMPRESSION 4

/* #enum KcmLinearized */
#define KCM_IS_INVERTIBLE 1
#define KCM_IS_NOT_INVERTIBLE 2

/* (enum/int) type of data in ICC mft's */
#define KCM_ICC_TYPE_0			(1)	/* PCS/device definitions the same */
#define KCM_ICC_TYPE_1			(2)	/* PCS/device definitions differentiated */


/* compatibility definitions which will eventually be removed */

#define	KCM_IN_SPACE			KCM_SPACE_IN				/* (regVal ColorSpace) */
#define	KCM_OUT_SPACE			KCM_SPACE_OUT				/* (regVal ColorSpace) */
#define	KCM_NUM_OUT_VAR			KCM_NUM_VAR_OUT				/* (int) # of output functions in this PT */
#define	KCM_NUM_IN_VAR_1		KCM_NUM_VAR_1_IN			/* (int) # of input variables for output function 1 */
#define	KCM_NUM_IN_VAR_2		KCM_NUM_VAR_2_IN			/* (int) # of input variables for output function 2 */
#define	KCM_NUM_IN_VAR_3		KCM_NUM_VAR_3_IN			/* (int) # of input variables for output function 3 */
#define	KCM_NUM_IN_VAR_4		KCM_NUM_VAR_4_IN			/* (int) # of input variables for output function 4 */
#define	KCM_NUM_IN_VAR_5		KCM_NUM_VAR_5_IN			/* (int) # of input variables for output function 5 */
#define	KCM_NUM_IN_VAR_6		KCM_NUM_VAR_6_IN			/* (int) # of input variables for output function 6 */
#define	KCM_NUM_IN_VAR_7		KCM_NUM_VAR_7_IN			/* (int) # of input variables for output function 7 */
#define	KCM_NUM_IN_VAR_8		KCM_NUM_VAR_8_IN			/* (int) # of input variables for output function 8 */
#define	KCM_PRODUCT_VERSION		KCM_PT_VERSION				/* (structStr) PT version number */
#define	KCM_IN_MEDIUM			KCM_MEDIUM_IN				/* (#enum KcmMedium) medium for which the PT was designed, input */
#define	KCM_DEVICE_NAME_IN		KCM_DEVICE_MFG_IN			/* (string) Brand name of input device */
#define	KCM_OUT_MEDIUM			KCM_MEDIUM_OUT				/* (#enum KcmMedium) Medium of output */
#define	KCM_IN_MEDIUM_DESC		KCM_MEDIUM_DESC_IN			/* (string) description of input medium */
#define	KCM_OUT_MEDIUM_DESC		KCM_MEDIUM_DESC_OUT			/* (string) description of output medium */
#define	KCM_IN_MEDIUM_SENSE		KCM_MEDIUM_SENSE_IN			/* (#enum KcmMediumSense) input medium +/- */
#define	KCM_OUT_MEDIUM_SENSE	KCM_MEDIUM_SENSE_OUT		/* (#enum KcmMediumSense) output medium +/- */
#define	KCM_IN_FILM_PRODUCT		KCM_MEDIUM_PRODUCT_IN		/* (regVal MediumProduct) Input medium product */
#define	KCM_OUT_FILM_PRODUCT	KCM_MEDIUM_PRODUCT_OUT		/* (regVal MediumProduct) Output medium product */
#define	KCM_IN_ILLUM_TYPE		KCM_ILLUM_TYPE_IN			/* (enum KcmIllumType) Illum. type and temp */
#define	KCM_OUT_ILLUM_TYPE		KCM_ILLUM_TYPE_OUT			/* (regVal Illumination) Illum. type and temp */
#define	KCM_INPUT_WHITE_PT		KCM_WHITE_POINT_IN			/* (3 floats) xyY of white point */
#define	KCM_OUTPUT_WHITE_PT		KCM_WHITE_POINT_OUT			/* (3 floats) xyY of white point */
#define	KCM_INPUT_PHOSPHOR		KCM_DEVICE_PHOSPHOR_IN		/* (regVal Phosphor) */
#define	KCM_OUTPUT_PHOSPHOR		KCM_DEVICE_PHOSPHOR_OUT		/* (regVal Phosphor) */
#define	KCM_BLACK_SHAPE			KCM_PRT_BLACK_SHAPE			/* (#enum KcmBlackShape) */
#define	KCM_BLACKSTART_DELAY	KCM_PRT_BLACKSTART_DELAY	/* (#enum KcmBlackDelay) */
#define	KCM_DEVICE_SOFTWARE		KCM_CREATOR					/* (string) software used with device */
#define	KCM_LINE_RULINGS		KCM_PRT_LINE_RULINGS		/* (4 floats) screen rulings(C,M,Y,K) */
#define	KCM_SCREEN_ANGLES		KCM_PRT_SCREEN_ANGLES		/* (4 floats) screen angles(C,M,Y,K) */
#define	KCM_DMAX				KCM_DMAX_OUT				/* (4 floats) max. density(C,M,Y,K) */
#define	KCM_INPUT_LINEARIZED	KCM_DEVICE_LINEARIZED_IN	/* (#enum KcmLinearized) Input linearized? */
#define	KCM_OUTPUT_LINEARIZED	KCM_DEVICE_LINEARIZED_OUT	/* (#enum KcmLinearized) Output linearized? */
#define	KCM_DEVICE_NAME_OUT		KCM_DEVICE_MFG_OUT			/* (string) Brand name of output device */
#define	KCM_RAW					KCM_COMPOSITION_STATE		/* (#enum KcmXFormState) raw or connected PT */

/* compatibility attribute values */
#define KCM_RGB_MONITOR 2
#define KCM_RGB_MONITOR_S "2"
#define KCM_INPUT_EFFECT 1
#define KCM_OUTPUT_EFFECT 2
#define KCM_COLOR_SPACE_UNKNOWN_S "0"
#define KCM_MEDIUM_UNKNOWN 0
#define KCM_SENSE_UNKNOWN 0
#define KCM_FILM_UNKNOWN 0
#define KCM_GCR_UNKNOWN 0
#define KCM_UCR_UNKNOWN 0

#endif		/* _ATTRIB_H_ */
