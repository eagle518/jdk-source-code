/*
 * @(#)io.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)io.c	1.8 99/02/09
 
	This file contains functions to read and write binary fut files.

	Author:	Kit Enscoe, George Pawle

	All opens, closes, reads and writes are performed with the functions
	KpOpen, Kp_close, Kp_read, and Kp_write, respectively
	to provide an "operating system independent" i/o interface.  These
	functions are implemented differently for each operating system, and are
	defined in the library kcms_sys.

	To handle architecture dependent byte ordering, byte swapping is performed
	as neccessary when reading a file, by checking the byte ordering of the
	"magic" numbers.  The "standard" byte ordering is Most Significant Byte First
	(e.g. Sun, Macintosh) but this default can be overridden (see below).

	BEWARE of asynchronous usage of a fut while it is being written out
	since its tables may be temporarily byte reversed.

	COPYRIGHT (c) 1989-1999 Eastman Kodak Company.
	As  an  unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */


#include "string.h" 
#include "fut_util.h"

static fut_chan_p	fut_read_chan	(KpFd_p, fut_hdr_p, KpInt32_t);
static fut_itbl_p	fut_read_itbl	(KpFd_p, fut_hdr_p);
static fut_otbl_p	fut_read_otbl	(KpFd_p, fut_hdr_p);
static fut_gtbl_p	fut_read_gtbl	(KpFd_p, KpInt32_t);
static KpInt32_t	fut_write_chan	(KpFd_p, fut_chan_p, chan_hdr_p);
static KpInt32_t	fut_write_itbl	(KpFd_p, fut_itbl_p	);
static KpInt32_t	fut_write_otbl	(KpFd_p, fut_otbl_p);
static KpInt32_t	fut_write_gtbl	(KpFd_p, fut_gtbl_p);
static KpInt32_t	fut_skip_idstr	(KpFd_p, fut_hdr_p);


/* fut_load_fp loads a fut from the file named "filename", performing the
 * open and close automatically.  It returns a pointer to the loaded fut
 * or NULL on error.
 */
fut_p
	fut_load_fp (	KpChar_p		filename,
					KpFileProps_t	fileProps)
{
fut_p		fut = NULL;
KpFd_t		fd;
fut_hdr_t	futio;

	if (KpOpen (filename, "r", &fd, &fileProps) ) {

		if (Kp_read (&fd, (KpGenericPtr_t)&futio.magic, sizeof(KpInt32_t))) {

			if ((futio.magic == FUT_CIGAM) || (futio.magic == FUT_MAGIC) ) {

				fut = fut_alloc_fut ();				/* allocate a new fut structure */
				if (fut != NULL) {
					if ( ! fut_read_futhdr (&fd, &futio) ||		/* read in the encoded header, */
						! fut_skip_idstr (&fd, &futio) ||		/* skip the id string, */
						! fut_read_tbls (&fd, fut, &futio) ||	/* read the tables, */
						! fut_io_decode (fut, &futio) ) {		/* and decode file header */
						fut = fut_free (fut);				/* if error, free fut */
					}
				}
			}
		}

		(void) Kp_close (&fd);
	}

	return (fut);
}


/* fut_store_fp stores fut to the file named "filename", performing the
 * open and close automatically.  Returns 1 on success, 0 or negative
 * on error.
 */
KpInt32_t
	fut_store_fp (	fut_p			fut,
					KpChar_p		filename,
					KpFileProps_t	fileProps)
{
KpFd_t		fd;
KpInt32_t	ret = 0;
fut_hdr_t	futio;

	/* Open with the new e mode for exclusive.  The file must be closed, or at least unlocked when done */
	if (KpOpen (filename, "e", &fd, &fileProps) ) {

		if ((fut_io_encode (fut, &futio)) &&				/* encode the header */
			(fut_write_hdr (&fd, &futio) )) {				/* write out the header */
			ret = fut_write_tbls (&fd, fut, &futio);		/* and the tables */
		}

		(void) Kp_close (&fd);
	}

	return (ret);
}


