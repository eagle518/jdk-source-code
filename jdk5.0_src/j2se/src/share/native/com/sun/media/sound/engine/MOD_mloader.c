/*
 * @(#)MOD_mloader.c	1.11 03/12/19
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
MLOADER.C

Description:
These routines are used to access the available module loaders

Portability:
All systems - all compilers

*/


UNIMOD of;

static LOADER *firstloader=NULL;


const UWORD finetune[16]={
    8363,	8413,	8463,	8529,	8581,	8651,	8723,	8757,
    7895,	7941,	7985,	8046,	8107,	8169,	8232,	8280
};




void ML_RegisterLoader(LOADER *ldr)
{
    if (firstloader==NULL) {
	firstloader=ldr;
	ldr->next=NULL;
    }
    else{
	ldr->next=firstloader;
	firstloader=ldr;
    }
}



void *MyMalloc(long size)
/*
	Same as malloc, but sets error variable ml_errno when it failed
*/
{
    void *d;

    d=mod_malloc(size);
    if (d==NULL) {
	gModPlayerErrorMessage=ERROR_ALLOC_STRUCT;
    }
    return d;
}



void *MyCalloc(long nitems,long size)
/*
	Same as calloc, but sets error variable ml_errno when it failed
*/
{
    void *	d;
    long	i;
		
    i = nitems*size;

    d=(char*)mod_malloc(i);
    if (d==NULL) 
	{
	    gModPlayerErrorMessage=ERROR_ALLOC_STRUCT;
	} 
    else
	{
	    mod_memset(d, 0, i);
	}
    return d;
}



BOOL ReadComment(UWORD len)
{
    int t;

    if (len) {
	if (!(of.comment=(char *)MyMalloc(len+1))) return 0;
	_mm_read_str(of.comment,len);
	of.comment[len]=0;

	/* strip any control-characters in the comment: */

	for(t=0;t<len;t++) {
	    if (of.comment[t]<32) of.comment[t]=' ';
	}
    }
    return 1;
}



BOOL AllocPatterns(void)
{
    int s,t,tracks=0;

    /* Allocate track sequencing array */

    if (!(of.patterns=(UWORD *)MyCalloc((ULONG)of.numpat*of.numchn,sizeof(UWORD)))) return 0;
    if (!(of.pattrows=(UWORD *)MyCalloc(of.numpat,sizeof(UWORD)))) return 0;

    for(t=0;t<of.numpat;t++) {

	of.pattrows[t]=64;

	for(s=0;s<of.numchn;s++) {
	    of.patterns[(t*of.numchn)+s]=tracks++;
	}
    }

    return 1;
}


BOOL AllocTracks(void)
{
    if (!(of.tracks=(UBYTE **)MyCalloc(of.numtrk,sizeof(UBYTE *)))) return 0;
    return 1;
}



BOOL AllocInstruments(void)
{
    if (!(of.instruments=(INSTRUMENT *)MyCalloc(of.numins,sizeof(INSTRUMENT)))) return 0;
    return 1;
}


BOOL AllocSamples(INSTRUMENT *i)
{
    UWORD u,n;

    if ((n=i->numsmp) != 0) {
	if (!(i->samples=(SAMPLE *)MyCalloc(n,sizeof(SAMPLE)))) return 0;

	for(u=0; u<n; u++) {
	    i->samples[u].panning=128;
	    i->samples[u].handle=-1;
	}
    }
    return 1;
}


char *DupStr(char *s,UWORD len)
/*
	Creates a CSTR out of a character buffer of 'len' bytes, but strips
	any terminating non-printing characters like 0, spaces etc.
*/
{
    UWORD t;
    char *d=NULL;

    /* Scan for first printing char in buffer [includes high ascii up to 254] */

    while (len) {
        if (!(s[len-1]>=0 && s[len-1]<=0x20)) break;
	len--;
    }

    if (len) {

	/* When the buffer wasn't completely empty, allocate
	   a cstring and copy the buffer into that string, except
	   for any control-chars */

	if ((d=(char *)mod_malloc(len+1))!=NULL) {
            for(t=0;t<len;t++) d[t]=(s[t]>=0 && s[t]<32) ? ' ': s[t];
	    d[t]=0;
	}
    }

    return d;
}



