/*
 * @(#)MOD_virtch.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**
**	History	-
**	12/1/96		Created
**	4/17/97		Changed min to MIN_VALUE
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
VIRTCH.C

Description:
All-c sample mixing routines, using a 32 bits mixing buffer

Portability:
All systems - all compilers

*/

#define FRACBITS 11
#define FRACMASK ((1L<<FRACBITS)-1)

#define TICKLSIZE 3600
#define TICKWSIZE (TICKLSIZE*2)
#define TICKBSIZE (TICKWSIZE*2)
static SLONG*	VC_TICKBUF;	//static SLONG VC_TICKBUF[TICKLSIZE];

#ifndef MIN_VALUE
#define MIN_VALUE(a, b) (((a)<(b)) ? (a) : (b))
#endif

typedef struct{
    UBYTE kick;                     /* =1 -> sample has to be restarted */
    UBYTE active;                   /* =1 -> sample is playing */
    UWORD flags;                    /* 16/8 bits looping/one-shot */
    SWORD handle;                  /* identifies the sample */
    ULONG start;                    /* start index */
    ULONG size;                     /* samplesize */
    ULONG reppos;                   /* loop start */
    ULONG repend;                   /* loop end */
    ULONG frq;                      /* current frequency */
    UBYTE vol;                      /* current volume */
    UBYTE pan;						/* current panning position */
    SLONG current;              	/* current index in the sample */
    SLONG increment;             	/* fixed-point increment value */
#ifdef __WATCOMC__
    UWORD lvolsel;					/* left volume table selector */
    UWORD rvolsel;					/* right volume table selector */
#else
    SLONG lvolmul;					/* left volume multiply */
    SLONG rvolmul;					/* right volume multiply */
#endif
} VINFO;


static VINFO* vinf;	//static VINFO vinf[32];
static VINFO *vnf;

static UWORD samplesthatfit;
static SLONG idxsize, idxlpos,idxlend,maxvol;

static int ampshift;

static SLONG lvolmul, rvolmul;


static void VC_InitializeMemory(void)
{
    if (vinf != NULL)
	return;	// allow us to be called repeatedly
		
    vinf = (VINFO*)mod_malloc(32*sizeof(VINFO));
    VC_TICKBUF = (SLONG*)mod_malloc(TICKLSIZE*sizeof(SLONG));
}

static void VC_FinalizeMemory(void)
{
    mod_free(vinf); vinf = NULL;
    mod_free(VC_TICKBUF); VC_TICKBUF = NULL;
}

static void VC_Sample32To16Copy(SLONG *srce, SWORD *dest, ULONG count, UBYTE shift)
{
    // [DEMOS]-> shift renamed to _shift to avoid variable collision
    SLONG c;
    int _shift= shift;

    while (count--) {
	c=*srce >> _shift;
	if (c>32767) c=32767;
	else if (c<-32768) c=-32768;
	*dest++ = (SWORD)c;
	srce++;
    }
}


static SLONG fraction2long(ULONG dividend, UWORD divisor)
/*
	Converts the fraction 'dividend/divisor' into a fixed point longword.
*/
{
    ULONG whole, part;

    whole=dividend/divisor;
    part=((dividend%divisor)<<FRACBITS)/divisor;

    return((whole<<FRACBITS)|part);
}


static UWORD samples2bytes(UWORD samples)
{
    if (md_mode & DMODE_16BITS) samples<<=1;
    if (md_mode & DMODE_STEREO) samples<<=1;
    return samples;
}


static UWORD bytes2samples(UWORD bytes)
{
    if (md_mode & DMODE_16BITS) bytes>>=1;
    if (md_mode & DMODE_STEREO) bytes>>=1;
    return bytes;
}


/**************************************************
***************************************************
***************************************************
**************************************************/


static SBYTE *Samples[MAXSAMPLEHANDLES];


