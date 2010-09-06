#include <stdio.h>
#include <dlfcn.h>
#include <Xm/Xm.h>
#include "VendorSI.h"

/* Motif 2.1 changes - Colm
 *
 * Motif re-defines Xt's vendorShellClassRec - This file exists to
 * make Motif's structure available to other libraries that don't
 * have a direct dependency on Motif, but which can ensure that
 * Motif's version of this structure is used by linking against this
 * source file's object.
 *
 * _XmInitVendorShell() below will copy the definition structure
 * (from VendorS.c) into this structure at run-time, so that those
 * libs with a loose dependency on Motif can get a correctly initialised
 * version of vendorShellClassRec.
 */

VendorShellClassRec vendorShellClassRec = {
    {
	0,	/* superclass	*/ 
	0,	/* class_name 		*/ 
	0,	/* size 		*/ 
	0,	/* Class Initializer 	*/ 
	0,	/* class_part_init 	*/ 
	0,	/* Class init'ed ? 	*/ 
	0,	/* initialize 	*/ 
	0,	/* initialize_hook	*/ 
	0,	/* realize 	*/ 
	0,	/* actions 	*/ 
	0,	/* num_actions	*/ 
	0,	/* resources 	*/ 
	0,	/* resource_count 	*/ 
	0,	/* xrm_class 	*/ 
	0,	/* compress_motion 	*/ 
	0,	/* compress_exposure 	*/ 
	0,	/* compress_enterleave	*/ 
	0,	/* visible_interest 	*/ 
	0,			/* destroy	*/ 
	0,			/* resize 	*/ 
	0,				/* expose	*/ 
	0,		/* set_values 	*/ 
	0,	/* set_values_hook */ 
	0,	/* set_values_almost */ 
	0,	/* get_values_hook */ 
	0,	/* accept_focus 	*/ 
	0,	/* intrinsics version 	*/ 
	0,	/* callback offsets 	*/ 
	0,	/* tm_table	*/ 
	0,	/* query_geometry */ 
	0,	/* display_accelerator */ 
	0,	/* extension		*/ 
 },	
 { 					/* composite_class	*/
	0,		/* geometry_manager 	*/ 
	0,		 	/* change_managed 	*/ 
	0, 		/* insert_child 	*/ 
	0, 		/* delete_child 	*/ 
	0,/* extension */ 
 },                           
    {                            	/* shell class		*/
	0,	/* extension 		*/ 
    }, 
    {                            	/* wmshell class	*/
	NULL, 				/* extension            */ 
    }, 	
    {   				/* vendorshell class	*/
	NULL, 				/* extension            */ 
    }                            
};	   

void _XmInitVendorShell(void *dst, void *src)
{
	if (dst != src)
		memcpy(dst, src, sizeof(VendorShellClassRec));
}
