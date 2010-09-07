/*
 * @(#)MOD_mikmod.h	1.12 03/12/19
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
*/
/*****************************************************************************/
#ifndef MIKMOD_H
#define MIKMOD_H

#include "X_API.h"

//typedef signed char	SBYTE;          /* has to be 1 byte signed */
//typedef unsigned char	UBYTE;          /* has to be 1 byte unsigned */
typedef short			SWORD;          /* has to be 2 bytes signed */
typedef unsigned short	UWORD;          /* has to be 2 bytes unsigned */
typedef long			SLONG;          /* has to be 4 bytes signed */
#ifndef _WINDOWS_
typedef unsigned long	ULONG;          /* has to be 4 bytes unsigned */
typedef int				BOOL;           /* doesn't matter.. 0=FALSE, <>0 true */
#endif

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define abs(x)			(((x)<0) ? -(x) : (x))

#ifndef NULL
#define NULL	0L
#endif

#define strlen(x)					XStrLen((char *)(x))
#define strcpy(x, y)				XStrCpy((char *)(x), (char *)(y))
#define strdup(str)					XDuplicateStr((char *)str)
#define memcpy(dest, src, size)		XBlockMove(src, dest, size)
#define memcmp(dest, src, size)		XMemCmp(dest, src, size)
#define strncmp(s1, s2, size)		XStrnCmp(s1, s2, size)
#define mod_malloc(size)			XNewPtr(size)
#define mod_free(x)					XDisposePtr(x)
#define mod_memset(ptr, c, size)	XSetMemory(ptr, size, c)


void*				mod_realloc(void *ptr, long size);

#define mikbanner \
"=======================================================================\n" \
"MIKMOD v2.10 - Portable version -  Programmed by MikMak of HaRDCoDE '95\n" \
"-----------------------------------------------------------------------\n" \
"Macintosh Sound Driver (MSD) - Copyright (c) 1996 Dmitry Boldyrev [DEMOS]\n" \
"=======================================================================\n" \
"This program is SHAREWARE - Read MIKMOD.TXT for more info\n" \
"E-Mail : mikmak@stack.urc.tue.nl\n"

/*
	error variables:
	===============
*/

extern const char *ERROR_ALLOC_STRUCT;
extern const char *ERROR_LOADING_PATTERN;
extern const char *ERROR_LOADING_TRACK;
extern const char *ERROR_LOADING_HEADER;
extern const char *ERROR_NOT_A_MODULE;
extern const char *ERROR_LOADING_SAMPLEINFO;
extern const char *ERROR_OUT_OF_HANDLES;
extern const char *ERROR_SAMPLE_TOO_BIG;
extern const char *gModPlayerErrorMessage;



#define _mm_rewind(x) _mm_fseek(x,0,SEEK_SET)
int  _mm_fseek(long offset,int whence);
void _mm_setiobase(long iobase);
long _mm_ftell();


extern SBYTE  _mm_read_SBYTE   (void);
extern UBYTE _mm_read_UBYTE  (void);

extern SWORD  _mm_read_M_SWORD (void);
extern SWORD  _mm_read_I_SWORD (void);

extern UWORD _mm_read_M_UWORD(void);
extern UWORD _mm_read_I_UWORD(void);

extern SLONG  _mm_read_M_SLONG (void);
extern SLONG  _mm_read_I_SLONG (void);

extern ULONG _mm_read_M_ULONG(void);
extern ULONG _mm_read_I_ULONG(void);

extern int _mm_read_str(char *str, int size);

extern int _mm_read_SBYTES    (SBYTE  *buffer, int number);
extern int _mm_read_UBYTES   (UBYTE *buffer, int number);

extern int _mm_read_M_SWORDS  (SWORD  *buffer, int number);
extern int _mm_read_I_SWORDS  (SWORD  *buffer, int number);

extern int _mm_read_M_UWORDS (UWORD *buffer, int number);
extern int _mm_read_I_UWORDS (UWORD *buffer, int number);

extern int _mm_read_M_SLONGS  (SLONG  *buffer, int number);
extern int _mm_read_I_SLONGS  (SLONG  *buffer, int number);

extern int _mm_read_M_ULONGS (ULONG *buffer, int number);
extern int _mm_read_I_ULONGS (ULONG *buffer, int number);



/**************************************************************************
****** Unitrack stuff: ****************************************************
**************************************************************************/

/*
	prototypes:
*/

