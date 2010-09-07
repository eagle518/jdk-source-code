/*
 * @(#)MOD_load_uni.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**
**	History	-
**	12/1/96		Created
**	7/17/97		Added compile time switch
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
*/
/*****************************************************************************/
#include "X_API.h"

#if USE_MOD_API == TRUE
#include "MOD_mikmod.h"
/*

Name:
LOAD_UNI.C

Description:
UNIMOD (mikmod's internal format) module loader.

Portability:
All systems - all compilers (hopefully)

*/


static BOOL UNI_Test(void)
{
    char id[4];
    if(!_mm_read_str(id,4)) return 0;
    if(!memcmp(id,"UN05",4)) return 1;
    return 0;
}


static BOOL UNI_Init(void)
{
    return 1;
}


static void UNI_Cleanup(void)
{
    ;
}


static char *StrRead(void)
{
    char *s;
    UWORD len;

    len=_mm_read_I_UWORD();
    if(!len) return NULL;

    s=(char *)mod_malloc(len+1);
    _mm_read_str(s,len);
    s[len]=0;

    return s;
}


static UBYTE *TrkRead(void)
{
    UBYTE *t;
    UWORD len;

    len=_mm_read_I_UWORD();
    t=(UBYTE *)mod_malloc(len);
    _mm_read_str((char *) t,len);
    return t;
}



static BOOL UNI_Load(void)
{
    int t,u;

    _mm_fseek(4,SEEK_SET);

    /* try to read module header */

    of.numchn	=_mm_read_UBYTE();
    of.numpos	=_mm_read_I_UWORD();
    of.reppos	=_mm_read_I_UWORD();
    of.numpat	=_mm_read_I_UWORD();
    of.numtrk	=_mm_read_I_UWORD();
    of.numins	=_mm_read_I_UWORD();
    of.initspeed=_mm_read_UBYTE();
    of.inittempo=_mm_read_UBYTE();
    _mm_read_UBYTES(of.positions,256);
    _mm_read_UBYTES(of.panning,32);
    of.flags	=_mm_read_UBYTE();

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    of.songname=StrRead();
    of.modtype=StrRead();
    of.comment=StrRead();   /* <- new since UN01 */

    /*	printf("Song: %s\nModty: %s\n",of.songname,of.modtype);
     */

    if(!AllocInstruments()) return 0;
    if(!AllocTracks()) return 0;
    if(!AllocPatterns()) return 0;

    /* Read sampleinfos */

    for(t=0;t<of.numins;t++){

	INSTRUMENT *i=&of.instruments[t];

	i->numsmp=_mm_read_UBYTE();
	_mm_read_UBYTES(i->samplenumber,96);

	i->volflg=_mm_read_UBYTE();
	i->volpts=_mm_read_UBYTE();
	i->volsus=_mm_read_UBYTE();
	i->volbeg=_mm_read_UBYTE();
	i->volend=_mm_read_UBYTE();

	for(u=0;u<12;u++){
	    i->volenv[u].pos=_mm_read_I_SWORD();
	    i->volenv[u].val=_mm_read_I_SWORD();
	}

	i->panflg=_mm_read_UBYTE();
	i->panpts=_mm_read_UBYTE();
	i->pansus=_mm_read_UBYTE();
	i->panbeg=_mm_read_UBYTE();
	i->panend=_mm_read_UBYTE();

	for(u=0;u<12;u++){
	    i->panenv[u].pos=_mm_read_I_SWORD();
	    i->panenv[u].val=_mm_read_I_SWORD();
	}

	i->vibtype	=_mm_read_UBYTE();
	i->vibsweep	=_mm_read_UBYTE();
	i->vibdepth	=_mm_read_UBYTE();
	i->vibrate	=_mm_read_UBYTE();

	i->volfade	=_mm_read_I_UWORD();
	i->insname	=StrRead();

	/*		printf("Ins: %s\n",i->insname);
	 */
	if(!AllocSamples(i)) return 0;

	for(u=0;u<i->numsmp;u++){

	    SAMPLE *s=&i->samples[u];

	    s->c2spd	= _mm_read_I_UWORD();
	    s->transpose= _mm_read_SBYTE();
	    s->volume	= _mm_read_UBYTE();
	    s->panning	= _mm_read_UBYTE();
	    s->length	= _mm_read_I_ULONG();
	    s->loopstart= _mm_read_I_ULONG();
	    s->loopend	= _mm_read_I_ULONG();
	    s->flags	= _mm_read_I_UWORD();
	    s->seekpos	= 0;

	    s->samplename=StrRead();
	}
    }

    /* Read patterns */

    _mm_read_I_UWORDS(of.pattrows,of.numpat);
    _mm_read_I_UWORDS(of.patterns,of.numpat*of.numchn);

    /* Read tracks */

    for(t=0;t<of.numtrk;t++){
	of.tracks[t]=TrkRead();
    }

    return 1;
}


LOADER load_uni={
    NULL,
    "UNI",
    "Portable UNI loader v0.3",
    UNI_Init,
    UNI_Test,
    UNI_Load,
    UNI_Cleanup
};

#endif	// USE_MOD_API

