/*
 * @(#)GenModFiles.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	GetModFiles.c
**
**		Plays MOD files
**
**
**	History	-
**	12/1/96		Created
**	12/30/96	Changed copyright
**	1/22/97		Added support for MOD code in all output loops
**	1/23/97		Added async fade code via PV_ServeModFade
**	1/24/97		Changed GM_GetModVolume to return 0 if not playing
**				Added GM_SetModLoop & GM_GetModTempoBPM & GM_SetModTempoBPM
**	2/5/97		Changed MOD handling not to keep MOD loaded into memory once parsed
**	3/13/97		Removed some type casting errors
**	3/18/97		Added GM_GetModLoop
**	4/18/97		Removed MOD_Driver layer
**	6/10/97		Added GM_SetModReverbStatus
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/8/98		Changed BOOL_FLAG to XBOOL
*/
/*****************************************************************************/

#include "X_API.h"
#include "GenSnd.h"
#include "GenPriv.h"

#if USE_MOD_API == TRUE

#include "MOD_mikmod.h"

//	GLOBAL variables for MOD engine
//		modptr		- source file. Used only for parsing file. Once loaded you can NULL out
//		modsize		- source file size
//		modpos		- position
//		md_mixfreq	- sample playback rate ie. 22050
//		md_mode		- mode DMODE_16BITS|DMODE_STEREO|DMODE_INTERP
//		md_device	- set to 0
//		md_numchn	- number of channels
//

extern UWORD					repcnt;
extern UWORD					mp_sngspd;       /* current songspeed */

unsigned char					*modptr;
long							modpos, modsize;

static char						modLoaded = FALSE;
static char						modSetup = FALSE;
static char						modPaused = FALSE;

static void PV_CallModCallback(GM_ModData *mod)
{
    if (mod)
	{
	    if (mod->callback)
		{
		    (*mod->callback)(mod);
		}
	    mod->callback = NULL;
	}
}

// Process any fading mod files
static void PV_ServeModFade(GM_ModData *modReference)
{
    long	value;

    if (modReference)
	{
	    if (modReference->modFadeRate)
		{
		    modReference->modFixedVolume -= modReference->modFadeRate;
		    value = XFIXED_TO_LONG(modReference->modFixedVolume);
		    if (value > modReference->modFadeMaxVolume)
			{
			    value = modReference->modFadeMaxVolume;
			    modReference->modFadeRate = 0;
			}
		    if (value < modReference->modFadeMinVolume)
			{
			    value = modReference->modFadeMinVolume;
			    modReference->modFadeRate = 0;
			}
		    GM_SetModVolume(modReference, (INT16)value);
		    if ((modReference->modFadeRate == 0) && modReference->modEndAtFade)
			{
			    GM_StopModFile(modReference);
			}
		}
	}
}

static void PV_TickHandler(void)
{
    static long	ticks = 0;

    if (modPaused == FALSE)
	{
	    MP_HandleTick();    /* play 1 tick of the module */
	    MD_SetBPM(mp_bpm);
	    PV_ServeModFade((GM_ModData *)MusicGlobals->pModPlaying);
	    ticks++;
	    if (ticks & 1)
		{
		    if (MP_Ready()) 
			{
			    modPaused = TRUE;
			    MD_PlayStop();          /* stop playing */
			    PV_CallModCallback((GM_ModData *)MusicGlobals->pModPlaying);
			    MusicGlobals->pModPlaying = NULL;
			}
		}
	}
}

// Will write the mod output in the left and right channel of the
// MusicGlobals variable
void PV_WriteModOutput(Quality q, XBOOL stereo)
{
    INT32		*sourceLR;
    GM_ModData	*mod;
    LOOPCOUNT	count, doublecount;
    short int	dmaBuffer[MAX_CHUNK_SIZE * 4];	// 16 bit, stereo plus extra
    long		volume;

    mod = (GM_ModData *)MusicGlobals->pModPlaying;
    q = q;
    if (mod)
	{
	    sourceLR = &MusicGlobals->songBufferDry[0];
	    VC_WriteSamples((SBYTE *) dmaBuffer, (UWORD)MusicGlobals->One_Loop);
	    volume = (MusicGlobals->MasterVolume * mod->modVolume) / MAX_SONG_VOLUME;
	    if (stereo)
		{
		    // walk through MOD stereo data and mix it into HAE's mix buffers
		    for (count = 0; count < MusicGlobals->One_Loop * 2; count += 2)
			{
			    sourceLR[count + 0] += dmaBuffer[count + 0] * volume;
			    sourceLR[count + 1] += dmaBuffer[count + 1] * volume;
			}
		}
	    else
		{
		    // walk through MOD stereo data and mix it down to mono then into HAE's mix buffers
		    for (count = 0; count < MusicGlobals->One_Loop; count++)
			{
			    doublecount = count << 1;
			    sourceLR[count] += ((dmaBuffer[doublecount + 0] + 
						 dmaBuffer[doublecount + 1]) * volume) / 2;
			}
		}
	}
}