void 	UniSetRow(UBYTE *t);
UBYTE	UniGetByte(void);
UBYTE  *UniFindRow(UBYTE *t,UWORD row);
void   	UniReset(void);
void   	UniWrite(UBYTE data);
void   	UniNewline(void);
void 	UniInstrument(UBYTE ins);
void 	UniNote(UBYTE note);
void 	UniPTEffect(UBYTE eff,UBYTE dat);
UBYTE  *UniDup(void);
void 	UniSkipOpcode(UBYTE op);
BOOL	UniInit(void);
void	UniCleanup(void);
UWORD   TrkLen(UBYTE *t);
BOOL	MyCmp(UBYTE *a,UBYTE *b,UWORD l);

/*
	all known effects:
*/

enum {
    UNI_NOTE=1,
    UNI_INSTRUMENT,
    UNI_PTEFFECT0,
    UNI_PTEFFECT1,
    UNI_PTEFFECT2,
    UNI_PTEFFECT3,
    UNI_PTEFFECT4,
    UNI_PTEFFECT5,
    UNI_PTEFFECT6,
    UNI_PTEFFECT7,
    UNI_PTEFFECT8,
    UNI_PTEFFECT9,
    UNI_PTEFFECTA,
    UNI_PTEFFECTB,
    UNI_PTEFFECTC,
    UNI_PTEFFECTD,
    UNI_PTEFFECTE,
    UNI_PTEFFECTF,
    UNI_S3MEFFECTA,
    UNI_S3MEFFECTD,
    UNI_S3MEFFECTE,
    UNI_S3MEFFECTF,
    UNI_S3MEFFECTI,
    UNI_S3MEFFECTQ,
    UNI_S3MEFFECTT,
    UNI_XMEFFECTA,
    UNI_XMEFFECTG,
    UNI_XMEFFECTH,
    UNI_XMEFFECTP
};


/**************************************************************************
****** mikmod types: ******************************************************
**************************************************************************/


/*
	Sample format flags:
*/

#define SF_16BITS       1
#define SF_SIGNED   	2
#define SF_DELTA        4
#define SF_BIG_ENDIAN	8
#define SF_LOOP         16
#define SF_BIDI         32
#define SF_OWNPAN       64
#define SF_REVERSE		128


/*
	Envelope flags:
*/

#define EF_ON           1
#define EF_SUSTAIN      2
#define EF_LOOP         4


/*
	Unimod flags
*/

#define UF_XMPERIODS    1               /* if set use XM periods/finetuning */
#define UF_LINEAR       2               /* if set use LINEAR periods */


typedef struct ENVPT{
    SWORD pos;
    SWORD val;
} ENVPT;


typedef struct SAMPLE{
    UWORD c2spd;            /* finetune frequency */
    SBYTE transpose;        /* transpose value */
    UBYTE volume;           /* volume 0-64 */
    UBYTE panning;          /* panning */
    ULONG length;           /* length of sample (in samples!) */
    ULONG loopstart;        /* repeat position (relative to start, in samples) */
    ULONG loopend;          /* repeat end */
    UWORD flags;            /* sample format */
    ULONG seekpos;			/* seek position in file */
    char *samplename;       /* name of the sample */
    SWORD handle;           /* sample handle */
} SAMPLE;


typedef struct INSTRUMENT{
    UBYTE numsmp;
    UBYTE samplenumber[96];

    UBYTE volflg;           /* bit 0: on 1: sustain 2: loop */
    UBYTE volpts;
    UBYTE volsus;
    UBYTE volbeg;
    UBYTE volend;
    ENVPT volenv[12];

    UBYTE panflg;           /* bit 0: on 1: sustain 2: loop */
    UBYTE panpts;
    UBYTE pansus;
    UBYTE panbeg;
    UBYTE panend;
    ENVPT panenv[12];

    UBYTE vibtype;
    UBYTE vibsweep;
    UBYTE vibdepth;
    UBYTE vibrate;

    UWORD volfade;
    char  *insname;
    SAMPLE *samples;
} INSTRUMENT;


/*
	MikMod UNImod types:
	====================
*/

typedef struct UNIMOD{
    UBYTE		numchn;			/* number of channels */
    UWORD       numpos;         /* number of positions in this song */
    UWORD		reppos;			/* restart position */
    UWORD       numpat;         /* number of patterns in this song */
    UWORD       numtrk;         /* number of tracks */
    UWORD       numins;         /* number of samples */
    UBYTE       initspeed;      /* */
    UBYTE       inittempo;      /* */
    UBYTE       positions[256]; /* all positions */
    UBYTE       panning[32];  	/* 32 panning positions */
    UBYTE       flags;          /* */
    char       *songname;       /* name of the song */
    char       *modtype;        /* string type of module */
    char       *comment;        /* module comments */
    INSTRUMENT *instruments;    /* all samples */
    UWORD      *patterns;       /* array of PATTERN */
    UWORD      *pattrows;       /* array of number of rows for each pattern */
    UBYTE     **tracks;         /* array of pointers to tracks */
} UNIMOD;