BOOL LargeRead(SBYTE *buffer, ULONG size);
BOOL LargeRead(SBYTE *buffer, ULONG size)
{
    ULONG todo;

    while (size) {
	/* how many bytes to load (in chunks of 8000) ? */

	todo=(size>8000)?8000:size;

	/* read data */

	SL_Load(buffer, todo);
	/* and update pointers.. */

	size-=todo;
	buffer+=todo;
    }
    return 1;
}



SWORD VC_SampleLoad(ULONG length, ULONG loopstart, ULONG loopend, UWORD flags)
{
    int handle;
    ULONG t;

    SL_Init(flags, (UWORD)((flags | SF_SIGNED) & ~SF_16BITS));

    /* Find empty slot to put sample address in */

    for (handle=0;handle<MAXSAMPLEHANDLES;handle++)
	if (Samples[handle]==NULL)
	    break;

    if (handle==MAXSAMPLEHANDLES) {
	gModPlayerErrorMessage = ERROR_OUT_OF_HANDLES;
	return -1;
    }

    if ((Samples[handle]=(SBYTE *)mod_malloc(length+16))==NULL) {
	gModPlayerErrorMessage = ERROR_SAMPLE_TOO_BIG;
	return -1;
    }

    // fix whacked out samples
	
    if ( flags & SF_LOOP ) {
	if ( loopend > length ) 
	    loopend=length;  
	if ( loopstart > length ) 
	    flags &= ~SF_LOOP;  
    }

    /* read sample into buffer. */
    LargeRead(Samples[handle], length);

    /* Unclick samples: */
    if (flags & SF_LOOP) {
	if (flags & SF_BIDI)
	    for (t=0;t<16;t++) Samples[handle][loopend+t]=Samples[handle][(loopend-t)-1];
	else
	    for (t=0;t<16;t++) Samples[handle][loopend+t]=Samples[handle][t+loopstart];
    } else {
	for (t=0;t<16;t++) Samples[handle][t+length]=0;
    }

    return handle;
}



void VC_SampleUnload(SWORD handle)
{
    void *sampleadr=Samples[handle];

    mod_free(sampleadr);
    Samples[handle]=NULL;
}


/**************************************************
***************************************************
***************************************************
**************************************************/


#ifndef __WATCOMC__


static void (*SampleMix)(SBYTE *srce, SLONG *dest, SLONG index, SLONG increment, UWORD todo);


static void MixStereoNormal(SBYTE *srce, SLONG *dest, SLONG index, SLONG increment, UWORD todo)
{
    SBYTE sample;

    while (todo>0) {
	sample=srce[index>>FRACBITS];
	*(dest++)+=lvolmul*sample;
	*(dest++)+=rvolmul*sample;
	index+=increment;
	todo--;
    }
}


static void MixMonoNormal(SBYTE *srce, SLONG *dest, SLONG index, SLONG increment, UWORD todo)
{
    SBYTE sample;

    while (todo>0) {
	sample=srce[index>>FRACBITS];
	*(dest++)+=lvolmul*sample;
	index+=increment;
	todo--;
    }
}


static void MixStereoInterp(SBYTE *srce, SLONG *dest, SLONG index, SLONG increment, UWORD todo)
{
    SWORD sample, a,b;

    while (todo>0) {
	a=srce[index>>FRACBITS];
	b=srce[1+(index>>FRACBITS)];
	sample= (SWORD)(a + (((long)(b - a) * (index&FRACMASK)) >> FRACBITS));

	*(dest++)+=lvolmul*sample;
	*(dest++)+=rvolmul*sample;
	index+=increment;
	todo--;
    }
}


static void MixMonoInterp(SBYTE *srce, SLONG *dest, SLONG index, SLONG increment, UWORD todo)
{
    SWORD sample, a,b;

    while (todo>0) {
	a=srce[index>>FRACBITS];
	b=srce[1+(index>>FRACBITS)];
	sample = (SWORD)(a + (((long)(b-a)*(index&FRACMASK))>>FRACBITS));

	*(dest++)+=lvolmul*sample;

	index+=increment;
	todo--;
    }
}

