/*
 * @(#)MOD_load_stm.c	1.13 03/12/19
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
LOAD_STM.C

Description:
ScreamTracker 2 (STM) module Loader - Version 1.oOo Release 2 
A Coding Nightmare by Rao and Air Richter of HaRDCoDE
You can now play all of those wonderful old C.C. Catch STM's!

Portability:
All systems - all compilers (hopefully)

*/

typedef struct STMNOTE{
    UBYTE note,insvol,volcmd,cmdinf;
} STMNOTE;


/* Raw STM sampleinfo struct: */

typedef struct STMSAMPLE{
    char  filename[12]; /* Can't have long comments - just filename comments :) */
    char  unused;       /* 0x00 */
    UBYTE instdisk;     /* Instrument disk */
    UWORD reserved;     /* ISA in memory when in ST 2 */
    UWORD length;       /* Sample length */
    UWORD loopbeg;      /* Loop start point */
    UWORD loopend;      /* Loop end point */
    UBYTE volume;       /* Volume */
    UBYTE reserved2;    /* More reserved stuff */
    UWORD c2spd;        /* Good old c2spd */
    UBYTE reserved3[4]; /* Yet more of PSi's reserved stuff */
    UWORD isa;          /* Internal Segment Address -> */
    /*    contrary to the tech specs, this is NOT actually */
    /*    written to the stm file. */
} STMSAMPLE;

/* Raw STM header struct: */

typedef struct STMHEADER{
    char songname[20];
    char trackername[8];   /* !SCREAM! for ST 2.xx */
    char unused;           /* 0x1A */
    char filetype;         /* 1=song, 2=module (only 2 is supported, of course) :) */
    char ver_major;        /* Like 2 */
    char ver_minor;        /* "ditto" */
    UBYTE inittempo;       /* initspeed= stm inittempo>>4 */
    UBYTE  numpat;         /* number of patterns */
    UBYTE   globalvol;     /* <- WoW! a RiGHT TRiANGLE =8*) */
    UBYTE    reserved[13]; /* More of PSi's internal stuff */
    STMSAMPLE sample[31];  /* STM sample data */
    UBYTE patorder[128];   /* Docs say 64 - actually 128 */
} STMHEADER;


static STMNOTE *stmbuf;
static STMHEADER *mh_stm;

const char STM_Version[]="ScreamTracker 2.x";



static BOOL STM_Test(void)
{
    char str[9],filetype;

    _mm_fseek(21,SEEK_SET);
    _mm_read_str(str,9);
    _mm_read_str(&filetype,1);
    if(!memcmp(str,"!SCREAM!",8) || (filetype!=2)) /* STM Module = filetype 2 */
	return 0;
    return 1;
}



static BOOL STM_Init(void)
{
    stmbuf=NULL;
    if(!(mh_stm=(STMHEADER *)MyCalloc(1,sizeof(STMHEADER)))) return 0;
    return 1;
}

static void STM_Cleanup(void)
{
    if(mh_stm!=NULL) mod_free(mh_stm);
    if(stmbuf!=NULL) mod_free(stmbuf);
}



