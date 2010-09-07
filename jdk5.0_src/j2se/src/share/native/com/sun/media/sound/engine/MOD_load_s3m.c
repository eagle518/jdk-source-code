/*
 * @(#)MOD_load_s3m.c	1.12 03/12/19
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
**	3/4/98		Fixed a pattern loading problem that skipped a portion of the play
**				back pattern
*/
/*****************************************************************************/
#include "X_API.h"

#if USE_MOD_API == TRUE
#include "MOD_mikmod.h"
/*

Name:
LOAD_S3M.C

Description:
Screamtracker (S3M) module loader

Portability:
All systems - all compilers (hopefully)

*/

/**************************************************************************
**************************************************************************/

typedef struct S3MNOTE{
    UBYTE note,ins,vol,cmd,inf;
} S3MNOTE;

typedef S3MNOTE S3MTRACK[64];


/* Raw S3M header struct: */

typedef struct S3MHEADER{
    char  songname[28];
    char  t1a;
    char  type;
    UBYTE unused1[2];
    UWORD ordnum;
    UWORD insnum;
    UWORD patnum;
    UWORD flags;
    UWORD tracker;
    UWORD fileformat;
    char  scrm[4];
    UBYTE mastervol;
    UBYTE initspeed;
    UBYTE inittempo;
    UBYTE mastermult;
    UBYTE ultraclick;
    UBYTE pantable;
    UBYTE unused2[8];
    UWORD special;
    UBYTE channels[32];
} S3MHEADER;


/* Raw S3M sampleinfo struct: */

typedef struct S3MSAMPLE{
    UBYTE type;
    char  filename[12];
    UBYTE memsegh;
    UWORD memsegl;
    ULONG length;
    ULONG loopbeg;
    ULONG loopend;
    UBYTE volume;
    UBYTE dsk;
    UBYTE pack;
    UBYTE flags;
    ULONG c2spd;
    UBYTE unused[12];
    char  sampname[28];
    char  scrs[4];
} S3MSAMPLE;


/**************************************************************************
**************************************************************************/



static S3MNOTE *s3mbuf;        /* pointer to a complete S3M pattern */
static UWORD *paraptr;         /* parapointer array (see S3M docs) */
static S3MHEADER *mh_s3m;
static UBYTE remap[32];


const char S3M_Version[]="Screamtracker 3.xx";



static BOOL S3M_Test(void)
{
    char id[4];
    _mm_fseek(0x2c,SEEK_SET);
    if(!_mm_read_str(id,4)) return 0;
    if(!memcmp(id,"SCRM",4)) return 1;
    return 0;
}

static BOOL S3M_Init(void)
{
    s3mbuf=NULL;
    paraptr=NULL;
    mh_s3m = NULL;

    s3mbuf = (S3MNOTE *)MyMalloc(16*64*sizeof(S3MNOTE));
    if(s3mbuf == 0L) return 0;

    mh_s3m = (S3MHEADER *)MyMalloc(sizeof(S3MHEADER));
    if(mh_s3m == 0L) return 0;

    return 1;	// ok
}

static void S3M_Cleanup(void)
{
    if(s3mbuf!=NULL) mod_free(s3mbuf);
    if(paraptr!=NULL) mod_free(paraptr);
    if(mh_s3m!=NULL) mod_free(mh_s3m);
}




static BOOL S3M_ReadPattern(void)
{
    int row=0,flag,ch;
    S3MNOTE *n;
    S3MNOTE dummy;

    /* clear pattern data */

    mod_memset(s3mbuf, (char)255, 16*64*sizeof(S3MNOTE));

    while(row<64){

	flag=_mm_read_UBYTE();

	//		if(flag==255)
	//		{
	//			gModPlayerErrorMessage=ERROR_LOADING_PATTERN;
	//			return 0;
	//		}

	if(flag)
	    {
		ch=flag&31;

		if(mh_s3m->channels[ch]<16)
		    {
			n=&s3mbuf[(64U*remap[ch])+row];
		    }
		else
		    {
			n=&dummy;
		    }

		if(flag&32)
		    {
			n->note=_mm_read_UBYTE();
			n->ins=_mm_read_UBYTE();
		    }

		if(flag&64)
		    {
			n->vol=_mm_read_UBYTE();
		    }

		if(flag&128)
		    {
			n->cmd=_mm_read_UBYTE();
			n->inf=_mm_read_UBYTE();
		    }
	    }
	else
	    {
		row++;
	    }
    }
    return 1;
}



