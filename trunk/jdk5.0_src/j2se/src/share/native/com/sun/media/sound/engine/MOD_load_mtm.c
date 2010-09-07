/*
 * @(#)MOD_load_mtm.c	1.12 03/12/19
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
**	12/18/97	Cleaned up some warnings
*/
/*****************************************************************************/
#include "X_API.h"

#if USE_MOD_API == TRUE
#include "MOD_mikmod.h"
/*

Name:
LOAD_MTM.C

Description:
MTM module loader

Portability:
All systems - all compilers (hopefully)

*/

/**************************************************************************
**************************************************************************/


typedef struct MTMSAMPLE{
    char  samplename[22];
    ULONG length;
    ULONG reppos;
    ULONG repend;
    UBYTE finetune;
    UBYTE volume;
    UBYTE attribute;
} MTMSAMPLE;



typedef struct MTMHEADER{
    UBYTE id[3];                            /* MTM file marker */
    UBYTE version;                          /* upper major, lower nibble minor version number */
    char  songname[20];                     /* ASCIIZ songname */
    UWORD numtracks;                        /* number of tracks saved */
    UBYTE lastpattern;                      /* last pattern number saved */
    UBYTE lastorder;                        /* last order number to play (songlength-1) */
    UWORD commentsize;                      /* length of comment field */
    UBYTE numsamples;                       /* number of samples saved */
    UBYTE attribute;                        /* attribute byte (unused) */
    UBYTE beatspertrack;            /* */
    UBYTE numchannels;                      /* number of channels used */
    UBYTE panpos[32];                       /* voice pan positions */
} MTMHEADER;


typedef struct MTMNOTE{
    UBYTE a,b,c;
} MTMNOTE;


/**************************************************************************
**************************************************************************/



static MTMHEADER *mh_mtm;
static MTMNOTE *mtmtrk;
static UWORD pat[32];

const char MTM_Version[]="MTM";



static BOOL MTM_Test(void)
{
    char id[3];
    if(!_mm_read_str(id,3)) return 0;
    if(!memcmp(id,"MTM",3)) return 1;
    return 0;
}


static BOOL MTM_Init(void)
{
    mtmtrk=NULL;
    mh_mtm=NULL;

    if(!(mtmtrk=(MTMNOTE *)MyCalloc(64,sizeof(MTMNOTE)))) return 0;
    if(!(mh_mtm=(MTMHEADER *)MyCalloc(1,sizeof(MTMHEADER)))) return 0;

    return 1;
}


static void MTM_Cleanup(void)
{
    if(mtmtrk!=NULL) mod_free(mtmtrk);
    if(mh_mtm!=NULL) mod_free(mh_mtm);
}



static UBYTE *MTM_Convert(void)
{
    int t;
    UBYTE a,b,c,inst,note,eff,dat;

    UniReset();
    for(t=0;t<64;t++){

	a=mtmtrk[t].a;
	b=mtmtrk[t].b;
	c=mtmtrk[t].c;

	inst=((a&0x3)<<4)|(b>>4);
	note=a>>2;

	eff=b&0xf;
	dat=c;


	if(inst!=0){
	    UniInstrument((UBYTE)(inst - 1));
	}

	if(note!=0){
	    UniNote((UBYTE)(note + 24));
	}

	/* mtm bug bugfix: when the effect is volslide,
	   slide-up _always_ overrides slide-dn. */

	if(eff==0xa && dat&0xf0) dat&=0xf0;

	UniPTEffect(eff,dat);
	UniNewline();
    }
    return UniDup();
}