/* fut_write_hdr writes the header information to an open binary fut file
 * from a futio header structure.  This structure was initialized by a call to
 * fut_io_encode and contains information describing shared and identity (ramp)
 * tables and also indicates the content of the remaining part of the file.
 *
 * It returns 1 (TRUE) on success and 0 (FALSE) if a write error occured.
 */
KpInt32_t
	fut_write_hdr (	KpFd_p		fd,
					fut_hdr_p	hdr)
{
KpInt32_t	i, ret, format;
chan_hdr_p	chan;

#if defined (KPLSBFIRST)
	fut_swab_hdr (hdr);		/* swap bytes if necessary */
	format = FUT_CIGAM;		/* format being written */
#else
	format = FUT_MAGIC;		/* format being written */
#endif
	
		/* write each element, quitting if write error occurs */
	ret = Kp_write (fd, (KpGenericPtr_t)&format, sizeof(KpInt32_t) ) &&
		Kp_write (fd, (KpGenericPtr_t)&hdr->version, sizeof(KpInt32_t) ) &&
		Kp_write (fd, (KpGenericPtr_t)&hdr->idstr_len, sizeof(KpInt32_t) ) &&
		Kp_write (fd, (KpGenericPtr_t)&hdr->order, sizeof(KpInt32_t) ) &&
		Kp_write (fd, (KpGenericPtr_t) hdr->icode, sizeof(KpInt32_t)*FUT_NCHAN);

	for (i = 0, chan = hdr->chan; (ret > 0) && (i < FUT_NCHAN); ++i, chan++) {

		ret = Kp_write (fd, (KpGenericPtr_t) chan->size, sizeof(KpInt16_t)*FUT_NCHAN) &&
			Kp_write (fd, (KpGenericPtr_t) chan->icode, sizeof(KpInt32_t)*FUT_NCHAN) &&
			Kp_write (fd, (KpGenericPtr_t)&chan->ocode, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&chan->gcode, sizeof(KpInt32_t));
	}

	ret = (ret > 0) && Kp_write (fd, (KpGenericPtr_t)&hdr->more, sizeof(KpInt32_t));

#if defined (KPLSBFIRST)
	fut_swab_hdr (hdr);		/* always swap bytes back again */
#endif

	return  (ret);
}


/* fut_read_futhdr reads everything after the magic number, which is
 * needed for use with alternate transform types.
 *
 * It returns 1 (TRUE) on success and 0 (FALSE)  if a read error occured.
 */
KpInt32_t
	fut_read_futhdr (	KpFd_p		fd,
						fut_hdr_p	hdr)
{
KpInt32_t	i;
chan_hdr_p	chan;
		
	hdr->iDataClass = KCP_UNKNOWN;
	hdr->oDataClass = KCP_UNKNOWN;

	if ( ! Kp_read (fd, (KpGenericPtr_t)&hdr->version, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&hdr->idstr_len, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&hdr->order, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t) hdr->icode, sizeof(KpInt32_t)*FUT_NCHAN) ) {
		return  0;
	}

	for (i=0, chan=hdr->chan; i<FUT_NCHAN; ++i, chan++) {
		if ( ! Kp_read (fd, (KpGenericPtr_t) chan->size, sizeof(KpInt16_t)*FUT_NCHAN) ||
			! Kp_read (fd, (KpGenericPtr_t) chan->icode, sizeof(KpInt32_t)*FUT_NCHAN) ||
			! Kp_read (fd, (KpGenericPtr_t)&chan->ocode, sizeof(KpInt32_t)) ||
			! Kp_read (fd, (KpGenericPtr_t)&chan->gcode, sizeof(KpInt32_t)) ) {
			return  0;
		}
	}

	if ( ! Kp_read (fd, (KpGenericPtr_t)&hdr->more, sizeof(KpInt32_t)) ) {
		return 0;
	}

	switch (hdr->magic) {
	case FUT_MAGIC:			/* bytes in correct order, nothing to do */
		break;

	case FUT_CIGAM:			/* bytes are reversed, swap them now! */
		fut_swab_hdr (hdr);
		break;

	default:				/* garbage, return error */
		return 0;
	}

	return	1;
}