static UBYTE *S3M_ConvertTrack(S3MNOTE *tr)
{
    int t;

    UBYTE note,ins,vol,cmd,inf,lo,hi;

    UniReset();
    for(t=0;t<64;t++){

	note=tr[t].note;
	ins=tr[t].ins;
	vol=tr[t].vol;
	cmd=tr[t].cmd;
	inf=tr[t].inf;
	lo=inf&0xf;
	hi=inf>>4;


	if(ins!=0 && ins!=255){
	    UniInstrument((UBYTE)(ins - 1));
	}

	if(note!=255){
	    if(note==254) UniPTEffect(0xc,0);                       /* <- note off command */
	    else UniNote((UBYTE)(((note>>4)*12)+(note&0xf)));        /* <- normal note */
	}

	if(vol<255){
	    UniPTEffect(0xc,vol);
	    //			UniWrite(UNI_S3MVOLUME);
	    //			UniWrite(vol);
	}

	if(cmd!=255){
	    switch(cmd){

	    case 1:                 /* Axx set speed to xx */
		UniWrite(UNI_S3MEFFECTA);
		UniWrite(inf);
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

	    case 0xf:               /* Oxx set sampleoffset xx00h */
		UniPTEffect(0x9,inf);
		break;

	    case 0x11:              /* Qxy Retrig (+volumeslide) */
		UniWrite(UNI_S3MEFFECTQ);
		UniWrite(inf);
		break;

	    case 0x12:              /* Rxy tremolo speed x, depth y */
		UniPTEffect(0x6,inf);
		break;

	    case 0x13:              /* Sxx special commands */
		switch(hi){

		case 0: /* S0x set filter */
		    UniPTEffect(0xe, (UBYTE)(0x00|lo));
		    break;

		case 1: /* S1x set glissando control */
		    UniPTEffect(0xe, (UBYTE)(0x30|lo));
		    break;

		case 2: /* S2x set finetune */
		    UniPTEffect(0xe, (UBYTE)(0x50|lo));
		    break;

		case 3: /* S3x set vibrato waveform */
		    UniPTEffect(0xe, (UBYTE)(0x40|lo));
		    break;

		case 4: /* S4x set tremolo waveform */
		    UniPTEffect(0xe, (UBYTE)(0x70|lo));
		    break;

		case 8: /* S8x set panning position */
		    UniPTEffect(0xe, (UBYTE)(0x80|lo));
		    break;

		case 0xb:       /* SBx pattern loop */
		    UniPTEffect(0xe, (UBYTE)(0x60|lo));
		    break;

		case 0xc:       /* SCx notecut */
		    UniPTEffect(0xe,(UBYTE)(0xC0|lo));
		    break;

		case 0xd:       /* SDx notedelay */
		    UniPTEffect(0xe, (UBYTE)(0xD0|lo));
		    break;

		case 0xe:       /* SDx patterndelay */
		    UniPTEffect(0xe, (UBYTE)(0xE0|lo));
		    break;
		}
		break;

	    case 0x14:      /* Txx tempo */
		if(inf>0x20){
		    UniWrite(UNI_S3MEFFECTT);
		    UniWrite(inf);
		}
		break;

	    case 0x18:      /* Xxx amiga command 8xx */
		UniPTEffect(0x8,inf);
		break;
	    }
	}

	UniNewline();
    }
    return UniDup();
}