static BOOL MTM_Load(void)
{
    MTMSAMPLE s;
    INSTRUMENT *d;
    SAMPLE *q;

    int t,u;

    /* try to read module header */

    _mm_read_UBYTES(mh_mtm->id,3);
    mh_mtm->version		=_mm_read_UBYTE();
    _mm_read_str(mh_mtm->songname,20);
    mh_mtm->numtracks	=_mm_read_I_UWORD();
    mh_mtm->lastpattern	=_mm_read_UBYTE();
    mh_mtm->lastorder	=_mm_read_UBYTE();
    mh_mtm->commentsize	=_mm_read_I_UWORD();
    mh_mtm->numsamples	=_mm_read_UBYTE();
    mh_mtm->attribute	=_mm_read_UBYTE();
    mh_mtm->beatspertrack=_mm_read_UBYTE();
    mh_mtm->numchannels	=_mm_read_UBYTE();
    _mm_read_UBYTES(mh_mtm->panpos,32);

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    /* set module variables */

    of.initspeed=6;
    of.inittempo=125;
    of.modtype=strdup(MTM_Version);
    of.numchn=mh_mtm->numchannels;
    of.numtrk=mh_mtm->numtracks+1;                              /* get number of channels */
    of.songname=DupStr(mh_mtm->songname,20);    /* make a cstr of songname */
    of.numpos=mh_mtm->lastorder+1;              /* copy the songlength */
    of.numpat=mh_mtm->lastpattern+1;
    for(t=0;t<32;t++) of.panning[t]=mh_mtm->panpos[t]<<4;

    of.numins=mh_mtm->numsamples;
    if(!AllocInstruments()) return 0;

    d=of.instruments;

    for(t=0;t<of.numins;t++){

	d->numsmp=1;
	if(!AllocSamples(d)) return 0;
	q=d->samples;

	/* try to read sample info */

	_mm_read_str(s.samplename,22);
	s.length	=_mm_read_I_ULONG();
	s.reppos	=_mm_read_I_ULONG();
	s.repend	=_mm_read_I_ULONG();
	s.finetune	=_mm_read_UBYTE();
	s.volume	=_mm_read_UBYTE();
	s.attribute	=_mm_read_UBYTE();

	if(modpos > modsize){
	    gModPlayerErrorMessage=ERROR_LOADING_SAMPLEINFO;
	    return 0;
	}

	d->insname=DupStr(s.samplename,22);
	q->seekpos=0;
	q->c2spd=finetune[s.finetune];
	q->length=s.length;
	q->loopstart=s.reppos;
	q->loopend=s.repend;
	q->volume=s.volume;

	q->flags=0;

	if(s.repend-s.reppos>2) q->flags|=SF_LOOP;      /* <- 1.00 bugfix */

	if(s.attribute&1){

	    /* If the sample is 16-bits, convert the length
	       and replen byte-values into sample-values */

	    q->flags|=SF_16BITS;
	    q->length>>=1;
	    q->loopstart>>=1;
	    q->loopend>>=1;
	}

	d++;
    }

    _mm_read_UBYTES(of.positions,128);

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    if(!AllocTracks()) return 0;
    if(!AllocPatterns()) return 0;

    of.tracks[0]=MTM_Convert();             /* track 0 is empty */

    for(t=1;t<of.numtrk;t++){
	int s;

	for(s=0;s<64;s++){
	    mtmtrk[s].a=_mm_read_UBYTE();
	    mtmtrk[s].b=_mm_read_UBYTE();
	    mtmtrk[s].c=_mm_read_UBYTE();
	}

	if(modpos > modsize){
	    gModPlayerErrorMessage=ERROR_LOADING_TRACK;
	    return 0;
	}

	if(!(of.tracks[t]=MTM_Convert())) return 0;
    }

    for(t=0;t<of.numpat;t++){

	_mm_read_I_UWORDS(pat,32);

	for(u=0;u<of.numchn;u++){
	    of.patterns[((long)t*of.numchn)+u]=pat[u];
	}
    }

    /* read comment field */

    if(!ReadComment(mh_mtm->commentsize)) return 0;

    return 1;
}



LOADER load_mtm={
    NULL,
    "MultiTracker MOD",
    "",
    MTM_Init,
    MTM_Test,
    MTM_Load,
    MTM_Cleanup
};


#endif	// USE_MOD_API