/* fut_skip_idstr reads and discards the idstring from an open file descriptor.
 * Returns: 
 * 1 on success
 * 0 on id string or Kp_read error
 */
KpInt32_t
	fut_skip_idstr (	KpFd_p		fd,
						fut_hdr_p	hdr)
{
KpInt32_t	nbytes;
KpChar_t	dummy;
				
	nbytes = hdr->idstr_len;	/* Get length of idstring.  This includes the null terminator */

	/* skip past the idstr by reading into a scratch buffer and throwing away the data */
	while (nbytes > 0) {
		if ( ! Kp_read (fd, (KpGenericPtr_t) &dummy, sizeof(dummy)) ) {
			return (0);
		}
		nbytes--;
	}

	return (1);
}


/* fut_write_tbls writes the tables of a fut to an open file descriptor.
 * The header and idstring should have already been written to the file.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_write error
 *  -2 to -4 on a table specific error
 */
KpInt32_t
	fut_write_tbls (	KpFd_p		fd,
						fut_p		fut,
						fut_hdr_p	hdr)
{
KpInt32_t	i, ret = 1;

						/* write out the input tables */
	for ( i = 0; (i < FUT_NICHAN) && (ret > 0); i++ ) {
		if ( hdr->icode[i] == FUTIO_UNIQUE ) {
			ret = fut_write_itbl (fd, fut->itbl[i]);
		}
	}

						/* write out the output channels */
	for ( i = 0; (i < FUT_NOCHAN) && (ret > 0); i++ ) {
		if ( fut->chan[i] != 0 ) {
			ret = fut_write_chan (fd, fut->chan[i], & hdr->chan[i]);
		}
	}

	return ( (ret > 0) ? 1 : ret );
}


/* fut_read_tbls reads the tables from an open file descriptor and assigns
 * them to the already allocated 'fut'.  The header and idstring must have
 * been previously read from the file.  Upon a succesful return,
 * fut_io_decode must be called to generate linear tables and share the
 * shared ones.
 *
 * Returns 1 (TRUE) on success, 0 (FALSE) on failure.
 *
 * Note: on failure, some of the tables and channels amy be partially
 * read in and assigned to the fut.  It is a good idea to immediately
 * free the fut in this case.
 */
KpInt32_t
	fut_read_tbls (	KpFd_p		fd,
					fut_p		fut,
					fut_hdr_p	hdr)
{
KpInt32_t	i;

				/* make sure fut has been allocated and
					contains the magic number. */
	if ( ! IS_FUT(fut) ) {
		return (0);
	}

						/* read in the input tables */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ( hdr->icode[i] == FUTIO_UNIQUE ) {
			fut->itbl[i] = fut_read_itbl (fd, hdr);
			if ( fut->itbl[i] == NULL) {
				return (0);
			}
			fut->itblHandle[i] = fut->itbl[i]->handle;
		}
	}
						/* read in the output channels */
	for ( i=0; i<FUT_NOCHAN; i++ ) {
		if ( hdr->chan[i].gcode != 0 ) {
			fut->chan[i] = fut_read_chan (fd, hdr, i);
			if (fut->chan[i] == NULL ) {
				return (0);
			}
			fut->chanHandle[i] = fut->chan[i]->handle;
		}
	}

	return (1);
}


/* fut_write_chan writes the tables (input, output, and grid) for an output
 * channel to an open file descriptor.  The chan_hdr_t from the fut_hdr_t
 * structure previously obtained from fut_io_encode is needed to specify which
 * tables are to be written out (shared table and ramp tables are not).
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_write error
 * -5 on invalid channel error
 *  -2 to -4 on a table specific error
 */
