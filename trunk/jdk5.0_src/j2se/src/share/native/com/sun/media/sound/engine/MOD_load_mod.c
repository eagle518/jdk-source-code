/*
 * @(#)MOD_load_mod.c	1.12 03/12/19
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
LOAD_MOD.C

Description:
Generic MOD loader

Portability:
All systems - all compilers (hopefully)

*/

/*************************************************************************
*************************************************************************/


typedef struct MSAMPINFO{       /* sample header as it appears in a module */
    char  samplename[22];
    UWORD length;
    UBYTE finetune;
    UBYTE volume;
    UWORD reppos;
    UWORD replen;
} MSAMPINFO;


typedef struct MODULEHEADER{    /* verbatim module header */
    char       songname[20];                /* the songname.. */
    MSAMPINFO  samples[31];                         /* all sampleinfo */
    UBYTE      songlength;                          /* number of patterns used */
    UBYTE      magic1;                                      /* should be 127 */
    UBYTE      positions[128];                      /* which pattern to play at pos */
    UBYTE      magic2[4];                           /* string "M.K." or "FLT4" or "FLT8" */
} MODULEHEADER;

#define MODULEHEADERSIZE 1084


typedef struct MODTYPE{                         /* struct to identify type of module */
    char    id[5];
    UBYTE   channels;
    const char    *name;
} MODTYPE;


typedef struct MODNOTE{
    UBYTE a,b,c,d;
} MODNOTE;


/*************************************************************************
*************************************************************************/


const char protracker[]="ProTracker";
const char startracker[]="StarTracker";
const char fasttracker[]="FastTracker";
const char ins15tracker[]="ProTracker (15-instrument)";
const char oktalyzer[]="Oktalyzer";
const char taketracker[]="TakeTracker";


const MODTYPE modtypes[]={
    {"M.K.",4,protracker},    /* protracker 4 channel */
    {"M!K!",4,protracker},    /* protracker 4 channel */
    {"FLT4",4,startracker},   /* startracker 4 channel */
    {"4CHN",4,fasttracker},   /* fasttracker 4 channel */
    {"6CHN",6,fasttracker},   /* fasttracker 6 channel */
    {"8CHN",8,fasttracker},   /* fasttracker 8 channel */
    {"CD81",8,oktalyzer},     /* atari oktalyzer 8 channel */
    {"OKTA",8,oktalyzer},     /* atari oktalyzer 8 channel */
    {"16CN",16,taketracker},  /* taketracker 16 channel */
    {"32CN",32,taketracker},  /* taketracker 32 channel */
    {"    ",4,ins15tracker}   /* 15-instrument 4 channel */
};

static MODULEHEADER *mh_mod;        /* raw as-is module header */
static MODNOTE *patbuf;


static BOOL MOD_Test(void)
{
    int t;

    char id[4];

    _mm_fseek(MODULEHEADERSIZE-4,SEEK_SET);
    if(!_mm_read_str(id,4)) return 0;

    /* find out which ID string */

    for(t=0;t<10;t++){
	if(!memcmp(id,modtypes[t].id,4)) return 1;
    }

    return 0;
}


static BOOL MOD_Init(void)
{
    patbuf=NULL;
    if(!(mh_mod=(MODULEHEADER *)MyCalloc(1,sizeof(MODULEHEADER)))) return 0;
    return 1;
}


static void MOD_Cleanup(void)
{
    if(mh_mod!=NULL) mod_free(mh_mod);
    if(patbuf!=NULL) mod_free(patbuf);
}


/*

Old (amiga) noteinfo:

 _____byte 1_____   byte2_    _____byte 3_____   byte4_
/                \ /      \  /                \ /      \
0000          0000-00000000  0000          0000-00000000

Upper four    12 bits for    Lower four    Effect command.
bits of sam-  note period.   bits of sam-
ple number.                  ple number.


*/


const UWORD npertab[60]={

    /* -> Tuning 0 */

    1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,
    107,101,95,90,85,80,75,71,67,63,60,56
};


static void ConvertNote(MODNOTE *n)
{
    UBYTE instrument,effect,effdat,note;
    UWORD period;

    /* extract the various information from the 4 bytes that
       make up a single note */

    instrument=(n->a&0x10)|(n->c>>4);
    period=(((UWORD)n->a&0xf)<<8)+n->b;
    effect=n->c&0xf;
    effdat=n->d;

    /* Convert the period to a note number */

    note=0;
    if(period!=0){
	for(note=0;note<60;note++){
	    if(period>=npertab[note]) break;
	}
	note++;
	if(note==61) note=0;
    }

    if(instrument!=0){
	UniInstrument((UBYTE)(instrument - 1));
    }

    if(note!=0){
	UniNote((UBYTE)(note + 23));
    }

    UniPTEffect(effect,effdat);
}