/**************************************************************************
****** Loader stuff: ******************************************************
**************************************************************************/

/*
	loader structure:
*/

typedef struct LOADER{
    struct LOADER *next;
    const char	*type;
    const char 	*version;
    BOOL    (*Init)(void);
    BOOL	(*Test)(void);
    BOOL	(*Load)(void);
    void	(*Cleanup)(void);
} LOADER;


/*
	public loader variables:
*/

extern UNIMOD of;
extern unsigned char *modptr;
extern long modpos, modsize;
extern const UWORD finetune[16];


/*
	main loader prototypes:
*/

//void 	ML_InfoLoader(void);
void 	ML_RegisterLoader(LOADER *ldr);
UNIMOD	*ML_LoadFP(void);
UNIMOD	*ML_LoadFN(void);
void 	ML_Free(UNIMOD *mf);


/*
	other loader prototypes: (used by the loader modules)
*/

//BOOL 	InitTracks(void);
//void 	AddTrack(UBYTE *tr);
BOOL 	ReadComment(UWORD len);
BOOL 	AllocPatterns(void);
BOOL 	AllocTracks(void);
BOOL 	AllocInstruments(void);
BOOL 	AllocSamples(INSTRUMENT *i);
char    *DupStr(char *s,UWORD len);
void 	*MyMalloc(long size);
void 	*MyCalloc(long nitems,long size);


/*
	Declare external loaders:
*/
extern LOADER load_uni;
extern LOADER load_mod;
extern LOADER load_m15;
extern LOADER load_mtm;
extern LOADER load_s3m;
extern LOADER load_stm;
extern LOADER load_ult;
extern LOADER load_xm;


/**************************************************************************
****** Wavload stuff: *****************************************************
**************************************************************************/

//SAMPLE *MW_LoadWavFP(void);
//SAMPLE *MW_LoadWavFN(char *filename);
//void MW_FreeWav(SAMPLE *si);


/**************************************************************************
****** Driver stuff: ******************************************************
**************************************************************************/

/*
	max. number of handles a driver has to provide. (not strict)
*/

#define MAXSAMPLEHANDLES 128


/*
	possible mixing mode bits:
*/

#define DMODE_STEREO    1
#define DMODE_16BITS    2
#define DMODE_INTERP	4


/*
	4/18/97 NOT USED ANYMORE. NOW DIRECT CONNECTION
	driver structure:
*/

/*
typedef struct DRIVER{
//	struct DRIVER *next;
//	const char    *Name;
//	const char    *Version;
//	BOOL    (*IsPresent)            (void);
//	SWORD   (*SampleLoad)           (ULONG size,ULONG reppos,ULONG repend,UWORD flags);
//	void    (*SampleUnLoad)         (SWORD handle);
	BOOL    (*Init)                 (void);
//	void    (*Exit)                 (void);
//	void    (*PlayStart)            (void);
//	void    (*PlayStop)             (void);
//	void    (*Update)               (void);
//	void 	(*VoiceSetVolume)		(UBYTE voice,UBYTE vol);
//	void 	(*VoiceSetFrequency)	(UBYTE voice,ULONG frq);
//	void 	(*VoiceSetPanning)		(UBYTE voice,UBYTE pan);
//	void	(*VoicePlay)			(UBYTE voice,SWORD handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,UWORD flags);
} DRIVER;

extern DRIVER *md_driver;
*/
/*
	public driver variables:
*/


extern UWORD md_device;
extern UWORD md_mixfreq;
extern UWORD md_mode;
extern UBYTE md_numchn;
extern UBYTE md_bpm;
extern void (*md_player)(void);

/*
	main driver prototypes:
*/

void MD_InfoDriver(void);
//void MD_RegisterDriver(DRIVER *drv);
void MD_RegisterPlayer(void (*plr)(void));
SWORD MD_SampleLoad(ULONG size,ULONG reppos,ULONG repend,UWORD flags);
void MD_SampleUnLoad(SWORD handle);
BOOL MD_Init(void);
void MD_Exit(void);
void MD_PlayStart(void);
void MD_PlayStop(void);
void MD_SetBPM(UBYTE bpm);
void MD_Update(void);
void MD_VoiceSetVolume(UBYTE voice,UBYTE ivol);
void MD_VoiceSetFrequency(UBYTE voice,ULONG frq);
void MD_VoiceSetPanning(UBYTE voice,ULONG pan);
void MD_VoicePlay(UBYTE voice,SWORD handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,UWORD flags);
void SL_Init(UWORD infmt,UWORD outfmt);
void SL_Load(void *buffer,ULONG length);