KpInt32_t
  fut_write_chan (	KpFd_p		fd,
  					fut_chan_p	chan,
  					chan_hdr_p	chanio)
{
KpInt32_t	i, ret = 1;

	if ( ! IS_CHAN (chan) )
		return (-5);

						/* write out the input tables */
	for ( i = 0; (i < FUT_NICHAN) && (ret > 0); i++ ) {
		if ( chanio->icode[i] == FUTIO_UNIQUE )
			ret = fut_write_itbl (fd, chan->itbl[i]);
	}

						/* write out the output table */
	if ( ret > 0 ) {
		if ( chanio->ocode == FUTIO_UNIQUE )
			ret = fut_write_otbl (fd, chan->otbl);
	}

						/* write out the grid tables */
	if ( ret > 0 ) {
		if ( chanio->gcode == FUTIO_UNIQUE )
			ret = fut_write_gtbl (fd, chan->gtbl);
	}

	return (ret);
}


/* fut_read_chan reads the tables (input, output, and grid) for an output
 * channel from an open file descriptor.  The chan_hdr_t from the fut_hdr_t
 * structure previously read in is needed to determine which tables exist
 * for the channel and need to be read in.
 *
 * It returns a newly allocated fut_chan_t structure on success and NULL
 * on failure.
 */
fut_chan_p	
	fut_read_chan (	KpFd_p		fd,
					fut_hdr_p	hdr,
					KpInt32_t	whichChan)
{
chan_hdr_p	chanio = &hdr->chan[whichChan];
KpInt32_t	i, ret = 1, tbl_size;
fut_chan_p	chan;

					/* allocate a new chan structure */
	chan = fut_alloc_chan();
	if ( chan == NULL ) {
		return (NULL);
	}

						/* read in the input tables */
	for ( i = 0; (i < FUT_NICHAN) && ret; i++ ) {
		if ( chanio->icode[i] == FUTIO_UNIQUE ) {
			chan->itbl[i] = fut_read_itbl (fd, hdr);
			if (chan->itbl[i] != NULL){
				chan->itblHandle[i] = chan->itbl[i]->handle;
			} else {
				ret = KPFALSE;
			}
		}
	}

						/* read in the output table */
	if ( ret ) {
		if ( chanio->ocode == FUTIO_UNIQUE ) {
			chan->otbl = fut_read_otbl (fd, hdr);
			if ( chan->otbl != NULL ) {
				chan->otblHandle = chan->otbl->handle;
			} else {
				ret = KPFALSE;
			}
		}
	}
						/* read in the grid table */
	if ( ret ) {
		if ( chanio->gcode == FUTIO_UNIQUE ) {
			tbl_size = sizeof(fut_gtbldat_t);
			for (i = 0; i < FUT_NICHAN; i++) {
				if (chanio->size[i] != 0) {
					tbl_size *= chanio->size[i];
				}
			}
			chan->gtbl = fut_read_gtbl (fd, tbl_size);
			if (chan->gtbl != NULL) {
				chan->gtblHandle = chan->gtbl->handle;
			} else {
				ret = KPFALSE;
			}
		} else {
			ret = KPFALSE;
		}
	}

	if ( ! ret ) {
		fut_free_chan (chan);
		chan = NULL;
	}

	return (chan);
}


/* fut_write_itbl writes an input table to an open binary fut file.
 * Individual members are written one by one to reduce machine architecture
 * dependency, while arrays of ints are written in a single write call.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_write error
 * -2 on invalid input table error
 *
 * If necessary, we swap bytes before writing out the table and swap them
 * back again afterwards.  The machine reading back the table can always
 * determine the byte ordering by examining the magic numbers.
 */
