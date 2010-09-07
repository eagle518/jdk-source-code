/*
 * @(#)MOD_mdriver.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**
**	History	-
**	12/1/96		Created
**	4/18/97		Removed MOD_Driver layer
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
MDRIVER.C

Description:
These routines are used to access the available soundcard drivers.

Portability:
All systems - all compilers

*/

//DRIVER *md_driver;

UWORD md_device         =0;
UWORD md_mixfreq        =22050;
UWORD md_mode           =0;
UBYTE md_numchn         =0;
UBYTE md_bpm            =125;

static void dummyplay(void)
{
}

void (*md_player)(void)=dummyplay;

static SWORD sl_old;
static UWORD sl_infmt;
static UWORD sl_outfmt;
static SWORD* sl_buffer;	// static SWORD sl_buffer[1024];

BOOL gModIsPlaying = 0;


void SL_Init(UWORD infmt,UWORD outfmt)
{
    sl_old=0;
    sl_infmt=infmt;
    sl_outfmt=outfmt;
}


void SL_Exit(void);
void SL_Exit(void)
{
}

#define MM_BIG_ENDIAN
#define endian_switch(x) 			(((((UWORD)(x)) & 0xFF00) >> 8) | 		\
			  			    			((((UWORD)(x)) & 0xFF) << 8))

void SL_Load(void *buffer, ULONG length)
{
    SBYTE *bptr=(SBYTE *)buffer;
    SWORD *wptr=(SWORD *)buffer;
    UWORD stodo;
    int t;

    /* compute number of samples to load */
    //	if (sl_outfmt & SF_16BITS) length>>=1;

#ifdef DEBUG
    if (IsError(sl_buffer == NULL))
	;
#endif
    while (length) {

	stodo= (length<1024) ? (UWORD)length : 1024;

	if (sl_infmt&SF_16BITS) {
	    if ( !(sl_infmt&SF_BIG_ENDIAN) ) {
		SWORD *sp = (SWORD *) sl_buffer;
		for ( t=0; t <  stodo; t++ ) {
		    *sp++ = _mm_read_I_SWORD();
		}
	    }
	    else
		_mm_read_str((char *) sl_buffer,sizeof(SWORD)*stodo);
	}
	else {
	    SBYTE *s;
	    SWORD *d;

	    _mm_read_str((char *)sl_buffer,sizeof(SBYTE)*stodo);

	    s=(SBYTE *)sl_buffer;
	    d=sl_buffer;
	    s+=stodo;
	    d+=stodo;

	    for (t=0;t<stodo;t++) {
		s--;
		d--;
		*d=(*s)<<8;
	    }
	}

	if (sl_infmt & SF_DELTA) {
	    for (t=0;t<stodo;t++) {
		sl_buffer[t]+=sl_old;
		sl_old=sl_buffer[t];
	    }
	}

	if ((sl_infmt^sl_outfmt) & SF_SIGNED) {
	    for (t=0;t<stodo;t++) {
		sl_buffer[t]^=0x8000;
	    }
	}

	if (sl_outfmt & SF_16BITS) {
	    for (t=0;t<stodo;t++) *(wptr++)=sl_buffer[t];
	}
	else {
	    for (t=0;t<stodo;t++) *(bptr++)=sl_buffer[t]>>8;
	}

	length-=stodo;
    }
}


void MD_InfoDriver(void)
{
}

/*
void MD_RegisterDriver(DRIVER *drv)
{
//	md_driver = drv;
}
*/

SWORD MD_SampleLoad(ULONG size,ULONG reppos,ULONG repend,UWORD flags)
{
    //	SWORD result=md_driver->SampleLoad(size,reppos,repend,flags);
    SWORD result=VC_SampleLoad(size,reppos,repend,flags);

    SL_Exit();
    return result;
}


void MD_SampleUnLoad(SWORD handle)
{
    //	md_driver->SampleUnLoad(handle);
    VC_SampleUnload(handle);
}


BOOL MD_Init(void)
{
    if (sl_buffer == NULL)
	sl_buffer = (SWORD*)mod_malloc(1024*sizeof(SWORD));
    //	return md_driver->Init();
    if(!VC_Init()) {
	return 0;
    }
    return 1;
}


void MD_Exit(void)
{
    //	if (md_driver)
    //	{
    //		md_driver->Exit();
    //	}
    VC_Exit();
    mod_free(sl_buffer); 
    sl_buffer = NULL;
}


void MD_PlayStart(void)
{
    /* safety valve, prevents entering
       playstart twice: */

    if (gModIsPlaying) return;
    //	md_driver->PlayStart();
    VC_PlayStart();

    gModIsPlaying=1;
}


void MD_PlayStop(void)
{
    /* safety valve, prevents calling playStop when playstart
       hasn't been called: */

    if (gModIsPlaying) 
	{
	    gModIsPlaying = 0;
	    //		md_driver->PlayStop();
	    VC_PlayStop();
	}
}


void MD_SetBPM(UBYTE bpm)
{
    md_bpm=bpm;
}


void MD_RegisterPlayer(void (*player)(void))
{
    md_player=player;
}

/*
void MD_Update(void)
{
	if (gModIsPlaying)
		md_driver->Update();
}
*/

void MD_VoiceSetVolume(UBYTE voice,UBYTE vol)
{
    //	md_driver->VoiceSetVolume(voice,vol);
    VC_VoiceSetVolume(voice,vol);
}


void MD_VoiceSetFrequency(UBYTE voice,ULONG frq)
{
    //	md_driver->VoiceSetFrequency(voice,frq);
    VC_VoiceSetFrequency(voice, frq);
}


void MD_VoiceSetPanning(UBYTE voice,ULONG pan)
{
    //	md_driver->VoiceSetPanning(voice,(UBYTE)pan);
    VC_VoiceSetPanning(voice,(UBYTE)pan);
}


void MD_VoicePlay(UBYTE voice,SWORD handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,UWORD flags)
{
    //	md_driver->VoicePlay(voice,handle,start,size,reppos,repend,flags);
    VC_VoicePlay(voice,handle,start,size,reppos,repend,flags);
}

#endif	// USE_MOD_API