#endif


static UWORD NewPredict(SLONG index, SLONG end, SLONG increment, UWORD todo)
/*
	This functions returns the number of resamplings we can do so that:

		- it never accesses indexes bigger than index 'end'
		- it doesn't do more than 'todo' resamplings
*/
{
    SLONG di;

    di=(end-index)/increment;
    index+=(di*increment);

    if (increment<0) {
	while (index>=end) {
	    index+=increment;
	    di++;
	}
    }
    else {
	while (index<=end) {
	    index+=increment;
	    di++;
	}
    }
    return ((di<todo) ? di : todo);
}


static void VC_AddChannel(SLONG *ptr, UWORD todo)
/*
	Mixes 'todo' stereo or mono samples of the current channel to the tickbuffer.
*/
{
    SLONG end;
    UWORD done;
    SBYTE *s;

    /* Vraag een far ptr op van het sampleadres
       op byte offset vnf->current, en hoeveel samples
       daarvan geldig zijn (VOORDAT segment overschrijding optreed) */

    if (!(s=Samples[vnf->handle])) {
	vnf->current=0;
	vnf->active=0;
	return;
    }

    while (todo>0) {

	/* update the 'current' index so the sample loops, or
	   stops playing if it reached the end of the sample */

	if (vnf->flags&SF_REVERSE) {

	    /* The sample is playing in reverse */

	    if (vnf->flags&SF_LOOP) {

		/* the sample is looping, so check if
		   it reached the loopstart index */

		if (vnf->current<idxlpos) {
		    if (vnf->flags&SF_BIDI) {

			/* sample is doing bidirectional loops, so 'bounce'
			   the current index against the idxlpos */

			vnf->current=idxlpos+(idxlpos-vnf->current);
			vnf->flags&=~SF_REVERSE;
			vnf->increment=-vnf->increment;
		    }
		    else
			/* normal backwards looping, so set the
			   current position to loopend index */

			vnf->current=idxlend-(idxlpos-vnf->current);
		}
	    }
	    else {

		/* the sample is not looping, so check
		   if it reached index 0 */

		if (vnf->current<0) {

		    /* playing index reached 0, so stop
		       playing this sample */

		    vnf->current=0;
		    vnf->active=0;
		    break;
		}
	    }
	}
	else {

	    /* The sample is playing forward */

	    if (vnf->flags&SF_LOOP) {

		/* the sample is looping, so check if
		   it reached the loopend index */

		if (vnf->current>idxlend) {
		    if (vnf->flags&SF_BIDI) {

			/* sample is doing bidirectional loops, so 'bounce'
			   the current index against the idxlend */

			vnf->flags|=SF_REVERSE;
			vnf->increment=-vnf->increment;
			vnf->current=idxlend-(vnf->current-idxlend); /* ?? */
		    }
		    else
			/* normal backwards looping, so set the
			   current position to loopend index */

			vnf->current=idxlpos+(vnf->current-idxlend);
		}
	    }
	    else {

		/* sample is not looping, so check
		   if it reached the last position */

		if (vnf->current>idxsize) {

		    /* yes, so stop playing this sample */

		    vnf->current=0;
		    vnf->active=0;
		    break;
		}
	    }
	}


	if (vnf->flags & SF_REVERSE)
	    end = (vnf->flags & SF_LOOP) ? idxlpos : 0;
	else
	    end = (vnf->flags & SF_LOOP) ? idxlend : idxsize;

	/* Als de sample simpelweg niet beschikbaar is, of als
	   sample gestopt moet worden sample stilleggen en stoppen */
	/* mix 'em: */

	done=NewPredict(vnf->current, end,vnf->increment,todo);

	if (!done) {
	    /*			printf("predict stopped it. current %ld, end %ld\n", vnf->current,end);
	     */			vnf->active=0;
	     break;
	}

	/* optimisation: don't mix anything if volume is zero */

	if (vnf->vol) {
#ifdef __WATCOMC__
	    if (md_mode & DMODE_STEREO)
		AsmStereoNormal(s, ptr,vnf->current,vnf->increment,done);
	    else
		AsmMonoNormal(s, ptr,vnf->current,vnf->increment,done);
#else
	    SampleMix(s, ptr,vnf->current,vnf->increment,done);
#endif
	}
	vnf->current+=(vnf->increment*done);

	todo-=done;
	ptr+=(md_mode & DMODE_STEREO) ? (done<<1) : done;
    }
}