KpInt32_t
	fut_write_itbl (	KpFd_p		fd,
						fut_itbl_p	itbl)
{
KpInt32_t ignored = 0, ret;

	if ( ! IS_ITBL(itbl) )
		return (-2);

#if defined (KPLSBFIRST)
	fut_swab_itbl (itbl);	/* swap bytes if necessary */
#endif

	/* write out the itbl structure */
	ret = Kp_write (fd, (KpGenericPtr_t)&itbl->magic, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&itbl->size, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t) itbl->tbl, sizeof(fut_itbldat_t)*(FUT_INPTBL_ENT+1));

#if defined (KPLSBFIRST)
	fut_swab_itbl (itbl);	/* always swap bytes back again */
#endif

	return (ret ? 1 : -1);
}


/* fut_read_itbl reads in an input table from an open binary fut file.
 * Individual members are read in one by one to reduce machine architecture
 * dependency, while arrays of ints are read in a single read call.  If
 * the magic number read in is byte reversed, then we swap all the bytes
 * in the structure.
 *
 * A pointer to a newly allocated table is returned, or NULL on error.
 */
fut_itbl_p	
	fut_read_itbl (	KpFd_p		fd,
					fut_hdr_p	hdr)
{
fut_itbl_p	itbl;
KpInt32_t	ignored;

					/* create an itbl to read into */
	itbl = fut_new_itblEx (KCP_PT_TABLES, hdr->iDataClass, 2, NULL, NULL);
	if ( itbl == NULL ) {
		return (NULL);
	}
					/* read in itbl structure.  If read error
						then free structure and return error */
	if ( ! Kp_read (fd, (KpGenericPtr_t)&itbl->magic, sizeof(KpInt32_t)) ||
		! (itbl->magic == FUT_IMAGIC || itbl->magic == FUT_CIGAMI) ||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&itbl->size, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t) itbl->tbl, sizeof(fut_itbldat_t)*(FUT_INPTBL_ENT+1)) )	{

		goto ErrOut;
	}

	if ( itbl->magic == FUT_CIGAMI ) {	/* bytes are reversed, swap them now! */
		fut_swab_itbl (itbl);
	}

	itbl->dataClass = hdr->iDataClass;
	
	/* Clip the input table entries to avoid memory violation errors during
	 * interpolation (see note in fut_calc_itbl()).  This is only necessary
	 * for older fut files created before the fix to fut_calc_itbl.  Perhaps
	 * we should use a different file version number to avoid doing this
	 * all the time.  On the other hand, this is only performed once on
	 * input for 257 elements so why bother.
	 */
	{
		KpInt32_t		i;
		fut_itbldat_p	idat;
		fut_itbldat_t	imax;

				/* maximum itbl entry value */
		imax = (itbl->size - 1) << FUT_INP_DECIMAL_PT;

		for ( i=FUT_INPTBL_ENT+1, idat=itbl->tbl; --i >= 0; idat++ ) {
			/*
			* may as well check for *any* illegal values while
			* we're at it, to prevent any "mysterious" memory
			* violations due to garbled file data.  Note that
			* the unsigned compare catches illegal values both
			* negative and too large in one test.
			*/
			if ( ((unsigned long)(*idat)) >= ((unsigned long)imax)) {
				if ( *idat == imax ) {
					*idat = imax-1;	/* next lower value to avoid boundary */
				}
				else {
					goto ErrOut;
				}
			}
		}
	}

	return (itbl);


ErrOut:
	itbl->magic = FUT_IMAGIC;		/* in case garbled or reversed */
	fut_free_itbl (itbl);
	return (NULL);
}


/* fut_write_otbl writes an output table to an open binary fut file.
 * Individual members are written one by one to reduce machine architecture
 * dependency, while arrays of ints are written in a single write call.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_write error
 * -3 on invalid output table error
 *
 * If necessary, we swap bytes before writing out the table and swap them
 * back again afterwards.  The machine reading back the table can always
 * determine the byte ordering by examining the magic numbers.
 */