static void STM_ConvertNote(STMNOTE *n)
{
    UBYTE note,ins,vol,cmd,inf;

    /* extract the various information from the 4 bytes that
       make up a single note */

    note=n->note;
    ins=n->insvol>>3;
    vol=(n->insvol&7)+(n->volcmd>>1);
    cmd=n->volcmd&15;
    inf=n->cmdinf;

    if(ins!=0 && ins<32){
	UniInstrument((char)(ins - 1));
    }

    /* special values of [SBYTE0] are handled here -> */
    /* we have no idea if these strange values will ever be encountered */
    /* but it appears as though stms sound correct. */
    if(note==254 || note==252) UniPTEffect(0xc,0); /* <- note off command (???) */
    else
	/* if note < 251, then all three bytes are stored in the file */
	if(note<251)
	    {
		UniNote((char)((((note>>4)+2)*12)+(note&0xf)));      /* <- normal note and up the octave by two */
	    }
    if(vol<65){
	UniPTEffect(0xc,vol);
    }

    if(cmd!=255){
	switch(cmd){

	case 1:                 /* Axx set speed to xx and add 0x1c to fix StoOoPiD STM 2.x */
	    UniPTEffect(0xf, (char)(inf>>4));
	    break;

	case 2:                 /* Bxx position jump */
	    UniPTEffect(0xb,inf);
	    break;

	case 3:                 /* Cxx patternbreak to row xx */
	    UniPTEffect(0xd,inf);
	    break;

	case 4:                 /* Dxy volumeslide */
	    UniWrite(UNI_S3MEFFECTD);
	    UniWrite(inf);
	    break;

	case 5:                 /* Exy toneslide down */
	    UniWrite(UNI_S3MEFFECTE);
	    UniWrite(inf);
	    break;

	case 6:                 /* Fxy toneslide up */
	    UniWrite(UNI_S3MEFFECTF);
	    UniWrite(inf);
	    break;

	case 7:                 /* Gxx Tone portamento,speed xx */
	    UniPTEffect(0x3,inf);
	    break;

	case 8:                 /* Hxy vibrato */
	    UniPTEffect(0x4,inf);
	    break;

	case 9:                 /* Ixy tremor, ontime x, offtime y */
	    UniWrite(UNI_S3MEFFECTI);
	    UniWrite(inf);
	    break;

	case 0xa:               /* Jxy arpeggio */
	    UniPTEffect(0x0,inf);
	    break;

	case 0xb:               /* Kxy Dual command H00 & Dxy */
	    UniPTEffect(0x4,0);
	    UniWrite(UNI_S3MEFFECTD);
	    UniWrite(inf);
	    break;

	case 0xc:               /* Lxy Dual command G00 & Dxy */
	    UniPTEffect(0x3,0);
	    UniWrite(UNI_S3MEFFECTD);
	    UniWrite(inf);
	    break;

	    /* Support all these above, since ST2 can LOAD these values */
	    /* but can actually only play up to J - and J is only */
	    /* half-way implemented in ST2 */

	case 0x18:      /* Xxx amiga command 8xx - What the hell, support panning. :) */
	    UniPTEffect(0x8,inf);
	    break;
	}
    }

}


static UBYTE *STM_ConvertTrack(STMNOTE *n)
{
    int t;

    UniReset();
    for(t=0;t<64;t++)
	{       STM_ConvertNote(n);
	UniNewline();
	n+=of.numchn;
	}
    return UniDup();
}




static BOOL STM_LoadPatterns(void)
{
    int t,s,tracks=0;

    if(!AllocPatterns()) return 0;
    if(!AllocTracks()) return 0;

    /* Allocate temporary buffer for loading
       and converting the patterns */

    if(!(stmbuf=(STMNOTE *)MyCalloc(64U*of.numchn,sizeof(STMNOTE)))) return 0;

    for(t=0;t<of.numpat;t++){

	for(s=0;s<(64L*of.numchn);s++){
	    stmbuf[s].note=_mm_read_UBYTE();
	    stmbuf[s].insvol=_mm_read_UBYTE();
	    stmbuf[s].volcmd=_mm_read_UBYTE();
	    stmbuf[s].cmdinf=_mm_read_UBYTE();
	}

	if(modpos > modsize){
	    gModPlayerErrorMessage=ERROR_LOADING_PATTERN;
	    return 0;
	}

	for(s=0;s<of.numchn;s++){
	    if(!(of.tracks[tracks++]=STM_ConvertTrack(stmbuf+s))) return 0;
	}
    }

    return 1;
}



