/*
 * @(#)kcms_io.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)kcms_io.c	1.24 98/12/02
 *
 *	This file defines KCMS file/memory I/O routines
 *
 *      int Kp_open()
 *      int Kp_close()
 *      int Kp_read()
 *      int Kp_write()
 *
 * All return 1 on success and 0 on failure.
 *
 * These functions are the same as those in io_file, with an added feature:
 * if the mode in Kp_open() is "m", then the the file descriptor
 * returned refers to a "memory" file, which may be read from and written
 * to using Kp_read() and Kp_write() just as an ordinary file. In this case,
 * the filename argument is ignored and a 4th and 5th argument, buf and size,
 * are required which specify a pointer to the buffer and the buffer size,
 * respectively.  Note that a call to Kp_close() is still required to free
 * up the descriptor.
 * Note that since this extension requires a multi-byte structure to maintain
 * the memory file info, the file ID is passed as a pointer to the structure
 * rather than the structure itself.
 * 
 * COPYRIGHT (c) 1989-2003, Eastman Kodak Company
 * As  an  unpublished  work pursuant to Title 17 of the United
 * States Code.  All rights reserved.
 */


#include "kcms_sys.h"
#include <stdarg.h>

#if defined (KPUNIX)
#include <sys/file.h>
#endif

#if defined (KPMAC)
#include <Files.h>
#include <string.h>
#endif

#if defined(KPWIN) || defined(KPWATCOM)
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif


/*----------------------------------------------------------------------*/
static int KpFdCheck (
			KpFd_t	FAR *P)
{
	if (NULL == P)
		return KCMS_IO_ERROR;

	if (	(P->type == KCMS_IO_NULLFILE)
			|| (P->type == KCMS_IO_SYSFILE)
			|| (P->type == KCMS_IO_MEMFILE)
		#if !defined KCMS_NO_CRC
			|| (P->type == KCMS_IO_CALCCRC)
		#endif
			) {
		return KCMS_IO_SUCCESS;
	}

	return KCMS_IO_ERROR;
}
/*----------------------------------------------------------------------*/
KpInt32_t KpOpen (
			KpChar_p		filename,
			KpChar_p		mode,
			KpFd_t			FAR *fd,
			KpFileProps_t	FAR *fileProps,
			...)
{
	if (fd == NULL)
	  return KCMS_IO_ERROR;
	
	switch (*mode) {
	case 'r':
	case 'w':
	case 'e':
		if (1 != KpFileOpen (filename, mode, fileProps, &fd->fd.sys)) {
			fd->type = KCMS_IO_NULLFILE;
			return KCMS_IO_ERROR;
		}

		fd->type = KCMS_IO_SYSFILE;
		break;

	case 'm': {
		va_list		ap;				/* arg pointer */
		char		KPHUGE *buf;			/* used only if mode = "m" */
		KpInt32_t	size;			/* used only if mode = "m" */

		va_start (ap, fileProps);
		buf = va_arg (ap, char KPHUGE *);
		size = va_arg (ap, KpInt32_t);
		va_end (ap);

		fd->fd.mem.buf = buf;
		fd->fd.mem.size = size;
		if ((NULL == fd->fd.mem.buf) || (fd->fd.mem.size <= 0))
		  return KCMS_IO_ERROR;

		fd->fd.mem.pos = 0;
		fd->type = KCMS_IO_MEMFILE;
		break;
	}

#if !defined KCMS_NO_CRC
	case 'c':
		fd->fd.crc32 = 0xFFFFFFFFL;	/* initial value for crc routine */
		fd->type = KCMS_IO_CALCCRC;
		break;
#endif

	default:
		fd->type = KCMS_IO_NULLFILE;
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}

/*----------------------------------------------------------------------*/
KpInt32_t Kp_open (
			KpChar_p	filename,
			KpChar_p	mode,
			KpFd_t		FAR *fd,
			ioFileChar	FAR *fileProps,
			...)
{
KpFileProps_t	kpfileProps;

	/* Convert ioFileChar to KpFileProps_t */
#if defined (KPMAC)
	if (fileProps != NULL) {
		kpfileProps.vRefNum = fileProps->vRefNum;
		kpfileProps.dirID = 0;
		strncpy (kpfileProps.fileType, fileProps->fileType, 5);
		strncpy (kpfileProps.creatorType, fileProps->creatorType, 5);
	} 
	else {
		kpfileProps.vRefNum = 0;
		kpfileProps.dirID = 0;
		strncpy (kpfileProps.fileType, "    ", 5);
		strncpy (kpfileProps.creatorType, "    ", 5);
	}
#endif

	if (fd == NULL)
	  return KCMS_IO_ERROR;
	
	switch (*mode) {
	case 'r':
	case 'w':
	case 'e':
		if (1 != KpFileOpen (filename, mode, &kpfileProps, &fd->fd.sys)) {
			fd->type = KCMS_IO_NULLFILE;
			return KCMS_IO_ERROR;
		}

		fd->type = KCMS_IO_SYSFILE;
		break;

	case 'm': {
		va_list		ap;				/* arg pointer */
		char		KPHUGE *buf;			/* used only if mode = "m" */
		KpInt32_t	size;			/* used only if mode = "m" */

		va_start (ap, fileProps);
		buf = va_arg (ap, char KPHUGE *);
		size = va_arg (ap, KpInt32_t);
		va_end (ap);

		fd->fd.mem.buf = buf;
		fd->fd.mem.size = size;
		if ((NULL == fd->fd.mem.buf) || (fd->fd.mem.size <= 0))
		  return KCMS_IO_ERROR;

		fd->fd.mem.pos = 0;
		fd->type = KCMS_IO_MEMFILE;
		break;
	}

#if !defined KCMS_NO_CRC
	case 'c':
		fd->fd.crc32 = 0xFFFFFFFFL;	/* initial value for crc routine */
		fd->type = KCMS_IO_CALCCRC;
		break;
#endif

	default:
		fd->type = KCMS_IO_NULLFILE;
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}

/*----------------------------------------------------------------------*/
KpInt32_t Kp_close (
			KpFd_t	FAR *fd)
{
	KpInt32_t		Status;

	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	Status = KCMS_IO_SUCCESS;
	switch (fd->type) {
	case KCMS_IO_NULLFILE:
		break;

	case KCMS_IO_SYSFILE:
		if (1 != KpFileClose (fd->fd.sys))
			Status = KCMS_IO_ERROR;

		break;

	case KCMS_IO_MEMFILE:
		/* force a bus error if fd is used after it's freed */
		fd->fd.mem.buf = (char FAR *) -1L;
		fd->fd.mem.size = 0;
		fd->fd.mem.pos = 0;	/* set offset to point to start of buffer */
		break;
		
#if !defined KCMS_NO_CRC
	case KCMS_IO_CALCCRC:
		fd->fd.crc32 = 0;
		break;
#endif

	default:
		Status = KCMS_IO_ERROR;
		break;
	}

	fd->type = KCMS_IO_NULLFILE;
	return Status;
}


/*----------------------------------------------------------------------*/
KpInt32_t Kp_read (
			KpFd_t			FAR *fd,
			KpLargeBuffer_t	buf,
			KpInt32_t		nbytes)
{
	void KPHUGE *memPtr;

	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	/* validate source */
	if (buf == NULL)
		return KCMS_IO_ERROR;


	switch (fd->type) {

	/* if regular file, read it */
	case KCMS_IO_SYSFILE: 
		if (1 != KpFileRead (fd->fd.sys, buf, &nbytes))
			return KCMS_IO_ERROR;

		break;

	/* if memory file, copy from memory */
	case KCMS_IO_MEMFILE:
		/* check for attemptting to read too much */
		if (fd->fd.mem.pos + nbytes > fd->fd.mem.size)
			return KCMS_IO_ERROR;

		/* validate source? */
		if (fd->fd.mem.buf == NULL)
			return KCMS_IO_ERROR;

		memPtr = (char KPHUGE *) fd->fd.mem.buf + fd->fd.mem.pos;

		KpMemCpy (buf, memPtr, nbytes);

		fd->fd.mem.pos += nbytes;
		break;

	default:
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}


/*----------------------------------------------------------------------*/
KpInt32_t Kp_write (
			KpFd_t			FAR *fd,
			KpLargeBuffer_t	buf,
			KpInt32_t		nbytes)
{
	void KPHUGE *memPtr;

	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	/* validate destination */
	if (buf == NULL)
		return KCMS_IO_ERROR;

	switch (fd->type) {

	/* regular file, write it */
	case KCMS_IO_SYSFILE:
		if (1 != KpFileWrite (fd->fd.sys, buf, nbytes))
			return KCMS_IO_ERROR;

		break;

	/* memory file, copy to memory */
	case KCMS_IO_MEMFILE:
		if (fd->fd.mem.pos + nbytes > fd->fd.mem.size)
			return KCMS_IO_ERROR;

		/* validate destination */
		if (fd->fd.mem.buf == NULL)
			return KCMS_IO_ERROR;

		memPtr = (char KPHUGE *) fd->fd.mem.buf + fd->fd.mem.pos;

		KpMemCpy (memPtr, buf, nbytes);

		fd->fd.mem.pos += nbytes;
		break;

#if !defined KCMS_NO_CRC
	/* calculate CRC */
	case KCMS_IO_CALCCRC:
		fd->fd.crc32 = Kp_Crc32 (fd->fd.crc32, nbytes, (KpChar_p) buf);
		break;
#endif

	default:
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}


/*----------------------------------------------------------------------*/
KpInt32_t Kp_skip (
			KpFd_t		FAR *fd,
			KpInt32_t	nbytes)
{

	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	switch (fd->type) {
	/* if regular file, error */
	case KCMS_IO_SYSFILE: 
		return KCMS_IO_ERROR;

	/* if memory file, move pointer */
	case KCMS_IO_MEMFILE:
		if (fd->fd.mem.pos + nbytes > fd->fd.mem.size)
			return KCMS_IO_ERROR;

		fd->fd.mem.pos += nbytes;
		break;

	default:
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}


/*----------------------------------------------------------------------*/
KpInt32_t Kp_set_position (
			KpFd_t		FAR *fd,
			KpInt32_t	nOffset)
{

	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	switch (fd->type) {
	/* if regular file, error */
	case KCMS_IO_SYSFILE: 
		return KCMS_IO_ERROR;

	/* if memory file, move pointer */
	case KCMS_IO_MEMFILE:
		if (nOffset >= fd->fd.mem.size)
			return KCMS_IO_ERROR;

		fd->fd.mem.pos = nOffset;
		break;

	default:
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}


/*----------------------------------------------------------------------*/
KpInt32_t Kp_get_position (
			KpFd_t		FAR *fd,
			KpInt32_p	nOffset)
{

	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	switch (fd->type) {
	/* if regular file, error */
	case KCMS_IO_SYSFILE: 
		return KCMS_IO_ERROR;

	/* if memory file, move pointer */
	case KCMS_IO_MEMFILE:
		*nOffset = fd->fd.mem.pos;
		break;

	default:
		return KCMS_IO_ERROR;
	}

	return KCMS_IO_SUCCESS;
}


#if !defined KCMS_NO_CRC
/*----------------------------------------------------------------------*/
KpInt32_t Kp_get_crc (
			KpFd_t		FAR *fd,
			KpCrc32_t	FAR *crc)
{
	if (KpFdCheck (fd) != KCMS_IO_SUCCESS)
		return KCMS_IO_ERROR;

	if (fd->type != KCMS_IO_CALCCRC)
		return KCMS_IO_ERROR;

	*crc = fd->fd.crc32;
	return KCMS_IO_SUCCESS;
}
#endif