KpInt32_t
	fut_write_otbl (	KpFd_p		fd,
						fut_otbl_p	otbl)
{
KpInt32_t	ignored = 0, ret;

	if ( ! IS_OTBL(otbl) ) {
		return (-3);
	}

#if defined (KPLSBFIRST)
	fut_swab_otbl (otbl);	/* swap bytes if necessary */
#endif

						/* write out the otbl structure */
	ret = Kp_write (fd, (KpGenericPtr_t)&otbl->magic, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t) otbl->tbl, sizeof(fut_otbldat_t)*FUT_OUTTBL_ENT);

#if defined (KPLSBFIRST)
	fut_swab_otbl (otbl);	/* always swap bytes back again */
#endif

	return (ret ? 1 : -1);
}


/* fut_read_otbl reads in an output table from an open binary fut file.
 * Individual members are read in one by one to reduce machine architecture
 * dependency, while arrays of ints are read in a single read call.  If
 * the magic number read in is byte reversed, then we swap all the bytes
 * in the structure.
 *
 * A pointer to a newly allocated table is returned, or NULL on error.
 */
fut_otbl_p
	fut_read_otbl (	KpFd_p		fd,
					fut_hdr_p	hdr)
{
fut_otbl_p	otbl;
KpInt32_t	ignored;

					/* create an otbl to read into */
	otbl = fut_new_otblEx (KCP_PT_TABLES, hdr->oDataClass, NULL, NULL);
	if ( otbl == NULL ) {
		return (NULL);
	}
					/* read in otbl structure.  If read error
						then free otbl and return error */
	if ( ! Kp_read (fd, (KpGenericPtr_t)&otbl->magic, sizeof(KpInt32_t)) ||
		! (otbl->magic == FUT_OMAGIC || otbl->magic == FUT_CIGAMO) ||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t) otbl->tbl, sizeof(fut_otbldat_t)*FUT_OUTTBL_ENT) )		{


		/* Note: set magic number for fut_free_otbl in case garbled */
		otbl->magic = FUT_OMAGIC;
		fut_free_otbl (otbl);
		return (NULL);
	}

	if ( otbl->magic == FUT_CIGAMO ) {	/* bytes are reversed, swap them now! */
		fut_swab_otbl (otbl);
	}

	otbl->dataClass = hdr->oDataClass;

	return (otbl);
}


/* fut_write_gtbl writes a grid table to an open binary fut file.
 * Individual members are written one by one to reduce machine architecture
 * dependency, while arrays of ints are written in a single write call.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_write error
 * -4 on invalid channel error
 *
 * If necessary, we swap bytes before writing out the table and swap them
 * back again afterwards.  The machine reading back the table can always
 * determine the byte ordering by examining the magic numbers.
 */
KpInt32_t
	fut_write_gtbl (	KpFd_p		fd,
						fut_gtbl_p	gtbl)
{
KpInt32_t	ret, ignored = 0, tbl_size;

						/* make sure gtbl has grid table
							array allocated */
	if ( ! IS_GTBL(gtbl) || gtbl->tbl == (fut_gtbldat_p)0 )
		return (-4);

	tbl_size = gtbl->tbl_size;	/* save tbl_size first */

#if defined (KPLSBFIRST)
	fut_swab_gtbl (gtbl);		/* then swap bytes if necessary */
#endif

						/* write out the gtbl structure */
	ret = Kp_write (fd, (KpGenericPtr_t)&gtbl->magic, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t)&gtbl->tbl_size, sizeof(KpInt32_t)) &&
		Kp_write (fd, (KpGenericPtr_t) gtbl->size, sizeof(KpInt16_t)*FUT_NCHAN) &&
		Kp_write (fd, (KpGenericPtr_t) gtbl->tbl, tbl_size);

#if defined (KPLSBFIRST)
	fut_swab_gtbl (gtbl);		/* always swap bytes back again */