/*
	Declare external drivers:
*/

//extern DRIVER drv_mac;          /* timing driver */

/**************************************************************************
****** Player stuff: ******************************************************
**************************************************************************/


typedef struct ENVPR{
    UBYTE flg;          /* envelope flag */
    UBYTE pts;			/* number of envelope points */
    UBYTE sus;			/* envelope sustain index */
    UBYTE beg;			/* envelope loop begin */
    UBYTE end;			/* envelope loop end */
    SWORD p;			/* current envelope counter */
    UWORD a;			/* envelope index a */
    UWORD b;			/* envelope index b */
    ENVPT *env;			/* envelope points */
} ENVPR;


typedef struct AUDTMP{
    INSTRUMENT 	*i;
    SAMPLE      *s;

    UWORD fadevol;          /* fading volume */

    ENVPR venv;
    ENVPR penv;

    UBYTE keyon;		/* if true=key is pressed. */
    UBYTE kick;			/* if true=sample has to be restarted */
    UBYTE sample;		/* which sample number (0-31) */
    SWORD handle;		/* which sample-handle */

    ULONG start;		/* The start byte index in the sample */

    UBYTE panning;		/* panning position */
    UBYTE pansspd;		/* panslide speed */

    SBYTE volume;		/* amiga volume (0 t/m 64) to play the sample at */
    UWORD period;		/* period to play the sample at */

    /* You should not have to use the values
       below in the player routine */

    SBYTE transpose;

    UBYTE note;			/* */

    SWORD ownper;
    SWORD ownvol;

    UBYTE *row;			/* row currently playing on this channel */

    SBYTE retrig;		/* retrig value (0 means don't retrig) */
    UWORD c2spd;		/* what finetune to use */

    SBYTE tmpvolume;	/* tmp volume */

    UWORD tmpperiod;	/* tmp period */
    UWORD wantedperiod;	/* period to slide to (with effect 3 or 5) */

    UWORD slidespeed;	/* */
    UWORD portspeed;	/* noteslide speed (toneportamento) */

    UBYTE s3mtremor;	/* s3m tremor (effect I) counter */
    UBYTE s3mtronof;	/* s3m tremor ontime/offtime */

    UBYTE s3mvolslide;	/* last used volslide */

    UBYTE s3mrtgspeed;	/* last used retrig speed */
    UBYTE s3mrtgslide;	/* last used retrig slide */

    UBYTE glissando;	/* glissando (0 means off) */
    UBYTE wavecontrol;	/* */

    SBYTE vibpos;		/* current vibrato position */
    UBYTE vibspd;		/* "" speed */
    UBYTE vibdepth;		/* "" depth */

    SBYTE trmpos;		/* current tremolo position */
    UBYTE trmspd;		/* "" speed */
    UBYTE trmdepth;		/* "" depth */

    UWORD soffset;		/* last used sample-offset (effect 9) */
} AUDTMP;


extern AUDTMP mp_audio[32];		/* max eight channels */
extern UBYTE  mp_bpm;			/* beats-per-minute speed */
extern UWORD  mp_patpos;		/* current row number (0-63) */
extern SWORD  mp_sngpos;		/* current song position */
extern UWORD  mp_sngspd;		/* current songspeed */

extern BOOL  mp_loop;
extern BOOL  mp_panning;
extern BOOL  mp_extspd;
extern UBYTE mp_volume;
extern BOOL	gModIsPlaying;

/*
	player prototypes:
*/

int	 MP_Ready(void);
void MP_NextPosition(void);
void MP_PrevPosition(void);
void MP_SetPosition(UWORD pos);
void MP_HandleTick(void);
void MP_Init(UNIMOD *m);


/**************************************************************************
****** Virtual channel stuff: *********************************************
**************************************************************************/

BOOL    VC_Init(void);
void    VC_Exit(void);

void    VC_PlayStart(void);
void    VC_PlayStop(void);

SWORD   VC_SampleLoad(ULONG size,ULONG reppos,ULONG repend,UWORD flags);
void    VC_SampleUnload(SWORD handle);

void    VC_WriteSamples(SBYTE *buf,UWORD todo);
UWORD   VC_WriteBytes(SBYTE *buf,UWORD todo);
void    VC_SilenceBytes(SBYTE *buf,UWORD todo);

void 	VC_VoiceSetVolume(UBYTE voice,UBYTE vol);
void 	VC_VoiceSetFrequency(UBYTE voice,ULONG frq);
void 	VC_VoiceSetPanning(UBYTE voice,UBYTE pan);
void 	VC_VoicePlay(UBYTE voice,SWORD handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,UWORD flags);
#endif