GM_ModData * GM_LoadModFile(void *pModFile, long fileSize)
{
    GM_ModData	*mod;

    if (fileSize < 1000)
	{
	    return NULL;
	}
    // Can only have one MOD file loaded at a time
    if (modLoaded)
	{
	    return NULL;
	}
    mod = (GM_ModData *)XNewPtr((long)sizeof(GM_ModData));
    if (mod)
	{
	    mod->modVolume = MAX_SONG_VOLUME;
	    mod->enableReverb = TRUE;
	    modptr = (unsigned char*)pModFile;
	    modsize = fileSize;
	    modpos = 0;

	    /*
	      Initialize soundcard parameters.. you _have_ to do this
	      before calling MD_Init(), and it's illegal to change them
	      after you've called MD_Init()
	    */

	    // always generate 16 bit, stereo. We dither down.
	    md_mode = DMODE_16BITS | DMODE_STEREO | DMODE_INTERP;
	    md_mixfreq = (UWORD)GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality);
	    md_device = 0;                                     /* standard device: autodetect */
	
	    /*
	      Register the loaders we want to use..
	    */

	    if (modSetup == FALSE)
		{
		    modSetup = TRUE;
		    ML_RegisterLoader(&load_m15);    /* if you use m15load, register it as first! */
		    ML_RegisterLoader(&load_mod);
		    ML_RegisterLoader(&load_mtm);
		    ML_RegisterLoader(&load_s3m);
		    ML_RegisterLoader(&load_stm);
		    ML_RegisterLoader(&load_ult);
		    ML_RegisterLoader(&load_uni);
		    //			ML_RegisterLoader(&load_xm);	// xm mod files are not working

		    /*
		      Register the drivers we want to use:
		    */

		    MD_RegisterPlayer(PV_TickHandler);
		}

	    /* Parse option switches using standard getopt function: */
	    /*  initialize soundcard */

	    if (MD_Init())
		{
		    mod->modControl = ML_LoadFN();
		    if (mod->modControl)
			{
				/*	initialize modplayer to play this module */
			    MP_Init((UNIMOD *)mod->modControl);
	
			    md_numchn = ((UNIMOD *)mod->modControl)->numchn;
			    modLoaded = TRUE;
			    repcnt = 0;
			    modptr = NULL;		// file is loaded
			    modsize = 0;
			}
		    else
			{
			    MD_Exit();
			    XDisposePtr(mod);
			    mod = NULL;
			}
		}
	    else
		{
		    MD_Exit();
		    XDisposePtr(mod);
		    mod = NULL;
		}
	}
    return mod;
}

void GM_FreeModFile(GM_ModData *mod)
{
    if (mod && modLoaded)
	{
	    GM_StopModFile(mod);	         	 // stop playing
	    ML_Free((UNIMOD *)mod->modControl);            // and free the module
	    MD_Exit();
	
	    XDisposePtr(mod);
	    modLoaded = FALSE;
	}
}

void GM_BeginModFile(GM_ModData *mod, GM_ModDoneCallbackPtr callback, long reference)
{
    if (mod && modLoaded)
	{
	    /*  start playing the module: */

	    //		modptr = (unsigned char*)mod->modFileData;
	    //		modsize = mod->modFileSize;
	    modpos = 0;

	    /*	initialize modplayer to play this module */
	    MP_Init((UNIMOD *)mod->modControl);
	    md_numchn = ((UNIMOD *)mod->modControl)->numchn;

	    MD_PlayStart();
	    MusicGlobals->pModPlaying = mod;
	    mod->reference = reference;
	    mod->callback = callback;
	    modPaused = FALSE;
	}
}