static UBYTE *ConvertTrack(MODNOTE *n)
{
    int t;

    UniReset();
    for(t=0;t<64;t++){
	ConvertNote(n);
	UniNewline();
	n+=of.numchn;
    }
    return UniDup();
}


static BOOL ML_LoadPatterns(void)
/*
	Loads all patterns of a modfile and converts them into the
	3 byte format.
*/
{
    int t,s,tracks=0;

    if(!AllocPatterns()) return 0;
    if(!AllocTracks()) return 0;

    /* Allocate temporary buffer for loading
       and converting the patterns */

    if(!(patbuf=(MODNOTE *)MyCalloc(64U*of.numchn,sizeof(MODNOTE)))) return 0;

    for(t=0;t<of.numpat;t++){

	/* Load the pattern into the temp buffer
	   and convert it */

	for(s=0;s<(64L * of.numchn);s++){
	    patbuf[s].a=_mm_read_UBYTE();
	    patbuf[s].b=_mm_read_UBYTE();
	    patbuf[s].c=_mm_read_UBYTE();
	    patbuf[s].d=_mm_read_UBYTE();
	}

	for(s=0;s<of.numchn;s++){
	    if(!(of.tracks[tracks++]=ConvertTrack(patbuf+s))) return 0;
	}
    }

    return 1;
}


static BOOL MOD_Load(void)
{
    int t,modtype;
    INSTRUMENT *d;          /* new sampleinfo structure */
    SAMPLE *q;
    MSAMPINFO *s;           /* old module sampleinfo */

    /* try to read module header */

    _mm_read_str((char *)mh_mod->songname,20);

    for(t=0;t<31;t++){
	s=&mh_mod->samples[t];
	_mm_read_str(s->samplename,22);
	s->length	=_mm_read_M_UWORD();
	s->finetune	=_mm_read_UBYTE();
	s->volume	=_mm_read_UBYTE();
	s->reppos	=_mm_read_M_UWORD();
	s->replen	=_mm_read_M_UWORD();
    }

    mh_mod->songlength	=_mm_read_UBYTE();
    mh_mod->magic1		=_mm_read_UBYTE();

    _mm_read_UBYTES(mh_mod->positions,128);
    _mm_read_UBYTES(mh_mod->magic2,4);

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    /* find out which ID string */

    for(modtype=0;modtype<10;modtype++){
	if(!memcmp(mh_mod->magic2,modtypes[modtype].id,4)) break;
    }

    if(modtype==10){

	/* unknown modtype */
	gModPlayerErrorMessage=ERROR_NOT_A_MODULE;
	return 0;
    }

    /* set module variables */

    of.initspeed=6;
    of.inittempo=125;
    of.numchn=modtypes[modtype].channels;      /* get number of channels */
    of.modtype=strdup(modtypes[modtype].name);      /* get ascii type of mod */
    of.songname=DupStr(mh_mod->songname,20);            /* make a cstr of songname */
    of.numpos=mh_mod->songlength;               /* copy the songlength */
    memcpy(of.positions,mh_mod->positions,128);         /* copy the position array */

    /* Count the number of patterns */

    of.numpat=0;

    for(t=0;t<128;t++){             /* <-- BUGFIX... have to check ALL positions */
	if(of.positions[t] > of.numpat){
	    of.numpat=of.positions[t];
	}
    }
    of.numpat++;
    of.numtrk=of.numpat*of.numchn;

    /* Finally, init the sampleinfo structures */

    of.numins=31;

    if(!AllocInstruments()) return 0;

    s=mh_mod->samples;   /* init source pointer */
    d=of.instruments;  /* init dest pointer */

    for(t=0;t<of.numins;t++){

	d->numsmp=1;
	if(!AllocSamples(d)) return 0;

	q=d->samples;

	/* convert the samplename */

	d->insname=DupStr(s->samplename,22);

	/* init the sampleinfo variables and
	   convert the size pointers to longword format */

	q->c2spd=finetune[s->finetune&0xf];
	q->volume=s->volume;
	q->loopstart=(ULONG)s->reppos<<1;
	q->loopend=q->loopstart+((ULONG)s->replen<<1);
	q->length=(ULONG)s->length<<1;
	q->seekpos=0;

	q->flags=SF_SIGNED;
	if(s->replen>1) q->flags|=SF_LOOP;

	/* fix replen if repend>length */

	if(q->loopend>q->length) q->loopend=q->length;

	s++;    /* point to next source sampleinfo */
	d++;    /* point to next destiny sampleinfo */
    }

    if(!ML_LoadPatterns()) return 0;
    return 1;
}



LOADER load_mod={
    NULL,
    "ProTracker MOD",
    "",
    MOD_Init,
    MOD_Test,
    MOD_Load,
    MOD_Cleanup
};

#endif	// USE_MOD_API
