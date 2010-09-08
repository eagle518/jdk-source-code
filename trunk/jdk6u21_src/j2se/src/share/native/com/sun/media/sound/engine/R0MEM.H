/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)R0MEM.H	1.6 10/03/23
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