BOOL ML_LoadSamples(void);
BOOL ML_LoadSamples(void)
{
    UWORD t,u;
    INSTRUMENT *i;
    SAMPLE *s;

    for(t=0;t<of.numins;t++) 
	{
	    i=&of.instruments[t];

	    for(u=0; u<i->numsmp; u++) 
		{
		    s=&i->samples[u];

		    /*			printf("Loading Sample %d\n",t); */

		    /* sample has to be loaded ? -> increase
		       number of samples and allocate memory and
		       load sample */

		    if (s->length) 
			{
			    if (s->seekpos) 
				{
				    _mm_fseek(s->seekpos,SEEK_SET);
				}

				/* Call the sample load routine of the driver module.
				   It has to return a 'handle' (>=0) that identifies
				   the sample */

			    s->handle=MD_SampleLoad(s->length,
						    s->loopstart,
						    s->loopend,
						    s->flags);

			    if (s->handle<0) 
				{
				    return 0;
				}
			}
		}
	}
    return 1;
}


static BOOL ML_LoadHeader(void)
{
    BOOL ok=0;
    LOADER *l;

    /* Try to find a loader that recognizes the module */

    for(l=firstloader; l!=NULL; l=l->next) {
	modpos = 0;
	if (l->Test()) break;
    }

    if (l==NULL) {
	gModPlayerErrorMessage=ERROR_NOT_A_MODULE;
	return 0;
    }

    /* init unitrk routines */

    if (!UniInit()) return 0;

    /* init module loader */

    if (l->Init()) {
	modpos = 0;
	ok=l->Load();
    }

    l->Cleanup();

    /* free unitrk allocations */

    UniCleanup();
    return ok;
}



static void ML_XFreeInstrument(INSTRUMENT *i)
{
    UWORD t;

    if (i->samples!=NULL) {
	for(t=0; t<i->numsmp; t++) {
	    if (i->samples[t].handle>=0) {
		MD_SampleUnLoad(i->samples[t].handle);
	    }
	}
	mod_free(i->samples);
    }
    if (i->insname!=NULL) mod_free(i->insname);
}


static void ML_FreeEx(UNIMOD *mf)
{
    UWORD t;

    if (mf->modtype!=NULL) mod_free(mf->modtype);
    if (mf->patterns!=NULL) mod_free(mf->patterns);
    if (mf->pattrows!=NULL) mod_free(mf->pattrows);

    if (mf->tracks!=NULL) {
	for(t=0;t<mf->numtrk;t++) {
	    if (mf->tracks[t]!=NULL) mod_free(mf->tracks[t]);
	}
	mod_free(mf->tracks);
    }

    if (mf->instruments!=NULL) {
	for(t=0;t<mf->numins;t++) {
	    ML_XFreeInstrument(&mf->instruments[t]);
	}
	mod_free(mf->instruments);
    }

    if (mf->songname!=NULL) mod_free(mf->songname);
    if (mf->comment!=NULL) mod_free(mf->comment);
}



/******************************************

	Next are the user-callable functions

******************************************/


void ML_Free(UNIMOD *mf)
{
    if (mf!=NULL) {
	ML_FreeEx(mf);
	mod_free(mf);
    }
    UniCleanup();
}




UNIMOD *ML_LoadFP(void)
{
    int t;
    UNIMOD *mf;

    /* init fileptr, clear errorcode, clear static modfile: */

    modpos = 0;
    gModPlayerErrorMessage=NULL;
    mod_memset(&of,0,sizeof(UNIMOD));

    /* init panning array */

    for(t=0;t<32;t++) {
	of.panning[t]=((t+1)&2)?255:0;
    }

    if (!ML_LoadHeader()) {
	ML_FreeEx(&of);
	return NULL;
    }

    if (!ML_LoadSamples()) {
	ML_FreeEx(&of);
	return NULL;
    }

    if (!(mf=(UNIMOD *)MyCalloc(1,sizeof(UNIMOD)))) {
	ML_FreeEx(&of);
	return NULL;
    }

    /* Copy the static UNIMOD contents
       into the dynamic UNIMOD struct */

    //	*mf = of;
    memcpy(mf,&of,sizeof(UNIMOD));

    return mf;
}



UNIMOD *ML_LoadFN(void)
{
    UNIMOD *mf;

    mf=ML_LoadFP();

    return mf;
}

#endif	// USE_MOD_API