static void VC_FillTick(SBYTE *buf, UWORD todo)
/*
	Mixes 'todo' samples to 'buf'.. The number of samples has
	to fit into the tickbuffer.
*/
{
    int t;

    /* clear the mixing buffer: */

    mod_memset(VC_TICKBUF, 0, (md_mode & DMODE_STEREO) ? todo << 3 : todo << 2);

    for (t = 0; t<md_numchn; t++)
	{
	    vnf=&vinf[t];

	    if (vnf->active)
		{
		    idxsize=(vnf->size<<FRACBITS)-1;
		    idxlpos=vnf->reppos<<FRACBITS;
		    idxlend=(vnf->repend<<FRACBITS)-1;
		    lvolmul=vnf->lvolmul;
		    rvolmul=vnf->rvolmul;
		    VC_AddChannel(VC_TICKBUF, todo);
		}
	}

    VC_Sample32To16Copy(VC_TICKBUF, (SWORD *)buf,
			(md_mode & DMODE_STEREO) ? todo << 1 : todo,
			(UBYTE)(16 - ampshift));
}



static void VC_WritePortion(SBYTE *buf, UWORD todo)
/*
	Writes 'todo' mixed SAMPLES (!!) to 'buf'. When todo is bigger than the
	number of samples that fit into VC_TICKBUF, the mixing operation is split
	up into a number of smaller chunks.
*/
{
    UWORD part;

    /* write 'part' samples to the buffer */

    while (todo) {
	part=MIN_VALUE(todo, samplesthatfit);
	VC_FillTick(buf, part);
	buf+=samples2bytes(part);
	todo-=part;
    }
}


static UWORD TICKLEFT;


void VC_WriteSamples(SBYTE *buf, UWORD todo)
{
    int t;
    UWORD part;

    while (todo>0) {

	if (TICKLEFT==0) {
	    md_player();

	    TICKLEFT=(125L*md_mixfreq)/(50L*md_bpm);

	    /* compute volume, frequency counter & panning parameters for each channel. */

	    for (t=0;t<md_numchn;t++) {
		int pan, vol,lvol,rvol;

		if (vinf[t].kick) {
		    vinf[t].current=(vinf[t].start << FRACBITS);
		    vinf[t].active=1;
		    vinf[t].kick=0;
		}

		if (vinf[t].frq==0) vinf[t].active=0;

		if (vinf[t].active) {
		    vinf[t].increment=fraction2long(vinf[t].frq, md_mixfreq);

		    if (vinf[t].flags & SF_REVERSE) vinf[t].increment=-vinf[t].increment;

		    vol=vinf[t].vol;
		    pan=vinf[t].pan;

		    if (md_mode & DMODE_STEREO) {
			lvol= ( vol * (255-pan) ) / 255;
			rvol= ( vol * pan ) / 255;
			vinf[t].lvolmul=(maxvol*lvol)/64;
			vinf[t].rvolmul=(maxvol*rvol)/64;
		    }
		    else {
			vinf[t].lvolmul=(maxvol*vol)/64;
		    }
		}
	    }
	}

	part=MIN_VALUE(TICKLEFT, todo);

	VC_WritePortion(buf, part);

	TICKLEFT-=part;
	todo-=part;

	buf+=samples2bytes(part);
    }
}


UWORD VC_WriteBytes(SBYTE *buf, UWORD todo)
/*
	Writes 'todo' mixed SBYTES (!!) to 'buf'. It returns the number of
	SBYTES actually written to 'buf' (which is rounded to number of samples
	that fit into 'todo' bytes).
*/
{
    todo=bytes2samples(todo);
    VC_WriteSamples(buf, todo);
    return samples2bytes(todo);
}


