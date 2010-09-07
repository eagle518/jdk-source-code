/*
 * @(#)FSError.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	File:		FSError.h

 
	Copyright:	© 1989-1990 by Apple Computer, Inc., all rights reserved.
 */
 
/************/
/** ERRORS **/
/************/
#ifdef	oldscaler

#define NO_ERR						0x0000
#define NULL_KEY					0x0000

/** EXTERNAL INTERFACE PACKAGE **/
#define BAD_INPUT_RECORD			0x1000
#define NULL_KEY_ERR				0x1001
#define NULL_INPUT_PTR_ERR			0x1002
#define NULL_MEMORY_BASES_ERR		0x1003
#define VOID_FUNC_PTR_BASE_ERR		0x1004
#define OUT_OFF_SEQUENCE_CALL_ERR	0x1005
#define BAD_CLIENT_ID_ERR			0x1006
#define NULL_SFNT_DIR_ERR			0x1007
#define	NULL_SFNT_FRAG_PTR_ERR		0x1008
#define NULL_OUTPUT_PTR_ERR			0x1009
#define INVALID_GLYPH_INDEX			0x100A

/* fnt_execute */
#define UNDEFINED_INSTRUCTION_ERR	0x1101
#define TRASHED_MEM_ERR				0x1102
#define INTERP_ERROR_ABORT			0x1103


/* fsg_CalculateBBox */
#define POINT_MIGRATION_ERR			0x1201

/* sc_ScanChar */
#define BAD_START_POINT_ERR			0x1301

/** SFNT DATA ERROR and errors in sfnt.c **/
#define SFNT_DATA_ERR				0x1400
#define POINTS_DATA_ERR				0x1401
#define INSTRUCTION_SIZE_ERR 		0x1402
#define CONTOUR_DATA_ERR			0x1403
#define GLYPH_INDEX_ERR				0x1404
#define BAD_MAGIC_ERR				0x1405
#define OUT_OF_RANGE_SUBTABLE		0x1406
#define UNKNOWN_COMPOSITE_VERSION	0x1407
#define CLIENT_RETURNED_NULL		0x1408
#define MISSING_SFNT_TABLE			0x1409
#define UNKNOWN_CMAP_FORMAT		0x140A
#define BITMAP_NOT_AVAILABLE		0x140B
#define NO_DATA_AVAILABLE_ERR		0x140C

/* spline call errors */
#define BAD_CALL_ERR				0x1500

#define TRASHED_OUTLINE_CACHE		0x1600

#else	
	/* New Scaler API-mapped Error Codes */

#define NO_ERR					scaler_no_problem
#define NULL_KEY					scaler_no_problem

/** EXTERNAL INTERFACE PACKAGE **/
#define BAD_INPUT_RECORD			scaler_bad_input
#define NULL_KEY_ERR				scaler_bad_input
#define NULL_INPUT_PTR_ERR			scaler_bad_input
#define NULL_MEMORY_BASES_ERR		scaler_bad_input
#define VOID_FUNC_PTR_BASE_ERR		scaler_bad_input
#define OUT_OFF_SEQUENCE_CALL_ERR	scaler_bad_input
#define BAD_CLIENT_ID_ERR			scaler_bad_input
#define NULL_SFNT_DIR_ERR			scaler_bad_input
#define NULL_SFNT_FRAG_PTR_ERR		scaler_bad_input
#define NULL_OUTPUT_PTR_ERR		scaler_bad_input
#define INVALID_GLYPH_INDEX			scaler_bad_glyph_index

/* fnt_execute */
#define UNDEFINED_INSTRUCTION_ERR	scaler_hinting_error
#define TRASHED_MEM_ERR			scaler_hinting_error
#define INTERP_ERROR_ABORT			scaler_hinting_error

/* fsg_CalculateBBox */
#define POINT_MIGRATION_ERR		scaler_scan_error

/* sc_ScanChar */
#define BAD_START_POINT_ERR		scaler_scan_error
#define SCAN_ERR					scaler_scan_error

/** SFNT DATA ERROR and errors in sfnt.c **/
#define SFNT_DATA_ERR				scaler_bad_font_data
#define POINTS_DATA_ERR			scaler_bad_font_data
#define INSTRUCTION_SIZE_ERR 		scaler_bad_font_data
#define CONTOUR_DATA_ERR			scaler_bad_font_data
#define GLYPH_INDEX_ERR			scaler_bad_font_data
#define BAD_MAGIC_ERR				scaler_bad_font_data
#define OUT_OF_RANGE_SUBTABLE		scaler_bad_font_data
#define UNKNOWN_COMPOSITE_VERSION	scaler_bad_font_data
#define CLIENT_RETURNED_NULL		scaler_get_fonttable_failed
#define MISSING_SFNT_TABLE			scaler_required_table_missing
#define UNKNOWN_CMAP_FORMAT		scaler_bad_font_data
#define BITMAP_NOT_AVAILABLE		scaler_no_output
#define NO_DATA_AVAILABLE_ERR		scaler_no_output

/* spline call errors */
#define BAD_CALL_ERR				scaler_hinting_error

#define TRASHED_OUTLINE_CACHE		scaler_bad_input

#endif

/************ For Debugging *************/

#ifdef XXX
#define DEBUG_ON
pascal 	Debug()						/* User break drop into Macsbug */
#ifdef	DEBUG_ON
extern	0xA9FF;
#else
{
	;
}
#endif

#ifdef	LEAVEOUT
pascal 	void DebugStr( aString) tt_int8 *aString; extern 0xABFF;
tt_int8 	*c2pstr();
#define BugInfo( aString) DebugStr( c2pstr(aString))
#endif

#endif	 
/****************************************/
