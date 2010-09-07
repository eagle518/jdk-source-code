/*
 * @(#)MOD_load_ult.c	1.12 03/12/19
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
LOAD_ULT.C

Description:
Ultratracker (ULT) module loader

Portability:
All systems - all compilers (hopefully)

*/

#define ULTS_16BITS     4
#define ULTS_LOOP       8
#define ULTS_REVERSE    16


/* Raw ULT header struct: */

typedef struct ULTHEADER{
    char  id[15];
    char  songtitle[32];
    UBYTE reserved;
} ULTHEADER;


/* Raw ULT sampleinfo struct: */

typedef struct ULTSAMPLE{
    char  samplename[32];
    char  dosname[12];
    SLONG  loopstart;
    SLONG  loopend;
    SLONG  sizestart;
    SLONG  sizeend;
    UBYTE volume;
    UBYTE flags;
    SWORD  finetune;
} ULTSAMPLE;


typedef struct ULTEVENT{
    UBYTE note,sample,eff,dat1,dat2;
} ULTEVENT;


const char *ULT_Version[]={
    "Ultra Tracker V1.3",
    "Ultra Tracker V1.4",
    "Ultra Tracker V1.5",
    "Ultra Tracker V1.6"
};


ULTEVENT ev;


static BOOL ULT_Test(void)
{
    char id[15];

    if(!_mm_read_str((char *)&id,15)) return 0;
    return(!strncmp(id,"MAS_UTrack_V00",14));
}


static BOOL ULT_Init(void)
{
    return 1;
}


static void ULT_Cleanup(void)
{
}


static int ReadUltEvent(ULTEVENT *event)
{
    UBYTE flag,rep=1;

    flag=_mm_read_UBYTE();

    if(flag==0xfc){
	_mm_read_str((char *)&rep,1);
	event->note	=_mm_read_UBYTE();
    }
    else{
	event->note=flag;
    }

    event->sample	=_mm_read_UBYTE();
    event->eff		=_mm_read_UBYTE();
    event->dat1		=_mm_read_UBYTE();
    event->dat2		=_mm_read_UBYTE();

    return rep;
}




static BOOL ULT_Load(void)
{
    int t,u,tracks=0;
    INSTRUMENT *d;
    SAMPLE *q;
    ULTSAMPLE s;
    ULTHEADER mh;
    UBYTE nos,noc,nop;

    /* try to read module header */

    _mm_read_str(mh.id,15);
    _mm_read_str(mh.songtitle,32);
    mh.reserved=_mm_read_UBYTE();

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    if(mh.id[14]<'1' || mh.id[14]>'4'){
	//printf("This version is not yet supported\n");
	return 0;
    }

    of.modtype=strdup(ULT_Version[mh.id[14]-'1']);
    of.initspeed=6;
    of.inittempo=125;

    /* read songtext */

    if(!ReadComment((UWORD)(mh.reserved * 32))) return 0;

    nos=_mm_read_UBYTE();

    if(modpos > modsize){
	gModPlayerErrorMessage=ERROR_LOADING_HEADER;
	return 0;
    }

    of.songname=DupStr(mh.songtitle,32);
    of.numins=nos;

    if(!AllocInstruments()) return 0;

    d=of.instruments;

    for(t=0;t<nos;t++){

	d->numsmp=1;
	if(!AllocSamples(d)) return 0;
	q=d->samples;

	/* try to read sample info */

	_mm_read_str(s.samplename,32);
	_mm_read_str(s.dosname,12);
	s.loopstart	=_mm_read_I_ULONG();
	s.loopend	=_mm_read_I_ULONG();
	s.sizestart	=_mm_read_I_ULONG();
	s.sizeend	=_mm_read_I_ULONG();
	s.volume	=_mm_read_UBYTE();
	s.flags		=_mm_read_UBYTE();
	s.finetune	=_mm_read_I_SWORD();

	if(modpos > modsize){
	    gModPlayerErrorMessage=ERROR_LOADING_SAMPLEINFO;
	    return 0;
	}

	d->insname=DupStr(s.samplename,32);

	q->seekpos=0;

	q->c2spd=8363;

	if(mh.id[14]>='4'){
	    _mm_read_I_UWORD();	/* read 1.6 extra info(??) word */
	    q->c2spd=s.finetune;
	}

	q->length=s.sizeend-s.sizestart;
	q->volume=s.volume>>2;
	q->loopstart=s.loopstart;
	q->loopend=s.loopend;

	q->flags=SF_SIGNED;

	if(s.flags&ULTS_LOOP){
	    q->flags|=SF_LOOP;
	}

	if(s.flags&ULTS_16BITS){
	    q->flags|=SF_16BITS;
	    q->loopstart>>=1;
	    q->loopend>>=1;
	}

	/*      printf("Sample %d %s length %ld\n",t,d->samplename,d->length); */
	d++;
    }

    _mm_read_UBYTES(of.positions,256);

    for(t=0;t<256;t++){
	if(of.positions[t]==255) break;
    }
    of.numpos=t;

    noc=_mm_read_UBYTE();
    nop=_mm_read_UBYTE();

    of.numchn=noc+1;
    of.numpat=nop+1;
    of.numtrk=of.numchn*of.numpat;

    if(!AllocTracks()) return 0;
    if(!AllocPatterns()) return 0;

    for(u=0;u<of.numchn;u++){
	for(t=0;t<of.numpat;t++){
	    of.patterns[(t*of.numchn)+u]=tracks++;
	}
    }

    /* read pan position table for v1.5 and higher */

    if(mh.id[14]>='3'){
	for(t=0;t<of.numchn;t++) of.panning[t]=_mm_read_UBYTE()<<4;
    }


    for(t=0;t<of.numtrk;t++){
	int rep,s,done;

	UniReset();
	done=0;

	while(done<64){

	    rep=ReadUltEvent(&ev);

	    if(modpos > modsize){
		gModPlayerErrorMessage=ERROR_LOADING_TRACK;
		return 0;
	    }

	    /*                      printf("rep %d: n %d i %d e %x d1 %d d2 %d \n",rep,ev.note,ev.sample,ev.eff,ev.dat1,ev.dat2); */


	    for(s=0;s<rep;s++){
		UBYTE eff;


		if(ev.sample){
		    UniInstrument((UBYTE)(ev.sample - 1));
		}

		if(ev.note){
		    UniNote((UBYTE)(ev.note + 23));
		}

		eff=ev.eff>>4;


				/*
				  ULT panning effect fixed by Alexander Kerkhove :
				*/


		if(eff==0xc) UniPTEffect(eff, (UBYTE)(ev.dat2 >> 2));
		else if(eff==0xb) UniPTEffect(8, (UBYTE)(ev.dat2 * 0xf));
		else UniPTEffect(eff,ev.dat2);

		eff=ev.eff&0xf;

		if(eff==0xc) UniPTEffect(eff, (UBYTE)(ev.dat1 >> 2));
		else if(eff==0xb) UniPTEffect(8, (UBYTE)(ev.dat1 * 0xf));
		else UniPTEffect(eff,ev.dat1);

		UniNewline();
		done++;
	    }
	}
	/*              printf("----------------"); */

	if(!(of.tracks[t]=UniDup())) return 0;
    }

    /*      printf("%d channels %d patterns\n",of.numchn,of.numpat); */
    /*      printf("Song %32.32s: There's %d samples\n",mh.songtitle,nos); */
    return 1;
}



LOADER load_ult={
    NULL,
    "UltraTracker MOD",
    "",
    ULT_Init,
    ULT_Test,
    ULT_Load,
    ULT_Cleanup
};

#endif	// USE_MOD_API