void VC_SilenceBytes(SBYTE *buf, UWORD todo)
/*
	Fill the buffer with 'todo' bytes of silence (it depends on the mixing
	mode how the buffer is filled)
*/
{
    /* clear the buffer to zero (16 bits
       signed ) or 0x80 (8 bits unsigned) */

    mod_memset(buf, (char)(md_mode & DMODE_16BITS ? 0 : 0x80), todo);

}


void VC_PlayStart(void)
{

    int t;

    for(t=0;t<32;t++){
	vinf[t].current=0;
	vinf[t].flags=0;
	vinf[t].handle=0;
	vinf[t].kick=0;
	vinf[t].active=0;
	vinf[t].frq=10000;
	vinf[t].vol=0;
	vinf[t].pan=(t&1)?0:255;
    }

    if(md_numchn>0)	/* sanity check - avoid core dump! */
	maxvol=16777216L / md_numchn;
    else
	maxvol=16777216L;


    /* instead of using a amplifying lookup table, I'm using a simple shift
       amplify now.. amplifying doubles with every extra 4 channels, and also
       doubles in stereo mode.. this seems to give similar volume levels
       across the channel range */

    ampshift=md_numchn/8;
    if (md_mode & DMODE_STEREO) ampshift++;

    if (md_mode & DMODE_INTERP)
	SampleMix=(md_mode & DMODE_STEREO) ? MixStereoInterp : MixMonoInterp;
    else
	SampleMix=(md_mode & DMODE_STEREO) ? MixStereoNormal : MixMonoNormal;

    samplesthatfit=TICKLSIZE;
    if (md_mode & DMODE_STEREO) samplesthatfit>>=1;
    TICKLEFT=0;
}


void VC_PlayStop(void)
{
}


BOOL VC_Init(void)
{
    int t;
	
    VC_InitializeMemory();
	
    for (t=0;t<32;t++) {
	vinf[t].current=0;
	vinf[t].flags=0;
	vinf[t].handle=0;
	vinf[t].kick=0;
	vinf[t].active=0;
	vinf[t].frq=10000;
	vinf[t].vol=0;
	vinf[t].pan=(t&1)?0:255;
    }

#ifdef __WATCOMC__
    if (md_mode & DMODE_INTERP) md_mode&=~DMODE_INTERP;

    for (t=0;t<65;t++) volsel[t]=0;

    for (t=0;t<65;t++) {
	if (!(volsel[t]=getalias())) return 0;
	setbase(volsel[t], (ULONG)voltab[t]);
    }
#endif

    return 1;
}


void VC_Exit(void)
{
#ifdef __WATCOMC__
    int t;
    for (t=0;t<65;t++) {
	if (volsel[t]) freedescriptor(volsel[t]);
    }
#endif

    VC_FinalizeMemory();
}


void VC_VoiceSetVolume(UBYTE voice, UBYTE vol)
{
    vinf[voice].vol=vol;
}


void VC_VoiceSetFrequency(UBYTE voice, ULONG frq)
{
    vinf[voice].frq=frq;
}


void VC_VoiceSetPanning(UBYTE voice, UBYTE pan)
{
    vinf[voice].pan=pan;
}


void VC_VoicePlay(UBYTE voice, SWORD handle, ULONG start, ULONG size, ULONG reppos, ULONG repend, UWORD flags)
{
    if (start>=size) return;

    if (flags&SF_LOOP) {
	if (repend>size) repend=size;    /* repend can't be bigger than size */
    }

    vinf[voice].flags=flags;
    vinf[voice].handle=handle;
    vinf[voice].start=start;
    vinf[voice].size=size;
    vinf[voice].reppos=reppos;
    vinf[voice].repend=repend;
    vinf[voice].kick=1;
}

#endif	// USE_MOD_API

