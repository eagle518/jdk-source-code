/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)R0MEM.H	1.4 03/12/19
 */

#if !defined(__R0MEM_H__)

#define __R0MEM_H__

#if defined(__cplusplus)
extern "C"
{
#endif


        void *R0MEM_allocate( unsigned long ulLength );

         int  R0MEM_free( void *pMemory );

         int  R0MEM_lock( void *pMemory, unsigned long ulLength );

         int  R0MEM_unlock( void *pMemory, unsigned long ulLength );

unsigned long R0MEM_sizeof( void *pMemory );

         int  R0MEM_IsBadReadPtr( void *pMemory, unsigned long ulLength );

#if defined(__cplusplus)
}
#endif


#endif