static BOOL STM_Load(void)
{
    int t;
    ULONG MikMod_ISA; /* We MUST generate our own ISA - NOT stored in the stm */
    INSTRUMENT *d;
    SAMPLE *q;

    /* try to read stm header */

    _mm_read_str(mh_stm->songname,20);
    _mm_read_str(mh_stm->trackername,8);
    mh_stm->unused		=_mm_read_UBYTE();
    mh_stm->filetype	=_mm_read_UBYTE();
    mh_stm->ver_major	=_mm_read_UBYTE();
    mh_stm->ver_minor	=_mm_read_UBYTE();
    mh_stm->inittempo	=_mm_read_UBYTE();
    mh_stm->numpat		=_mm_read_UBYTE();
    mh_stm->globalvol	=_mm_read_UBYTE();
    _mm_read_UBYTES(mh_stm->reserved,13);

    for(t=0;t<31;t++){
	STMSAMPLE *s=&mh_stm->sample[t];  /* STM sample data */
	_mm_read_str(s->filename,12);
	s->unused	=_mm_read_UBYTE();
	s->instdisk	=_mm_read_UBYTE();
	s->reserved	=_mm_read_I_UWORD();
	s->length	=_mm_read_I_UWORD();
	s->loopbeg	=_mm_read_I_UWORD();
	s->loopend	=_mm_read_I_UWORD();
	s->volume	=_mm_read_UBYTE();
	s->reserved2=_mm_read_UBYTE();
	s->c2spd	=_mm_read_I_UWORD();
	_mm_read_UBYTES(s->reserved3,4);
	s->isa		=_mm_read_I_UWORD();
    }
    _mm_read_UBYTES(mh_stm->patorder,128);

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    /* set module variables */

    of.modtype=strdup(STM_Version);
    of.songname=DupStr(mh_stm->songname,20); /* make a cstr of songname */

    of.numpat=mh_stm->numpat;

    of.initspeed=6; /* Always this */

    /* STM 2.x tempo has always been messed up... The default of 96 */
    /* is actually 124, so we add 1ch to the initial value of 60h */

    /* MikMak: No it's not.. STM tempo is UNI speed << 4 */

    of.inittempo=125;               /* mh_stm->inittempo+0x1c; */
    of.initspeed=mh_stm->inittempo>>4;
    of.numchn=4; /* get number of channels */

    t=0;
    while(mh_stm->patorder[t]!=99){ /* 99 terminates the patorder list */
	of.positions[t]=mh_stm->patorder[t];
	t++;
    }
    of.numpos=--t;
    of.numtrk=of.numpat*of.numchn;

    /* Finally, init the sampleinfo structures */

    of.numins=31; /* always this */

    if(!AllocInstruments()) return 0;
    if(!STM_LoadPatterns()) return 0;

    d=of.instruments;

    MikMod_ISA=_mm_ftell();
    MikMod_ISA=(MikMod_ISA+15)&0xfffffff0;

    for(t=0;t<of.numins;t++){

	d->numsmp=1;
	if(!AllocSamples(d)) return 0;
	q=d->samples;

	/* load sample info */

	d->insname=DupStr(mh_stm->sample[t].filename,12);
	q->c2spd=mh_stm->sample[t].c2spd;
	q->volume=mh_stm->sample[t].volume;
	q->length=mh_stm->sample[t].length;
	if (!mh_stm->sample[t].volume || q->length==1 ) q->length = 0; /* if vol = 0 or length = 1, then no sample */
	q->loopstart=mh_stm->sample[t].loopbeg;
	q->loopend=mh_stm->sample[t].loopend;
	q->seekpos=MikMod_ISA;

	MikMod_ISA+=q->length;

	MikMod_ISA=(MikMod_ISA+15)&0xfffffff0;

	/* Once again, contrary to the STM specs, all the sample data is */
	/* actually SIGNED! Sheesh */

	q->flags=SF_SIGNED;

	if(mh_stm->sample[t].loopend>0 && mh_stm->sample[t].loopend!=0xffff) q->flags|=SF_LOOP;

	/* fix replen if repend>length */

	if(q->loopend>q->length) q->loopend=q->length;

	d++;
    }

    return 1;
}


LOADER load_stm={
    NULL,
    "Scream Tracker 2.x MOD",
    "",
    STM_Init,
    STM_Test,
    STM_Load,
    STM_Cleanup
};

#endif	// USE_MOD_API