static BOOL S3M_Load(void)
{
    int t,u,track=0;
    INSTRUMENT *d;
    SAMPLE *q;
    UBYTE isused[16];
    UBYTE pan[32];

    /* try to read module header */

    _mm_read_str(mh_s3m->songname,28);
    mh_s3m->t1a			=_mm_read_UBYTE();
    mh_s3m->type		=_mm_read_UBYTE();
    _mm_read_UBYTES(mh_s3m->unused1,2);
    mh_s3m->ordnum		=_mm_read_I_UWORD();
    mh_s3m->insnum		=_mm_read_I_UWORD();
    mh_s3m->patnum		=_mm_read_I_UWORD();
    mh_s3m->flags		=_mm_read_I_UWORD();
    mh_s3m->tracker		=_mm_read_I_UWORD();
    mh_s3m->fileformat	=_mm_read_I_UWORD();
    _mm_read_str(mh_s3m->scrm,4);

    mh_s3m->mastervol	=_mm_read_UBYTE();
    mh_s3m->initspeed	=_mm_read_UBYTE();
    mh_s3m->inittempo	=_mm_read_UBYTE();
    mh_s3m->mastermult	=_mm_read_UBYTE();
    mh_s3m->ultraclick	=_mm_read_UBYTE();
    mh_s3m->pantable	=_mm_read_UBYTE();
    _mm_read_UBYTES(mh_s3m->unused2,8);
    mh_s3m->special		=_mm_read_I_UWORD();
    _mm_read_UBYTES(mh_s3m->channels,32);

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    /* set module variables */

    of.modtype=strdup(S3M_Version);
    of.songname=DupStr(mh_s3m->songname,28);    /* make a cstr of songname */
    of.numpat=mh_s3m->patnum;
    of.numins=mh_s3m->insnum;
    of.initspeed=mh_s3m->initspeed;
    of.inittempo=mh_s3m->inittempo;

    /* count the number of channels used */

    of.numchn=0;

    /*      for(t=0;t<32;t++) printf("%2.2x ",mh_s3m->channels[t]);
     */
    for(t=0;t<32;t++) remap[t]=0;
    for(t=0;t<16;t++) isused[t]=0;

    /* set a flag for each channel (1 out of of 16) thats being used: */

    for(t=0;t<32;t++){
	if(mh_s3m->channels[t]<16){
	    isused[mh_s3m->channels[t]]=1;
	}
    }

    /* give each of them a different number */

    for(t=0;t<16;t++){
	if(isused[t]){
	    isused[t]=of.numchn;
	    of.numchn++;
	}
    }

    /* build the remap array */

    for(t=0;t<32;t++){
	if(mh_s3m->channels[t]<16){
	    remap[t]=isused[mh_s3m->channels[t]];
	}
    }

    /* set panning positions */

    for(t=0;t<32;t++){
	if(mh_s3m->channels[t]<16){
	    if(mh_s3m->channels[t]<8){
		of.panning[remap[t]]=0x30;
	    }
	    else{
		of.panning[remap[t]]=0xc0;
	    }
	}
    }

    of.numtrk=of.numpat*of.numchn;

    /*      printf("Uses %d channels\n",of.numchn);
     */
    /* read the order data */

    _mm_read_UBYTES(of.positions,mh_s3m->ordnum);

    of.numpos=0;
    for(t=0;t<mh_s3m->ordnum;t++){
	of.positions[of.numpos]=of.positions[t];
	if(of.positions[t]<254) of.numpos++;
    }

    if((paraptr=(UWORD *)MyMalloc((of.numins+of.numpat)*sizeof(UWORD)))==NULL) return 0;

    /* read the instrument+pattern parapointers */

    _mm_read_I_UWORDS(paraptr,of.numins+of.numpat);

    /*      printf("pantab %d\n",mh_s3m->pantable);
     */
    if(mh_s3m->pantable==252){

	/* read the panning table */

	_mm_read_UBYTES(pan,32);

	/* set panning positions according to panning table (new for st3.2) */

	for(t=0;t<32;t++){
	    if((pan[t]&0x20) && mh_s3m->channels[t]<16){
		of.panning[remap[t]]=(pan[t]&0xf)<<4;
	    }
	}
    }

    /* now is a good time to check if the header was too short :) */

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    if(!AllocInstruments()) return 0;

    d=of.instruments;

    for(t=0;t<of.numins;t++){
	S3MSAMPLE s;

	d->numsmp=1;
	if(!AllocSamples(d)) return 0;
	q=d->samples;

	/* seek to instrument position */

	_mm_fseek(((long)paraptr[t])<<4,SEEK_SET);

	/* and load sample info */

	s.type		=_mm_read_UBYTE();
	_mm_read_str(s.filename,12);
	s.memsegh	=_mm_read_UBYTE();
	s.memsegl	=_mm_read_I_UWORD();
	s.length	=_mm_read_I_ULONG();
	s.loopbeg	=_mm_read_I_ULONG();
	s.loopend	=_mm_read_I_ULONG();
	s.volume	=_mm_read_UBYTE();
	s.dsk 		=_mm_read_UBYTE();
	s.pack		=_mm_read_UBYTE();
	s.flags		=_mm_read_UBYTE();
	s.c2spd		=_mm_read_I_ULONG();
	_mm_read_UBYTES(s.unused,12);
	_mm_read_str(s.sampname,28);
	_mm_read_str(s.scrs,4);

	if(modpos > modsize){
	    gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	    return 0;
	}

	d->insname=DupStr(s.sampname,28);
	q->c2spd= (unsigned short)s.c2spd;
	q->length=s.length;
	q->loopstart=s.loopbeg;
	q->loopend=s.loopend;
	q->volume=s.volume;
	q->seekpos=(((long)s.memsegh)<<16|s.memsegl)<<4;

	q->flags=0;

	if(s.flags&1) q->flags|=SF_LOOP;
	if(s.flags&4) q->flags|=SF_16BITS;
	if(mh_s3m->fileformat==1) q->flags|=SF_SIGNED;

	/* DON'T load sample if it doesn't have the SCRS tag */

	if(memcmp(s.scrs,"SCRS",4)!=0) q->length=0;

	/*              printf("%s\n",d->insname);
	 */
	d++;
    }

    if(!AllocTracks()) 
	{
	    gModPlayerErrorMessage=ERROR_ALLOC_STRUCT;
	    return 0;
	}
    if(!AllocPatterns()) 
	{
	    gModPlayerErrorMessage=ERROR_ALLOC_STRUCT;
	    return 0;
	}

    for(t=0;t<of.numpat;t++)
	{
	    /* seek to pattern position ( + 2 skip pattern length ) */
		
	    _mm_fseek((((long)paraptr[of.numins+t])<<4)+2,SEEK_SET);

	    if(!S3M_ReadPattern())
		{
		    gModPlayerErrorMessage = ERROR_LOADING_PATTERN;
		    return 0;
		}
	    for(u=0;u<of.numchn;u++)
		{
		    if(!(of.tracks[track++]=S3M_ConvertTrack(&s3mbuf[u*64]))) 
			{
			    gModPlayerErrorMessage = ERROR_LOADING_TRACK;
			    return 0;
			}
		}
	}

    return 1;
}


LOADER load_s3m={
    NULL,
    "Scream Tracker 3.x MOD",
    "",
    S3M_Init,
    S3M_Test,
    S3M_Load,
    S3M_Cleanup
};

#endif	// USE_MOD_API