void GM_StopModFile(GM_ModData *mod)
{
    if (mod && modLoaded)
	{
	    PV_CallModCallback(mod);
	    MD_PlayStop();
	    MusicGlobals->pModPlaying = NULL;
	}
}

XBOOL GM_IsModPlaying(GM_ModData *mod)
{
    XBOOL	play;

    play = FALSE;
    if (mod && modLoaded)
	{
	    play = ! MP_Ready();
	}
    return play;
}


short int GM_GetModVolume(GM_ModData *mod)
{
    short int	volume;

    volume = 0;
    if (mod && modLoaded)
	{
	    volume = mod->modVolume;
	}
    return volume;
}

void GM_SetModVolume(GM_ModData *mod, short int volume)
{
    if (mod && modLoaded)
	{
	    mod->modVolume = volume;
	}
}

// Set MOD song fade rate. Its a 16.16 fixed value
// Input:	mod			mod song to affect
//			fadeRate	amount to change every 11 ms
//						example:	FLOAT_TO_XFIXED(2.2) will decrease volume
//									FLOAT_TO_XFIXED(2.2) * -1 will increase volume
//			minVolume	lowest volume level fade will go
//			maxVolume	highest volume level fade will go
void GM_SetModFadeRate(GM_ModData *mod, XFIXED fadeRate, 
		       INT16 minVolume, INT16 maxVolume, XBOOL endSong)
{
    if (mod && modLoaded)
	{
	    mod->modFixedVolume = LONG_TO_XFIXED(mod->modVolume);
	    mod->modFadeMaxVolume = maxVolume;
	    mod->modFadeMinVolume = minVolume;
	    mod->modFadeRate = fadeRate;
	    mod->modEndAtFade = endSong;
	}
}



void GM_PauseMod(GM_ModData *mod)
{
    if (mod && modLoaded)
	{
	    modPaused = TRUE;
	}
}

void GM_ResumeMod(GM_ModData *mod)
{
    if (mod && modLoaded)
	{
	    md_mixfreq = (UWORD)GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality);
	    modPaused = FALSE;
	}
}

void GM_SetModTempoFactor(GM_ModData *mod, unsigned long fixedFactor)
{
    if (mod && modLoaded)
	{
	    mp_sngspd = (UWORD)((mp_sngspd * fixedFactor) / 65536L);
	}
}

void GM_SetModReverbStatus(GM_ModData *mod, XBOOL enableReverb)
{
    if (mod && modLoaded)
	{
	    mod->enableReverb = enableReverb;
	}
}

void GM_SetModTempoBPM(GM_ModData *mod, unsigned long newTempoBPM)
{
    if (mod && modLoaded)
	{
	    mp_bpm = (UBYTE)newTempoBPM;
	}
}

unsigned long GM_GetModTempoBPM(GM_ModData *mod)
{
    unsigned long tempo;

    tempo = 0;
    if (mod && modLoaded)
	{
	    tempo = mp_bpm;
	}
    return tempo;
}

void GM_SetModLoop(GM_ModData *mod, XBOOL loop)
{
    if (mod && modLoaded)
	{
	    mp_loop = loop;
	}
}

XBOOL GM_GetModLoop(GM_ModData *mod)
{
    if (mod && modLoaded)
	{
	    return (XBOOL)mp_loop;
	}
    return FALSE;
}

void GM_GetModSongName(GM_ModData *mod, char *cName)
{
    if (cName)
	{
	    cName[0] = 0;
		
	    if (mod && modLoaded)
		{
		    if (of.songname)
			{
			    XStrCpy(cName, of.songname);
			}
		}
	}
}

void GM_GetModSongComments(GM_ModData *mod, char *cName)
{
    if (cName)
	{
	    cName[0] = 0;
		
	    if (mod && modLoaded)
		{
		    if (of.comment)
			{
			    XStrCpy(cName, of.comment);
			}
		}
	}
}

unsigned long GM_GetModSongNameLength(GM_ModData *mod)
{
    unsigned long	size;

    size = 0;
    if (mod && modLoaded)
	{
	    if (of.songname)
		{
		    size = XStrLen(of.songname);
		}
	}
    return size;
}

unsigned long GM_GetModSongCommentsLength(GM_ModData *mod)
{
    unsigned long	size;

    size = 0;
    if (mod && modLoaded)
	{
	    if (of.songname)
		{
		    size = XStrLen(of.comment);
		}
	}
    return size;
}
#endif	// USE_MOD_API


// EOF of GetModFiles.c