#endif

	return (ret ? 1 : -1);
}


/* fut_read_gtbl reads in an grid table from an open binary fut file.
 * Individual members are read in one by one to reduce machine architecture
 * dependency, while arrays of ints are read in a single read call.  If
 * the magic number read in is byte reversed, then we swap all the bytes
 * in the structure.
 *
 * A pointer to a newly allocated table is returned, or NULL on error.
 */
fut_gtbl_p 
	fut_read_gtbl (	KpFd_p	fd, KpInt32_t	gtbl_size)
{
fut_gtbl_p	gtbl;
KpInt32_t	tbl_size, gtblMagic, ignored;

					/* allocate a gtbl structure */
	gtbl = fut_alloc_gtbl ();
	if ( gtbl == NULL ) {
		return (NULL);
	}

	/* read in gtbl structure.  If read error, or is garbage, or no table data, then
	 * free gtbl structure and return error */
	if ( ! Kp_read (fd, (KpGenericPtr_t)&gtbl->magic, sizeof(KpInt32_t)) ||
		! ((gtbl->magic == FUT_GMAGIC) || (gtbl->magic == FUT_CIGAMG)) ||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t))	||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&ignored, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t)&gtbl->tbl_size, sizeof(KpInt32_t)) ||
		! Kp_read (fd, (KpGenericPtr_t) gtbl->size, sizeof(KpInt16_t)*FUT_NCHAN)) {

		DIAG("fut_read_gtbl: error reading grid table struct.\n", 0);
		goto ErrOut;
	}

	/* Its kinda kludgy, but we must first get tbl_size in proper byte ordering
	 * before we can allocate and read in the grid table */
	gtblMagic = gtbl->magic;
	tbl_size = gtbl->tbl_size;
	if (gtblMagic == FUT_CIGAMG) {
		Kp_swab32 ((KpGenericPtr_t)&tbl_size, 1);
	}

	if ((tbl_size <= 0) || (tbl_size != gtbl_size)) {		/* verify tbl_size */
		goto ErrOut;
	}

	/* allocate memory for the table */
	gtbl->magic = FUT_GMAGIC;
	gtbl->tbl_size = tbl_size;
	gtbl->tbl = fut_alloc_gtbldat (gtbl);
	if ( gtbl->tbl == NULL ) {
		goto ErrOut;
	}

	/* read in the table data */
	if ( ! Kp_read (fd, (KpGenericPtr_t)gtbl->tbl, tbl_size) ) {
		goto ErrOut;
	}

	/* See if bytes need swapping. */
	if (gtblMagic == FUT_CIGAMG) {		/* bytes are reversed, swap them now! */
		fut_swab_gtbl (gtbl);
	}
	
	gtbl->magic = FUT_GMAGIC;
	gtbl->tbl_size = tbl_size;

	return (gtbl);


ErrOut:
	gtbl->magic = FUT_GMAGIC;		/* in case garbled or reversed */
	fut_free_gtbl (gtbl);
	return (NULL);
}


#if !defined KCMS_NO_CRC
/* fut_cal_crc
 *
 *	An signed 32-bit fut cyclical redundancy check (CRC)
 */
KpInt32_t
	fut_cal_crc (	fut_p		fut,
					KpInt32_p	crc)
{
KpFd_t  fd;
KpInt32_t		ret = 1;
fut_hdr_t	futio;

	if ( ! IS_FUT(fut) ) {	/* check for valid fut */
		return (0);
	}

	if ( ! KpOpen (NULL, "c", &fd, NULL) ) {
		return (-1);
	}

	if ( ! fut_io_encode (fut, &futio) ) {	/* encode futio hdr */
		return (0);
	}

	ret = fut_write_tbls (&fd, fut, &futio);	/* write out the tables */

	Kp_get_crc (&fd, (KpCrc32_t FAR *)crc);

	(void) Kp_close (&fd);

	return (ret);
}
#endif
