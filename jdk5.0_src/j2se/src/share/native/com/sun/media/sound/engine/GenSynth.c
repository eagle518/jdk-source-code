/*
 * @(#)GenSynth.c	1.32 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSynth.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	General purpose Music Synthesis software, C-only implementation
**	(no assembly language optimizations made)
**
** Modification History:
**
**  6/7/93 		begin real serious work
**  6/8/93 		make a stab at getting MIDI performance to work
** 	8/17/93		improved note prioritization
**	8/18/93 	Implement complete API; do interpolated versions of loops and amp. scaling
**	8/21/93		Fixed bug with NoteDur processing samples after note has released
**	8/21/93		Fixed bug with NoteNextSize not being setup correctly inside of ServeMidiNote
**	8/22/93		Incorporated Windows changes for Microsoft C++ Compiler
**	8/23/93		Added new parameters to InitGeneralSound
**	8/24/93		Added even more new parameters to InitGeneralSound
**	8/24/93		Verified Cross platform compiled code
**	10/12/94	fixed pitch tables
**	11/7/95		Major changes, revised just about everything.
**	11/14/95	Fixed volume scale problem. Forgot to scale volume based upon song level
**	11/20/95	Removed the BF_ flags, now you must walk through the union structure
**				Forgot to set songVolume
**				Remove bit fields. BIT FIELDS DON'T WORK WITH MPW!!!!
**	11/24/95	Touched up GM_EndSample & GM_IsSoundDone & GM_BeginDoubleBuffer a bit
**   12/95		upgraded mixing bus to 32 bit; improved scaleback resolution; added reverb unit; first pass at volume ADSR
**	12/6/95		Moved function GM_GetAudioSampleFrame into GenSynth.c
**				Added GM_SetReverbType
**	12/7/95		Added reverb enable via channel controlers in ServeMIDINote
**				Removed some INTEL stuff
**				Moved DEFAULT_REVERB_TYPE to GENSND.H
**				Added GM_GetReverbType
**				Added some more behavior for note removal for sustaining notes
**	1/4/96		Added GM_ChangeSampleReverb for sound effects
**				Changed behavior for setting sample volumes
**	1/7/96		Changed GM_BeginDoubleBuffer to use a 32 bit value for volume
**				Changed PV_ScaleVolumeFromChannelAndSong to correct bug with sound effects
**				and song volume
**				Changed GM_BeginDoubleBuffer & GM_BeginSample to support effectsVolume
**	1/10/96		Split bulk of init code into GenSetup.c
**	1/18/96		Spruced up for C++ extra error checking
**				Added MIN_LOOP_SIZE
**	1/21/96		Changed references to 'true' & 'false' to TRUE and FALSE
**	1/28/96		Fixed 32 bit value bug in function ServeMIDINote
**	1/29/96		Added useSampleRate factor for playback of instruments
**	1/30/96		Fixed useSampleRate in ServeMIDINote
**	2/5/96		Removed unused variables. Working towards multiple songs
**	2/11/96		Removed FixedDivide & FixedMultiply. Use XFixedDivide & XFixedMultiply
**	2/13/96		Moved MusicIRQ into its own function, PV_ProcessSequencerEvents, and now called it
**				from ProcessSampleFrame.
**				Renamed MusicIRQ to PV_MusicIRQ;
**	2/18/96		Added pInstrument->panPlacement use in ServeMidiNote
**	3/1/96		Changed static variables to static const
**	3/6/96		Fixed a divide by zero error in PV_ADSRModule
**	4/6/96		Added default velocity translation curve
**	4/20/96		Moved myFixedMultiply into X_API.c and moved all references to XFixedMultiply
**				Moved myFixedDivide into X_API.c and moved all references to XFixedDivide
**	4/21/96		Removed register usage in parameters
**	4/25/96		Removed SwapWord. Use XGetShort instead
**				Added a test for NULL pInstrument in PV_ServeInstrumentCurves
**				Fixed looping sample bug that caused the sample to stop playing after looping for 8.9 minutes!
**	5/19/96		Removed some compiler warnings
**	6/18/96		Changed behavior of GM_EndAllNotes. Now puts playing notes into release mode rather than killing them.
**	7/4/96		Changed font and re tabbed
**				Fixed logLookupTable entry for -3
**	7/5/96		Added GM_KillAllNotes
**	7/8/96		Improved enveloping and wave shaping code
**	7/9/96		Fixed casting problem that created a bug with Windows compilers
**	7/10/96		Fixed reverb unit to be able to be turned off, in ServeMIDINote
**	9/25/96		Added GM_EndSongNotes
**	9/27/96		Added more parameters to ServeMIDINote & StopMIDINote
**	10/1/96		Fixed GM_KillAllNotes & GM_EndSongNotes & GM_EndAllNotes to terminate notes better
**	10/23/96	Added defines for MOD_WHEEL_CONTROL = 'MODW & SAMPLE_NUMBER = 'SAMP'
**	11/9/96		Added GM_KillSongNotes
**	11/18/96	Added more velocity curves
**	12/3/96		Added 2times linear curve
**	12/4/96		Added 2.5 times linear curve
**	12/6/96		Added cool CBM curve
**	12/10/96	Added yet another CBM curve
**	12/15/96	Added controls for DEFAULT_VELOCITY_CURVE
**	12/30/96	Changed copyright
**	1/12/97		Changed maxNormalizedVoices to mixLevel
**	1/16/97		Changed LFORecord to LFORecords
**	1/23/97		Added support for PV_PostFilterStereo
**				Changed PV_CleanNoteEntry to use XSetMemory
**	2/1/97		Fixed StopMIDINote to support note offset and look for notes to turn off
**				based upon the starting offset
**	2/13/97		Added some INLINE's to help code size and performance
**	3/14/97		Fixed GM_GetAudioSampleFrame to display mono output correctly
**	3/16/97		Modified ServeMIDINote to support root key replacement in
**				keysplits
**	3/18/97		Fixed a bug with new root key replacement. I forgot to handle the
**				no split case
**	4/14/97		Changed KeymapSplit to GM_KeymapSplit
**	4/21/97		Modified ServeMIDINote to support volume levels on a per split basis.
**				Fixed a bug in PV_ServeInstrumentCurves. In the LOWPASS_AMOUNT case
**				it multiplied the orignal and lost the original settings. So when
**				midi data changed it would eventually end in 0.
**	6/10/97		Moved MOD output stage to before Midi output stage to allow for reverb
**				of MOD files to be included in mix.
**				Reworked ProcessSampleFrame to seperate clearing of output buffer
**				and mixing of active voices so that we can mix verb into MOD files
**	6/20/97		Added support for new 16 bit drop sample cases
**	7/18/97		Added HAE_BuildMixerSlice & HAE_GetMaxSamplePerSlice
**				Changed ProcessSampleFrame to reference a register version of MusicGlobals,
**				and removed reference to FAR
**	7/22/97		Changed SYNC_BUFFER_TIME to BUFFER_SLICE_TIME
**	7/28/97		Sun removed an extra ; from GetWaveShape
**	8/18/97		Changed X_WIN_HAE to USE_HAE_EXTERNAL_API
**	8/25/97		Commentted out DEBUG_STR
**				Added optional tone code to HAE_BuildBufferSlice
**	10/3/97		Changed PV_CalcScaleBack to use MAX_VOICES constant instead of fixed value
**	10/15/97	Added processingSlice flag to PV_ServeThisInstrument to help with thread issues
**	12/16/97	Moe: removed compiler warnings
**	1/14/98		$$kk: changes to allow a sample to start, loop NoteLoopTarget between the loop points, and then play to the end
**	1/16/98		Modifed StopMIDINote to only stop one note instead of all with the same
**				play/stop criteria
**	1/22/98		Renamed LOWPASS_AMOUNT to LOW_PASS_AMOUNT
**	1/29/98		Put more code wrappers around verb type 8
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/10/98		Fixed problem with PV_ServeThisInstrument that allowed a looping sample
**				to stop looping after time
**	2/11/98		Put code wrappers around functions not used for WebTV
**				Renamed GetWaveShape to PV_GetWaveShape
**	2/20/98		kcr		more fully integrate new variable reverbs and variable chorus
**	2/23/98		Put more wrappers around MONO only code and STEREO only code
**	3/16/98		Changed PV_ServeStereoInstruments & PV_ServeMonoInstruments on the way it
**				handles reverb.
**	4/2/98		MOE: Hacked ServeMidiNote() to set avoidReverb and reverbLevel in
**				such a way that changing reverb type in the middle of a long note
**				kind-of works.
**	5/7/98		Changed ServeMIDINote to correctly set the verb amount so that it fixes
**				the changing of verb in the middle of a note
**	6/18/98		Modified ServeMIDINote to scale the pan from the unmodified pan stored
**				during playback
**	6/29/98		Fixed reverb problem. All verb was enabled, rather than working with
**				the instrument designers choices.
**	7/7/98		Removed usage of reverbIsVariable and now use GM_IsReverbFixed instead.
**	7/28/98		Renamed inst_struct to pInstrument
**	8/5/98		Modified PV_DoubleBufferCallbackAndSwap to do the callback before setting
**				the voice to be finished.
**	8/12/98		Made PV_ModifyVelocityFromCurve public
**	8/13/98		Added to HAE_BuildMixerSlice calculation of pMixer->timeSliceDifference
**				Added GM_GetMixerUsedTime
**	8/14/98		Added GM_GetMixerUsedTimeInPercent
**	11/6/98		Removed noteDecayPref from the GM_Waveform structure and changed various API's
**				to reflect that.
**	11/9/98		Renamed NoteDur to voiceMode. Renamed this_voice to pVoice. Formatted.
**				Changed resonantFilterLookup from a short to a char to save space
**	11/10/98	Removed CLIP macro
**	11/30/98	Added GM_EndSongChannelNotes to support omni mode
**	12/7/98		Changed PV_CleanNoteEntry to preserve voiceMode during clean. Fixed a bug
**				in ServeMIDINote that would ignore sustaining notes.
**	12/22/98	Removed old USE_SEQUENCER flag
**	1/14/99		Fixed a bug in ServeMIDINote in which it ignore the force verb off in
**				the variable verbs
**	3/2/99		Changed NoteRefNum to NoteContext
**	3/4/99		Fixed ServeMIDINote to handle samples with loop points starting at 0.
**	3/5/99		Added threadContext to PV_ProcessSampleFrame and renamed from ProcessSampleFrame
**      03/22/02        $$fb Added resampled samplerate conversion
*/
/*****************************************************************************/
#include "GenSnd.h"
#include "GenPriv.h"

#if USE_HAE_EXTERNAL_API == TRUE
#include "HAE_API.h"
#endif

GM_Mixer * MusicGlobals = NULL;

// Variables - pitch tables

#define ys	97271
#define PTmake(x)		\
			(539*ys)/(x),	\
			(571*ys)/(x),	\
			(605*ys)/(x),	\
			(641*ys)/(x),	\
			(679*ys)/(x),	\
			(719*ys)/(x),	\
			(762*ys)/(x),	\
			(807*ys)/(x),	\
			(855*ys)/(x),	\
			(906*ys)/(x),	\
			(960*ys)/(x),	\
			(1017*ys)/(x)

static const UINT32 majorPitchTable[] =
{
    PTmake	(102400),
    PTmake	(51200),
    PTmake	(25600),	// 0..11
    PTmake	(12800),	// 12..23
    PTmake	(6400),		// 24..35
    PTmake	(3200),		// 36..47
    PTmake	(1600),		// 48..59
    PTmake	(800),		// 60-71: first entry of this table should = $1,0000.
    PTmake	(400),		// 72-83
    PTmake	(200),		// 84-95
    PTmake	(100),		// 96-107
    PTmake	(50),		// 108-119
    PTmake	(25),		// 120-127 ($80-up unused)
    PTmake	(25),		// MPW probably won't handle this the same.  This means
    PTmake	(25),		// divide by 25 then multiply by 2.  Same as divide by 12.5
    PTmake	(25)
};

// Needs to be recomputed if the synthesis time per chunk is ever different than 11 ms.
static const INT32 logLookupTable[] =
{ 0, 1000000, 5000000, 4000000, 3000000, 2500000, 2000000, 1500000, 1000000,
  700000,500000,400000,250000,200000,150000,150000,100000,100000,100000,100000};

static const UINT32 expDecayLookup[] =
{ 65536, 55110, 60678, 62370, 63060, 63592, 63876, 64132, 64286, 64438, 64534, 64634, 64716, 64716, 64830, 64830, 64918,64918, 64986, 64986,	// 0, 50-950 ms
  65040, 65040, 65084, 65084, 65120, 65120, 65152, 65152, 65180, 65180, 65204, 65204, 65224, 65224, 65244, 65244, 65260,65260, 65274, 65274,	// 1000-1950 ms
  65288, 65288, 65296, 65296, 65306, 65306, 65316, 65316, 65326, 65326, 65336, 65336, 65342, 65342, 65350, 65350, 65356,65356, 65364, 65364,	// 2000-2950ms
  65372, 65372, 65376, 65376, 65380, 65380, 65384, 65384, 65388, 65388, 65394, 65394, 65396, 65396, 65400, 65400, 65404,65404, 65408, 65408,	// 3000-3950ms
  65412, 65412, 65414, 65414, 65416, 65416, 65418, 65418, 65422, 65422, 65424, 65424, 65426, 65426, 65430, 65430, 65432,65432, 65434, 65434, 	// 4000ms
  65438, 65438, 65438, 65438, 65440, 65440, 65442, 65442, 65444, 65444, 65446, 65446, 65446, 65446, 65448, 65448, 65450,65450, 65452, 65452, 	// 5000ms
  65454, 65454, 65454, 65454, 65456, 65456, 65456, 65456, 65458, 65458, 65460, 65460, 65460, 65460, 65462, 65462, 65462,65462, 65464, 65464, 	// 6000ms
  65466, 65466, 65466, 65466, 65466, 65466, 65468, 65468, 65468, 65468, 65470, 65470, 65470, 65470, 65470, 65470, 65472,65472, 65472, 65472, 	// 7000ms
  65474, 65474, 65474, 65474, 65474, 65474, 65476, 65476, 65476, 65476, 65478, 65478, 65478, 65478, 65478, 65478, 65480,65480, 65480, 65480, 	// 8000ms
  65482, 65482, 65482, 65482, 65482, 65482, 65482, 65482, 65482, 65482, 65484, 65484, 65484, 65484, 65484, 65484, 65484,65484, 65484, 65484, 	// 9000ms
  65486, 65486, 65486, 65486, 65486, 65486, 65486, 65486, 65486, 65486, 65486, 65486, 65488, 65488, 65488, 65488, 65488,65488, 65488, 65488, 	// 10 seconds
  65490, 65490, 65490, 65490, 65490, 65490, 65490, 65490, 65490, 65490, 65490, 65490, 65492, 65492, 65492, 65492, 65492,65492, 65492, 65492, 	// 11 seconds
  65494, 65494, 65494, 65494, 65494, 65494, 65494, 65494, 65494, 65494, 65496, 65496, 65496, 65496, 65496, 65496, 65496,65496, 65496, 65496, 	// 12 seconds
  65498, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65500, 65500, 65500, 65500, 65500,65500, 65500, 65500, 	// 13 seconds
  65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502,65502, 65502, 65502,	// 14 seconds
  65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504,65504, 65504, 65504 	// 15 seconds
};

static const UINT32 fractionalPitchTable[] =
{
    65536,  65566,  65595,  65625,  65654,  65684,  65714,  65743,
    65773,  65803,  65832,  65862,  65892,  65922,  65951,  65981,
    66011,  66041,  66071,  66100,  66130,  66160,  66190,  66220,
    66250,  66280,  66309,  66339,  66369,  66399,  66429,  66459,
    66489,  66519,  66549,  66579,  66609,  66639,  66670,  66700,
    66730,  66760,  66790,  66820,  66850,  66880,  66911,  66941,
    66971,  67001,  67032,  67062,  67092,  67122,  67153,  67183,
    67213,  67244,  67274,  67304,  67335,  67365,  67395,  67426,
    67456,  67487,  67517,  67548,  67578,  67609,  67639,  67670,
    67700,  67731,  67761,  67792,  67823,  67853,  67884,  67915,
    67945,  67976,  68007,  68037,  68068,  68099,  68129,  68160,
    68191,  68222,  68252,  68283,  68314,  68345,  68376,  68407,
    68438,  68468,  68499,  68530,  68561,  68592,  68623,  68654,
    68685,  68716,  68747,  68778,  68809,  68840,  68871,  68902,
    68933,  68965,  68996,  69027,  69058,  69089,  69120,  69152,
    69183,  69214,  69245,  69276,  69308,  69339,  69370,  69402
};



/*
static const UBYTE defaultVolumeScale[] = {
// Subtle curve that ends into zero
127, 125, 123, 121, 119, 117, 115, 113, 111, 109, 108, 106, 104, 102, 101, 99, 97,
96, 94, 93, 91, 90, 88, 87, 85, 84, 82, 81, 80, 78, 77, 76, 75, 73, 72, 71, 70, 69,
67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 53, 52, 51, 50, 49, 48,
48, 47, 46, 45, 44, 44, 43, 42, 42, 41, 40, 40, 39, 38, 38, 37, 36, 36, 35, 35, 34,
33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24,
24, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17,
17, 16, 16, 16, 16, 0
};

static const UBYTE defaultVlumeScale[] = {
// harsh curve that ends into zero
127, 124, 121, 118, 115, 112, 109, 106, 104, 101, 98, 96, 93, 91, 89, 87, 84, 82, 80,
78, 76, 74, 72, 71, 69, 67, 65, 64, 62, 60, 59, 57, 56, 55, 53, 52, 51, 49, 48, 47, 46,
44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 34, 33, 32, 31, 30, 29, 29, 28, 27, 27, 26,
25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 16, 15, 15,
14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8,
8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 5, 5, 3, 1, 0
};
*/

/*
static const unsigned short defaultVolumeScale[] = {
// new more agressive curve
296,286,276,267,258,249,241,233,225,217,210,202,196,189,182,176,170,164,159,
153,148,143,138,133,129,124,120,116,112,108,104,101,97,94,90,87,84,81,78,76,
73,70,68,66,63,61,59,57,55,53,51,49,47,46,44,42,41,39,38,36,35,34,32,31,30,
29,28,27,26,25,24,23,22,21,20,19,19,18,17,16,16,15,14,14,13,13,12,12,11,
11,10,10,9,9,8,8,7,7,7,6,6,6,5,5,5,4,4,4,4,3,3,3,3,2,2,2,2,2,
1,1,1,1,1,1,0,0,0,0
};
*/

/*
static const unsigned short defaultVolumeScale[] = {
// new more agressive curve
517, 498, 480, 462, 445, 429, 413, 398, 384, 370, 356,
343, 331, 319, 307, 296, 285, 275, 265, 256, 246, 238, 229, 221, 213,
205, 198, 191, 184, 178, 171, 165, 160, 154, 149, 143, 138, 134, 129,
125, 120, 116, 112, 108, 105, 101, 98, 94, 91, 88, 85, 83, 80, 77,
75, 72, 70, 68, 66, 63, 61, 60, 58, 56, 54, 53, 51, 49, 48, 47, 45,
44, 43, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 31, 30, 29, 28, 28,
27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 20, 19,
19, 19, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15,
15, 14, 14, 14, 0
};
*/

/*
static const unsigned short defaultVolumeScale[] = {
// new more agressive curve
329, 318, 307, 297, 288, 278, 269, 260, 252, 244, 236,
228, 221, 214, 207, 200, 194, 187, 181, 176, 170, 165, 159, 154, 149, 145,
140, 136, 131, 127, 123, 119, 116, 112, 109, 105, 102, 99, 96, 93, 90, 88,
85, 82, 80, 78, 75, 73, 71, 69, 67, 65, 63, 61, 60, 58, 56, 55, 53, 52, 50,
49, 48, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 32, 31,
30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21,
21, 21, 20, 20, 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16,
16, 16, 15, 15, 15, 15, 15, 15, 14, 14, 14, 0
};
*/

/*
static const unsigned short defaultVolumeScale[] = {
// new more agressive curve
302, 292, 282, 273, 264, 255, 247, 239, 231, 223, 216, 208, 202, 195, 188,
182, 176, 170, 165, 159, 154, 149, 144, 139, 135, 130, 126, 122, 118, 114,
110, 107, 103, 100, 96, 93, 90, 87, 84, 82, 79, 76, 74, 72, 69, 67, 65,
63, 61, 59, 57, 55, 53, 52, 50, 48, 47, 45, 44, 42, 41, 40, 38, 37, 36,
35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 25, 24, 23, 22, 22, 21, 20,
20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 13, 12, 12,
12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7,
7, 7, 6, 6, 6, 6
};
*/

/*
static const unsigned short defaultVolumeScale[] = {
// 2.5 times linear curve
318, 315, 313, 310, 308, 305, 303, 300, 298, 295, 293, 290, 288, 285,
283, 280, 278, 275, 273, 270, 268, 265, 263, 260, 258, 255, 253, 250,
248, 245, 243, 240, 238, 235, 233, 230, 228, 225, 223, 220, 218, 215,
213, 210, 208, 205, 203, 200, 198, 195, 193, 190, 188, 185, 183, 180,
178, 175, 173, 170, 168, 165, 163, 160, 158, 155, 153, 150, 148, 145,
143, 140, 138, 135, 133, 130, 128, 125, 123, 120, 118, 115, 113, 110,
108, 105, 103, 100, 98, 95, 93, 90, 88, 85, 83, 80, 78, 75, 73, 70, 68,
65, 63, 60, 58, 55, 53, 50, 48, 45, 43, 40, 38, 35, 33, 30, 28, 25, 23,
20, 18, 15, 13, 10, 8, 5, 3, 0
};
*/

/*
static const unsigned short defaultVolumeScale[] = {
// 2.2 times linear curve
279, 277, 275, 273, 271, 268, 266, 264, 262, 260, 257, 255, 253, 251,
249, 246, 244, 242, 240, 238, 235, 233, 231, 229, 227, 224, 222, 220,
218, 216, 213, 211, 209, 207, 205, 202, 200, 198, 196, 194, 191, 189,
187, 185, 183, 180, 178, 176, 174, 172, 169, 167, 165, 163, 161, 158,
156, 154, 152, 150, 147, 145, 143, 141, 139, 136, 134, 132, 130, 128,
125, 123, 121, 119, 117, 114, 112, 110, 108, 106, 103, 101, 99, 97, 95,
92, 90, 88, 86, 84, 81, 79, 77, 75, 73, 70, 68, 66, 64, 62, 59, 57, 55,
53, 51, 48, 46, 44, 42, 40, 37, 35, 33, 31, 29, 26, 24, 22, 20, 18, 15,
13, 11, 9, 7, 4, 2, 0
};
*/


static const UBYTE defaultVolumeScale[] = {
    // cool S curve version 2
    252, 248, 246, 242, 240, 238, 232, 230, 228, 222,
    220, 218, 214, 212, 208, 204, 200, 198, 194, 190,
    190, 184, 182, 180, 174, 172, 170, 166, 164, 160,
    158, 154, 150, 148, 144, 144, 142, 138, 138, 136,
    132, 130, 128, 126, 124, 122, 120, 118, 116, 114,
    112, 110, 108, 106, 104, 102, 98, 98, 94, 92,
    92, 88, 86, 86, 82, 80, 78, 74, 74, 72,
    68, 68, 66, 62, 62, 62, 58, 58, 56, 54,
    54, 54, 50, 50, 48, 46, 46, 46, 42, 42,
    40, 38, 38, 38, 34, 34, 32, 30, 30, 30,
    26, 26, 24, 22, 22, 22, 20, 18, 18, 16,
    16, 16, 12, 12, 12, 10, 10, 10, 8, 8,
    8, 4, 4, 4, 2, 2, 1, 0
};

static const UBYTE volumeScaleSCurveOriginal[] = {
    // cool S Curve version 1
    254, 252, 252, 252, 252, 252, 252, 250, 250, 250, 250, 248, 248, 248, 246, 246,
    246, 246, 244, 244, 244, 244, 242, 242, 242, 242, 240, 240, 240, 238, 238, 238,
    238, 236, 236, 236, 236, 234, 234, 232, 230, 228, 226, 224, 224, 222, 220, 218,
    216, 214, 212, 210, 210, 208, 206, 204, 202, 200, 198, 196, 192, 190, 188, 186,
    184, 182, 180, 176, 174, 172, 170, 168, 164, 162, 158, 154, 152, 148, 144, 142,
    138, 136, 132, 128, 122, 118, 114, 108, 104, 100, 94, 90, 86, 82, 76, 72, 68, 62,
    58, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20,
    18, 16, 14, 12, 10, 8, 6, 4, 2, 0
};

static const UBYTE volumeScaleTwoTimes[] = {
    // two times linear curve
    254, 252, 250, 248, 246, 244, 242, 240, 238, 236, 234, 232, 230, 228, 226,
    224, 222, 220, 218, 216, 214, 212, 210, 208, 206, 204, 202, 200, 198, 196,
    194, 192, 190, 188, 186, 184, 182, 180, 178, 176, 174, 172, 170, 168, 166,
    164, 162, 160, 158, 156, 154, 152, 150, 148, 146, 144, 142, 140, 138, 136,
    134, 132, 130, 128, 126, 124, 122, 120, 118, 116, 114, 112, 110, 108, 106,
    104, 102, 100, 98, 96, 94, 92, 90, 88, 86, 84, 82, 80, 78, 76, 74, 72, 70,
    68, 66, 64, 62, 60, 58, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32,
    30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
};


static const UBYTE volumeScaleSubtle[] = {
    // Subtle curve that is above zero
    // This is the default velocity curve for WebTV
    254, 248, 243, 238, 232, 227, 222, 217, 213, 208, 203, 199, 194, 190, 186, 182, 178, 174,
    170, 166, 163, 159, 156, 152, 149, 146, 142, 139, 136, 133, 130, 127, 125, 122, 119, 117,
    114, 112, 109, 107, 104, 102, 100, 98, 96, 93, 91, 89, 87, 85, 84, 82, 80, 78, 76, 75,
    73, 72, 70, 68, 67, 65, 64, 63, 61, 60, 59, 57, 56, 55, 54, 52, 51, 50, 49, 48, 47, 46,
    45, 44, 43, 42, 41, 40, 39, 38, 38, 37, 36, 35, 34, 34, 33, 32, 31, 31, 30, 29, 29, 28,
    28, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 18, 17,
    17, 17, 16, 16, 15, 0
};

static const UBYTE volumeScaleTwoTimesExp[] = {
    // Rev Exp x2
    255, 250, 246, 242, 240, 236, 232, 228, 224, 220,
    216, 212, 210, 206, 202, 198, 194, 190, 186, 182,
    180, 176, 172, 168, 164, 160, 156, 152, 150, 146,
    142, 138, 134, 130, 128, 126, 124, 122, 120, 118,
    116, 112, 110, 108, 106, 104, 102, 100, 98, 96,
    94, 92, 90, 88, 86, 84, 82, 78, 76, 74,
    72, 70, 68, 66, 64, 62, 62, 60, 60, 58,
    58, 56, 56, 54, 54, 52, 52, 50, 50, 48,
    48, 46, 46, 44, 44, 42, 42, 40, 40, 38,
    38, 36, 36, 34, 34, 32, 32, 30, 30, 28,
    28, 26, 26, 24, 24, 22, 22, 20, 20, 18,
    18, 16, 16, 14, 14, 12, 12, 10, 10, 8,
    8, 6, 6, 4, 4, 2, 2, 0
};


INT32 PV_ModifyVelocityFromCurve(GM_Song *pSong, INT32 volume)
{
    volume = 127L - (volume & 0x7FL);		// restrict to 0-127 and reverse
    switch (pSong->velocityCurveType)
	{
	default:		// just in case its out of range
	case 0:	// default S curve
	    volume = defaultVolumeScale[volume];
	    break;
	case 1:	// more peaky S curve
	    volume = volumeScaleSCurveOriginal[volume];
	    break;
	case 2:	// WebTV curve
	    volume = volumeScaleSubtle[volume];
	    break;
	case 3:	// two time exponential
	    volume = volumeScaleTwoTimesExp[volume];
	    break;
	case 4:	// two times linear
	    volume = volumeScaleTwoTimes[volume];
	    break;
	}
    return volume;
}

void PV_DoCallBack(GM_Voice *this_one, void *threadContext)
{
    /* $$fb 2003-03-14: add VOICE_REFERENCE to callback. Fix for 4828556 */
    VOICE_REFERENCE ref;
    if (this_one->NoteEndCallback) {
    	//printf("PV_DoCallBack(voice=%x)\n", (int) this_one); fflush(stdout);
	ref = PV_GetSoundReferenceFromVoice(this_one);
	(*this_one->NoteEndCallback)(ref, this_one->NoteContext, threadContext);
    }
    /* $$fb 2002-03-17: Clean up resample code */
    GM_SetSampleResampleFromVoice(this_one, FALSE);
}

void PV_CleanNoteEntry(GM_Voice * the_entry)
{
    VoiceMode	mode;

    mode = the_entry->voiceMode;
    XSetMemory((char *)the_entry, (INT32)sizeof(GM_Voice), 0);
    the_entry->voiceMode = mode;
}



// Compute scale back amplification factors. Used to amplify and scale the processed audio frame.
//
//	Relies upon:
//	MusicGlobals->MaxNotes
//	MusicGlobals->MaxEffects
//	MusicGlobals->MaxNormNotes
//	MusicGlobals->MasterVolume
//
void PV_CalcScaleBack(void)
{
    register INT32	scaleSize;

    scaleSize = (MusicGlobals->MaxNotes*UPSCALAR + MusicGlobals->MaxEffects*UPSCALAR) << 8;
    if (MusicGlobals->mixLevel <= MAX_VOICES)
	{
	    scaleSize = scaleSize / (MusicGlobals->mixLevel*UPSCALAR) * MusicGlobals->MasterVolume;
	}
    else
	{
	    scaleSize = scaleSize * 100 / (MusicGlobals->mixLevel*UPSCALAR) * MusicGlobals->MasterVolume;
	}

    scaleSize = scaleSize >> 8;

    MusicGlobals->scaleBackAmount = (scaleSize << 8) / (MusicGlobals->MaxNotes*UPSCALAR+MusicGlobals->MaxEffects*UPSCALAR);

}


// used by macro THE_CHECK. This mainly used by double buffered audio clips
INT32 PV_DoubleBufferCallbackAndSwap(GM_DoubleBufferCallbackPtr doubleBufferCallback,
				     GM_Voice *pVoice)
{
    INT32	bufferSize;

    bufferSize = (INT32) (pVoice->NotePtrEnd - pVoice->NotePtr);
    // we hit the end of the loop call double buffer to notify swap
    (*doubleBufferCallback)(pVoice->NoteContext, pVoice->NotePtr, &bufferSize);
#ifdef DEBUG
    printf("Got filled buffer: %d \n", (int) bufferSize);
#endif
    // now we swap pointers
    if (bufferSize)
	{
	    if (pVoice->NotePtr == pVoice->doubleBufferPtr1)
		{
		    pVoice->NotePtr = pVoice->doubleBufferPtr2;
		    pVoice->NotePtrEnd = pVoice->doubleBufferPtr2 + bufferSize;
		}
	    else
		{
		    pVoice->NotePtr = pVoice->doubleBufferPtr1;
		    pVoice->NotePtrEnd = pVoice->doubleBufferPtr1 + bufferSize;
		}

	    pVoice->NoteLoopPtr = pVoice->NotePtr;
	    pVoice->NoteLoopEnd = pVoice->NotePtrEnd;
	    pVoice->voiceMode = VOICE_SUSTAINING;			// reset durations otherwise the voice will shut down
	    pVoice->NoteDecay = 0x7FFF;
	}
    else
	{
	    // $$kk: 04.19.99
	    //PV_DoCallBack(pVoice);
	    PV_DoCallBack(pVoice, NULL);
	    pVoice->voiceMode = VOICE_UNUSED;
	}
    return bufferSize;
}

XFIXED PV_GetWavePitch(XFIXED notePitch)
{
    switch (MusicGlobals->outputQuality)
	{
	case Q_48K:
	    // If performing at 48 khz then the pitch rates must be 1/8 * 1.0909
	    return XFixedDivide(notePitch, 2287802L);
	    //			return XFixedDivide(notePitch, FLOAT_TO_XFIXED(34.909090909));
	case Q_24K:
	    // If performing at 24 khz then the pitch rates must be 1/8 * 1.0909 / 2
	    return XFixedDivide(notePitch, 1143901L);	// 17.5
	    //			return XFixedDivide(notePitch, FLOAT_TO_XFIXED(17.454545455));	// 17.5
	case Q_44K:
	    // If performing at 44 khz then the pitch rates must be 1/8
	    return notePitch >> (17 - STEP_BIT_RANGE);
	case Q_22K:
	case Q_22K_TERP_44K:
	    // if performing at 22 khz then pitches must be 1/4 for the 24.8 fixed values
	    return notePitch >> (16 - STEP_BIT_RANGE);
	case Q_11K:
	case Q_11K_TERP_22K:
	    // If performing at 11 khz then the pitch rates must be 1/2
	    return notePitch >>  (15 - STEP_BIT_RANGE);
	case Q_8K:
	    // If performing at 8 khz then the pitch rates must be 1/2
	    //(15 - STEP_BIT_RANGE) = 8 * .72727272727
	    return XFixedDivide(notePitch, 381300L);
	    //			return XFixedDivide(notePitch, FLOAT_TO_XFIXED(5.8181818182));
	}
    // error,  return same pitch
    return notePitch;
}

// ------------------------------------------------------------------------------------------------------//



// Generic ADSR Unit
static void PV_ADSRModule(ADSRRecord *a, INT32 sustaining)
{
    INT32 currentTime = a->currentTime;
    INT32 index =  a->currentPosition;
    INT32 scalar, i;

    // Find the release or LAST marker when the note is being turned off.

    if ((!sustaining) && (a->mode != ADSR_RELEASE) && (a->mode != ADSR_TERMINATE))
	{
	    for (i = 0; i < ADSR_STAGES; i++)
		{
		    scalar = a->ADSRFlags[i];
		    if ((scalar == ADSR_RELEASE) || (scalar == ADSR_TERMINATE))
			{
			    index = i;
			    a->previousTarget = a->currentLevel;
			    goto foundRelease;
			}
		    if (scalar == ADSR_SUSTAIN)
			{
			    index = i+1;
			    a->previousTarget = a->currentLevel;
			    goto foundRelease;
			}

		}
	foundRelease:
	    currentTime = 0;
	    a->mode = ADSR_RELEASE;
	}

    switch (a->ADSRFlags[index])
	{
	case ADSR_SUSTAIN:
	    a->mode = ADSR_SUSTAIN;
	    if (a->ADSRLevel[index] < 0)
		{
		    if (a->ADSRLevel[index] < -50)
			a->sustainingDecayLevel = ( a->sustainingDecayLevel * (expDecayLookup[-a->ADSRLevel[index]/50000L] >> 1L)  ) >> 15L;
		    else
			a->sustainingDecayLevel = ( a->sustainingDecayLevel * (expDecayLookup[logLookupTable[-a->ADSRLevel[index]]/50000L] >> 1L)  ) >> 15L;
		}
	    else
		{
		    if (currentTime)
			{
			    currentTime += HAE_GetSliceTimeInMicroseconds() - 610;		// microseconds;
			    if (currentTime >= a->ADSRTime[index])
				currentTime = a->ADSRTime[index];
			    if (a->ADSRTime[index] >> 6L)
				{
				    scalar = (currentTime << 6) / (a->ADSRTime[index] >> 6);	// scalar is 0..4095
				}
			    else
				{
				    scalar = 0;
				}
			    a->currentLevel = a->previousTarget + (((a->ADSRLevel[index] - a->previousTarget) * scalar) >> 12L);
			}
		}
	    break;
	default:
	    currentTime += HAE_GetSliceTimeInMicroseconds() - 610;		// microseconds;
	    if (currentTime >= a->ADSRTime[index])
		{
		    a->previousTarget = a->ADSRLevel[index];
		    a->currentLevel = a->ADSRLevel[index];
		    currentTime -=  a->ADSRTime[index];
		    if (a->ADSRFlags[index] != ADSR_TERMINATE)
			index++;
		    else
			{
			    a->mode = ADSR_TERMINATE;
			    currentTime -= HAE_GetSliceTimeInMicroseconds() - 610;		// prevent long note times from overflowing if they stay on for more than 32.767 seconds
			}
		}
	    else
		{
		    if (currentTime)
			{
			    scalar = (currentTime << 6) / (a->ADSRTime[index] >> 6);	// scalar is 0..4095
			    a->currentLevel = a->previousTarget + (((a->ADSRLevel[index] - a->previousTarget) * scalar) >> 12);	// ADSRLevels max out at 64k
			}
		}
	    break;
	}

    // the index may have changed, so check for new cases.
#if 0
    switch (a->ADSRFlags[index])
	{
	case ADSR_GOTO:
	    index = a->ADSRLevel[index] - 1;
	    break;
	}
#endif

    a->currentTime = currentTime;
    a->currentPosition = index & 7;	// protect against runaway indexes
}

static INLINE INT32 PV_GetWaveShape (INT32 where, INT32 what_kind)
{
    switch (what_kind)
	{
	case SINE_WAVE:
	    if (where > 32768)
		return ((65536- where)* 4) - 65536;
	    else
		return (where * 4) - 65536;
	case SAWTOOTH_WAVE:
	    return (32768 - where) * 2;
	case SAWTOOTH_WAVE2:
	    return (where - 32768) * 2;
	case TRIANGLE_WAVE:
	    if (where > 32768)
		return ((65536- where)* 4) - 65536;
	    else
		return (where * 4) - 65536;
	case SQUARE_WAVE:
	    if (where > 32768)
		return 65536;
	    else
		return -65536;
	case SQUARE_WAVE2:
	    if (where > 32768)
		return 65536;
	    else
		return 0;
	default:
	    if (where > 32768)
		return ((65536- where)* 4) - 65536;
	    else
		return (where * 4) - 65536;
	}
}

static const char resonantFilterLookup[] =
{
    42, 40, 37, 35, 33, 63, 59, 56, 53,
    50, 47, 45, 42, 40, 37, 35, 33, 63, 59, 56, 53,
    50, 47, 45, 42, 40, 37, 35, 33, 32, 30, 28, 27,
    50, 47, 45, 42, 40, 37, 35, 33, 32, 30, 28, 27,
    50, 47, 45, 42, 40, 37, 35, 33, 32, 30, 28, 27,
    25, 24, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13,
    13, 12, 11, 11, 10,  9,  9,  8,  8,  7,  7,  7,
    6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  5,
    5,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static INLINE void PV_ServeInstrumentCurves(GM_Voice *pVoice)
{
    INT32 i, j, count, tieFromValue, scalar;
    GM_Instrument *pInstrument;
    INT32 curveCount;

    if (pVoice->ModWheelValue == pVoice->LastModWheelValue)
	return;
    pVoice->LastModWheelValue = pVoice->ModWheelValue;


    pInstrument = pVoice->pInstrument;

    if ( (pInstrument->curveRecordCount == 0) || (pInstrument == NULL) )
	return;
    for (i = 0; i < pInstrument->curveRecordCount; i++)
	{
	    if (pInstrument->curve[i].tieFrom == MOD_WHEEL_CONTROL)
		{
		    tieFromValue = pVoice->ModWheelValue;
		    curveCount = pInstrument->curve[i].curveCount;		// in case no hits occur
		    scalar = tieFromValue;
		    for (count = 0; count < curveCount; count++)
			{
			    if (pInstrument->curve[i].from_Value[count] <= tieFromValue)
				{
				    if (pInstrument->curve[i].from_Value[count+1] >= tieFromValue)
					{
					    scalar = pInstrument->curve[i].to_Scalar[count];
					    if (pInstrument->curve[i].from_Value[count] != pInstrument->curve[i].from_Value[count+1])
						{
						    INT32 from_difference = pInstrument->curve[i].from_Value[count+1] - pInstrument->curve[i].from_Value[count];
						    INT32 to_difference = pInstrument->curve[i].to_Scalar[count+1] - pInstrument->curve[i].to_Scalar[count];
						    scalar += ((((tieFromValue - pInstrument->curve[i].from_Value[count]) << 8) / from_difference) * to_difference) >> 8;
						}
					}
				}
			}
		    switch (pInstrument->curve[i].tieTo)
			{
			case LPF_FREQUENCY:
			    // pull original frequency and scale it based upon current mod wheel settings
			    pVoice->LPF_base_frequency = (pInstrument->LPF_frequency * scalar) >> 8;
			    break;
			case LOW_PASS_AMOUNT:
			    // pull original lowpassAmount and scale it based upon current mod wheel settings
			    pVoice->LPF_base_lowpassAmount = (pInstrument->LPF_lowpassAmount * scalar) >> 8;
			    break;
			case PITCH_LFO:
			    for (j = pVoice->LFORecordCount - 1; j >= 0; --j)
				{
				    if (pVoice->LFORecords[j].where_to_feed == PITCH_LFO)
					{
					    pVoice->LFORecords[j].level = (pInstrument->LFORecords[j].level * scalar) >> 8;
					    goto done;
					}
				}
			    break;
			case VOLUME_LFO:
			    for (j = pVoice->LFORecordCount - 1; j >= 0; --j)
				{
				    if (pVoice->LFORecords[j].where_to_feed == VOLUME_LFO)
					{
					    pVoice->LFORecords[j].level = (pInstrument->LFORecords[j].level * scalar) >> 8;
					    goto done;
					}
				}
			    break;
			case PITCH_LFO_FREQUENCY:
			    for (j = pVoice->LFORecordCount - 1; j >= 0; --j)
				{
				    if (pVoice->LFORecords[j].where_to_feed == PITCH_LFO)
					{
					    pVoice->LFORecords[j].period = (pInstrument->LFORecords[j].period * scalar) >> 8;
					    goto done;
					}
				}
			    break;
			case VOLUME_LFO_FREQUENCY:
			    for (j = pVoice->LFORecordCount - 1; j >= 0; --j)
				{
				    if (pVoice->LFORecords[j].where_to_feed == VOLUME_LFO)
					{
					    pVoice->LFORecords[j].period = (pInstrument->LFORecords[j].period * scalar) >> 8;
					    goto done;
					}
				}
			    break;
			}
		done:;
		}
	}
}

static void PV_LockInstrumentAndVoice(GM_Voice *pVoice)
{
    pVoice->processingSlice = TRUE;		// processing
    if (pVoice->pInstrument)
	{
	    pVoice->pInstrument->processingSlice = TRUE;
	}
}

static void PV_UnlockInstrumentAndVoice(GM_Voice *pVoice)
{
    pVoice->processingSlice = FALSE;		// done processing
    if (pVoice->pInstrument)
	{
	    pVoice->pInstrument->processingSlice = FALSE;
	}
}

// Process this active voice

// $$kk: 04.19.99
//static void PV_ServeThisInstrument(GM_Voice *pVoice)
static void PV_ServeThisInstrument(GM_Voice *pVoice, void *threadContext)
{
    register UINT32	start, end, loopend, size;
    register INT32			n, i, value;
    register XFIXED			wave_increment;
    LFORecord				*rec;

    if (pVoice->voiceMode == VOICE_ALLOCATED)
	{
	    // this is a special thread case in which we have allocate the voice
	    // but are in the middle of filling out the voice elements prior to
	    // starting.
	    return;
	}
    // lock voice and instrument
    PV_LockInstrumentAndVoice(pVoice);

    // process curves
    PV_ServeInstrumentCurves(pVoice);

    /* Calculate pitch bend before calculating the note's maximum
    ** # of samples that it can play this frame.
    */

    // Get the latest pitchbend controller value, which should be munged into an
    // 8.8 Fixed value for semitones to bend.
    n = pVoice->NotePitchBend;

    //	n += 12*256;	// if we need to bump up sound pitches for testing of advanced filters
    // Process LFO's
    pVoice->volumeLFOValue = 4096;	// default value. Will change below if there's a volume LFO unit present.
    pVoice->stereoPanBend = 0;	// default to no pan modifications.
    pVoice->LPF_resonance = pVoice->LPF_base_resonance;
    pVoice->LPF_lowpassAmount = pVoice->LPF_base_lowpassAmount;
    if (pVoice->LPF_base_frequency <= 0)			// if resonant frequency tied to note pitch, zero out frequency
	pVoice->LPF_frequency = 0;
    if (pVoice->LFORecordCount)
	for (i = 0; i < pVoice->LFORecordCount; i++)
	    {
		rec = &(pVoice->LFORecords[i]);
		//			PV_ADSRModule (&(rec->a), (pVoice->NoteDur > 100) || pVoice->sustainMode == SUS_ON_NOTE_ON);
		PV_ADSRModule (&(rec->a), (pVoice->voiceMode == VOICE_SUSTAINING) || pVoice->sustainMode == SUS_ON_NOTE_ON);
		if ( (rec->level) || (rec->DC_feed) )
		    {
			rec->LFOcurrentTime += HAE_GetSliceTimeInMicroseconds() - 610;
			if (rec->period)
			    {
				if (rec->LFOcurrentTime > rec->period)
				    rec->LFOcurrentTime -= rec->period;
				// Produce a percentage index into the current LFO period, scaled to 0..65536.
				// this calculation maxes out at about 15 seconds
				rec->currentWaveValue = (rec->LFOcurrentTime << 7) / (rec->period >> 9);
				value = (rec->a.currentLevel * rec->level) >> 16;
				//					value = (value * rec->a.sustainingDecayLevel) >> 16;
				value = (value * PV_GetWaveShape(rec->currentWaveValue, rec->waveShape)) >> 16;
			    }
			else
			    {
				value = 0;
			    }
			value += (rec->a.currentLevel * rec->DC_feed) >> 16;
			switch (rec->where_to_feed)
			    {
			    case PITCH_LFO:
				n += value;
				break;
			    case VOLUME_LFO:
				pVoice->volumeLFOValue += value;
				break;
			    case STEREO_PAN_LFO:
			    case STEREO_PAN_NAME2:
				pVoice->stereoPanBend += value;
				break;
			    case LPF_FREQUENCY:
				if (pVoice->LPF_base_frequency <= 0)
				    pVoice->LPF_frequency = value;
				else
				    pVoice->LPF_frequency = pVoice->LPF_base_frequency - value;
				break;
			    case LOW_PASS_AMOUNT:
				pVoice->LPF_lowpassAmount += value;
				break;
			    case LPF_DEPTH:
				pVoice->LPF_resonance += value;
				break;
			    default:
				//DEBUG_STR("\p Invalid LFO Unit Feed-To");
				break;
			    }
		    } // if rec->level
	    } // for ()

    // to test notes with only pitch bend or LFO applied:
    /*
      if (n == 0)
      {
      PV_UnlockInstrumentAndVoice(pVoice);	// done processing
      return;
      }
    */
    // If pitchbend value has changed, recalculate the pitch data for this instrument
    if (pVoice->LPF_base_frequency <= 0)
	pVoice->LPF_frequency =
	    resonantFilterLookup[(n + ((pVoice->NoteMIDIPitch - pVoice->LPF_base_frequency) << 8) + 0x80) >> 8]
	    * 256 + pVoice->LPF_frequency;

    if (n != pVoice->LastPitchBend)
	{
	    pVoice->LastPitchBend = (INT16)n;
	    n += pVoice->ProcessedPitch << 8;		// ProcessedPitch is based on the sample data and MIDI pitch.

	    // Clip value to within reasonable MIDI pitch range
	    if (n < -0x1800)
		n += 0x0c00;
	    if (n < -0x1800)
		n += 0x0c00;
	    if (n < -0x1800)
		n = -0x1800;
	    if (n > 0x08C00)
		n -= 0x0C00;		// 12 (one octave) in 8.8 Fixed form
	    if (n > 0x08C00)
		n -= 0x0C00;		// 12 (one octave) in 8.8 Fixed form
	    if (n > 0x08C00)
		n -= 0x0C00;		// 12 (one octave) in 8.8 Fixed form
	    if (n > 0x08C00)
		n = 0x0C00;		// 12 (one octave) in 8.8 Fixed form
	    // Process whole pitch value in semitones
	    pVoice->NotePitch = majorPitchTable[(n >> 8) + 24];
	    // Process fractional semitone values
	    pVoice->NotePitch = XFixedMultiply(pVoice->NotePitch, fractionalPitchTable[(n & 0xFF) >> 1]);
	    // factor in sample rate of sample, if enabled
	    pVoice->NotePitch = XFixedMultiply(pVoice->noteSamplePitchAdjust, pVoice->NotePitch);

	    // Recalculate number of samples in a slice
	    pVoice->NoteNextSize = 0;
	}

    if (pVoice->NoteNextSize == 0)
	{
	    wave_increment = PV_GetWavePitch(pVoice->NotePitch);
	    pVoice->NoteNextSize =
		(INT16)(((wave_increment * (XFIXED)MusicGlobals->One_Slice) >> STEP_BIT_RANGE) + 3);
	}
    size = pVoice->NoteNextSize;
    start = pVoice->NoteWave >> STEP_BIT_RANGE;
    end = (UINT32) (pVoice->NotePtrEnd - pVoice->NotePtr);
    pVoice->NoteNextSize = (INT16)size;

    // This is sure easier than the LFO modules!
    PV_ADSRModule (&(pVoice->volumeADSRRecord), (pVoice->voiceMode == VOICE_SUSTAINING) || pVoice->sustainMode == SUS_ON_NOTE_ON);
    pVoice->NoteVolumeEnvelope = (INT16)(((pVoice->volumeADSRRecord.currentLevel >> 2) *
					  pVoice->volumeADSRRecord.sustainingDecayLevel) >> 14L);
    pVoice->NoteVolumeEnvelopeBeforeLFO = pVoice->NoteVolumeEnvelope;
    if (pVoice->volumeLFOValue >= 0)		// don't handle volume LFO values less than zero.
	{
	    pVoice->NoteVolumeEnvelope = (INT16)((pVoice->NoteVolumeEnvelope * pVoice->volumeLFOValue) >> 12L);
	}

    if (pVoice->NoteLoopEnd)
	{
	    loopend = (UINT32) (pVoice->NoteLoopEnd - pVoice->NotePtr);
	}
    else
	{
	    loopend = 0;
	}

    // At end of loop, continue loop?
    if (start > (loopend - size))
	{
	    //$$kk: 01.14.98:	changes to allow a sample to start, loop NoteLoopTarget between the loop points,
	    //					and then play to the end

	    //		if (pVoice->NoteLoopCount == 0)
	    // we can keep looping if the NoteLoopTarget is set to a valid value (>0) and we haven't reached it yet.
	    // if it's set to <= 0, we loop continuously unless the NoteLoopProc tells us to quit.
	    if ((pVoice->NoteLoopCount < pVoice->NoteLoopTarget) || (pVoice->NoteLoopTarget <= 0))
		{
		    pVoice->NoteLoopCount++;
		    if (pVoice->NoteLoopProc)
			{
				// continue loop?

			    if ((*pVoice->NoteLoopProc)(pVoice->NoteContext, threadContext) == FALSE)
				{
				    // nope
				    pVoice->NoteLoopProc = NULL;
				    pVoice->NoteLoopPtr = NULL;
				    pVoice->NoteLoopEnd = NULL;
				    pVoice->voiceMode = VOICE_SUSTAINING;		// let rest of sample play out
				    loopend = 0;

				    pVoice->NoteLoopCount = 0;
				}
			    else
				{
				    // yes, so refresh note duration counter, otherwise sample will stop after 8.9 minutes
				    pVoice->voiceMode = VOICE_SUSTAINING;
				}
			} // if (pVoice->NoteLoopProc)
		} // can keep looping
	    else
		// stop looping
		{
		    pVoice->voiceMode = VOICE_SUSTAINING;		// let rest of sample play out
		    loopend = 0;
		}
	}
    /*
      else
      {
      pVoice->NoteLoopCount = 0;
      }
    */

    if ((pVoice->volumeADSRRecord.ADSRTime[0] != 0) || (pVoice->volumeADSRRecord.ADSRFlags[0] != ADSR_TERMINATE) || (pVoice->sampleAndHold != 0) )
	{
	    // New style volume ADSR instruments
	    if (pVoice->voiceMode == VOICE_RELEASING)
		{
		    if (loopend == 0)
			{
			    goto ENDING;		// this case handles sample-and-release or one-shot instruments
			}
		    if ((pVoice->volumeADSRRecord.mode == ADSR_TERMINATE) || (pVoice->volumeADSRRecord.mode == ADSR_RELEASE))
			{
			    if (pVoice->sampleAndHold == 0)
				{
				    goto ENDING;
				}
			}
		    goto LOOPING;		// this case handles sample-and-hold instruments
		}
	    else
		{
		    if (loopend == 0)
			{
			    goto ENDING;
			}
		    else
			{
			    goto LOOPING;
			}
		}
	}
    else
	{
	    // Old style instrument decay
	    if ((pVoice->voiceMode == VOICE_RELEASING) || (loopend == 0))
		{
		    if ((pVoice->voiceMode == VOICE_RELEASING) && (pVoice->NoteDecay == 0))
			{
				// Interpolation of volume ADSR levels will smooth out the transition to note termination.
			    pVoice->NoteVolumeEnvelope = 0;
			    pVoice->NoteVolumeEnvelopeBeforeLFO = 0;
			    goto PARTIAL;
			}
		ENDING:
		    if ( (end - start) <= size)
			{
			PARTIAL:
			    if (((pVoice->LPF_lowpassAmount != 0) || (pVoice->LPF_resonance != 0)) && (pVoice->channels == 1))
				{
				    if (pVoice->bitSize == 16)
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterPartialBufferProc16(pVoice, FALSE);
					    MusicGlobals->filterPartialBufferProc16(pVoice, FALSE, threadContext);
					}
				    else
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterPartialBufferProc(pVoice, FALSE);
					    MusicGlobals->filterPartialBufferProc(pVoice, FALSE, threadContext);
					}
				}
			    else
				/* $$fb 2002-01-06 added resample algorithms */
				if (pVoice->resampleParams != NULL) {
				    if (pVoice->bitSize == 16) {
					MusicGlobals->resamplePartialBufferProc16(pVoice, FALSE, threadContext);
				    } else {
					MusicGlobals->resamplePartialBufferProc(pVoice, FALSE, threadContext);
				    }
				}
				else
				    {
					if (pVoice->bitSize == 16)
					    {
						// $$kk: 04.19.99
						// MusicGlobals->partialBufferProc16(pVoice, FALSE);
						MusicGlobals->partialBufferProc16(pVoice, FALSE, threadContext);
					    }
					else
					    {
						// $$kk: 04.19.99
						// MusicGlobals->partialBufferProc(pVoice, FALSE);
						MusicGlobals->partialBufferProc(pVoice, FALSE, threadContext);
					    }
				    }
			}
		    else
			{
			    if (((pVoice->LPF_lowpassAmount != 0) || (pVoice->LPF_resonance != 0)) && (pVoice->channels == 1))
				{
				    if (pVoice->bitSize == 16)
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterFullBufferProc16(pVoice);
					    MusicGlobals->filterFullBufferProc16(pVoice, threadContext);
					}
				    else
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterFullBufferProc(pVoice);
					    MusicGlobals->filterFullBufferProc(pVoice, threadContext);
					}
				}
			    else
				/* $$fb 2002-01-06 added resample algorithms */
				if (pVoice->resampleParams != NULL) {
				    if (pVoice->bitSize == 16) {
					MusicGlobals->resampleFullBufferProc16(pVoice, threadContext);
				    } else {
					MusicGlobals->resampleFullBufferProc(pVoice, threadContext);
				    }
				}
				else
				    {
					if (pVoice->bitSize == 16)
					    {
						// $$kk: 04.19.99
						// MusicGlobals->fullBufferProc16(pVoice);
						MusicGlobals->fullBufferProc16(pVoice, threadContext);
					    }
					else
					    {
						// $$kk: 04.19.99
						// MusicGlobals->fullBufferProc(pVoice);
						MusicGlobals->fullBufferProc(pVoice, threadContext);
					    }
				    }
			}
		}
	    else
		{
		LOOPING:
		    if (loopend > (start + size) )
			{
			    if (((pVoice->LPF_lowpassAmount != 0) || (pVoice->LPF_resonance != 0)) && (pVoice->channels == 1))
				{
				    if (pVoice->bitSize == 16)
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterFullBufferProc16(pVoice);
					    MusicGlobals->filterFullBufferProc16(pVoice, threadContext);
					}
				    else
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterFullBufferProc(pVoice);
					    MusicGlobals->filterFullBufferProc(pVoice, threadContext);
					}
				}
			    else
				/* $$fb 2002-01-06 added resample algorithms */
				if (pVoice->resampleParams != NULL) {
				    if (pVoice->bitSize == 16) {
					MusicGlobals->resampleFullBufferProc16(pVoice, threadContext);
				    } else {
					MusicGlobals->resampleFullBufferProc(pVoice, threadContext);
				    }
				}
				else
				    {
					if (pVoice->bitSize == 16)
					    {
						// $$kk: 04.19.99
						// MusicGlobals->fullBufferProc16(pVoice);
						MusicGlobals->fullBufferProc16(pVoice, threadContext);
					    }
					else
					    {
						// $$kk: 04.19.99
						// MusicGlobals->fullBufferProc(pVoice);
						MusicGlobals->fullBufferProc(pVoice, threadContext);
					    }
				    }
			}
		    else
			{
			    if (((pVoice->LPF_lowpassAmount != 0) || (pVoice->LPF_resonance != 0)) && (pVoice->channels == 1))
				{
				    if (pVoice->bitSize == 16)
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterPartialBufferProc16(pVoice, TRUE);
					    MusicGlobals->filterPartialBufferProc16(pVoice, TRUE, threadContext);
					}
				    else
					{
					    // $$kk: 04.19.99
					    // MusicGlobals->filterPartialBufferProc(pVoice, TRUE);
					    MusicGlobals->filterPartialBufferProc(pVoice, TRUE, threadContext);
					}
				}
			    else
				/* $$fb 2002-01-06 added resample algorithms */
				if (pVoice->resampleParams != NULL) {
				    if (pVoice->bitSize == 16) {
					MusicGlobals->resamplePartialBufferProc16(pVoice, TRUE, threadContext);
				    }else {
					MusicGlobals->resamplePartialBufferProc(pVoice, TRUE, threadContext);
				    }
				}
				else
				    {
					if (pVoice->bitSize == 16)
					    {
						// $$kk: 04.19.99
						// MusicGlobals->partialBufferProc16(pVoice, TRUE);
						MusicGlobals->partialBufferProc16(pVoice, TRUE, threadContext);
					    }
					else
					    {
						// $$kk: 04.19.99
						// MusicGlobals->partialBufferProc(pVoice, TRUE);
						MusicGlobals->partialBufferProc(pVoice, TRUE, threadContext);
					    }
				    }
			}
		}
	}

    //DONE
    if (pVoice->voiceMode == VOICE_RELEASING)
	{
	    if ((pVoice->volumeADSRRecord.ADSRTime[0] != 0) || (pVoice->volumeADSRRecord.ADSRFlags[0] != ADSR_TERMINATE))
		{
		    // Handle new style volume ADSR's
		    if (pVoice->volumeADSRRecord.mode == ADSR_TERMINATE)
			{
			    if ((pVoice->volumeADSRRecord.currentLevel < 0x100) || (pVoice->volumeADSRRecord.sustainingDecayLevel < 0x100))
				{
				    PV_DoCallBack(pVoice, threadContext);
				    pVoice->voiceMode = VOICE_UNUSED;
				}
			}
		    else
			{
			    if (pVoice->volumeADSRRecord.sustainingDecayLevel == 0)
				{
				    PV_DoCallBack(pVoice, threadContext);
				    pVoice->voiceMode = VOICE_UNUSED;
				}
				// If low in volume, fade it out gracefully next cycle
			    if (pVoice->volumeADSRRecord.sustainingDecayLevel < 0x800)
				{
				    pVoice->volumeADSRRecord.sustainingDecayLevel = 0;
				}
			}
		}
	    else
		{
		    // old style scheme with NoteDecay values and so forth
		    if (pVoice->NoteDecay == 0)
			{
			    PV_DoCallBack(pVoice, threadContext);
			    pVoice->voiceMode = VOICE_UNUSED;
			}
		    else
			{
			    pVoice->NoteDecay--;
			}
		}
	}
    //	else
    //	{
    //		//pVoice->NoteDur--;
    //	}
    PV_UnlockInstrumentAndVoice(pVoice);	// done processing
}

static void PV_ClearReverbBuffer()
{
#if USE_VARIABLE_REVERB != FALSE
    if (GM_IsReverbFixed() == FALSE)
	{
	    register INT32 	*destL = &MusicGlobals->songBufferReverb[0];
	    register LOOPCOUNT	count, four_loop = MusicGlobals->Four_Loop;

	    for (count = 0; count < four_loop; count++)
		{
		    destL[0] = 0;
		    destL[1] = 0;
		    destL[2] = 0;
		    destL[3] = 0;
		    destL += 4;
		}
	}
#endif
}

static void PV_ClearChorusBuffer()
{
#if USE_NEW_EFFECTS
    register INT32 	*destL = &MusicGlobals->songBufferChorus[0];
    register LOOPCOUNT	count, four_loop = MusicGlobals->Four_Loop;

    for (count = 0; count < four_loop; count++)
	{
	    destL[0] = 0;
	    destL[1] = 0;
	    destL[2] = 0;
	    destL[3] = 0;
	    destL += 4;
	}
#endif
}


INLINE static void PV_ClearMixBuffers(XBOOL doStereo)
{
    register INT32 		*destL;
    register LOOPCOUNT	count, four_loop;

    destL = &MusicGlobals->songBufferDry[0];
    four_loop = MusicGlobals->Four_Loop;
    if (doStereo)
	{	// stereo
#if USE_STEREO_OUTPUT == TRUE
	    for (count = 0; count < four_loop; count++)
		{
		    destL[0] = 0;
		    destL[1] = 0;
		    destL[2] = 0;
		    destL[3] = 0;

		    destL[4] = 0;
		    destL[5] = 0;
		    destL[6] = 0;
		    destL[7] = 0;
		    destL += 8;
		}
#endif
	}
    else
	{	// mono
#if USE_MONO_OUTPUT == TRUE
	    for (count = 0; count < four_loop; count++)
		{
		    destL[0] = 0;
		    destL[1] = 0;
		    destL[2] = 0;
		    destL[3] = 0;
		    destL+= 4;
		}
#endif
	}

    PV_ClearReverbBuffer();
    PV_ClearChorusBuffer();
}



#if USE_MONO_OUTPUT == TRUE
// Process active sample voices in mono

// $$kk: 04.19.99
//INLINE static void PV_ServeMonoInstruments(void)
INLINE static void PV_ServeMonoInstruments(void *threadContext)
{
    register GM_Mixer	*myMusicGlobals;
    register LOOPCOUNT	count;
    register GM_Voice	*pVoice;
    XBOOL				someSoundActive;

    myMusicGlobals = MusicGlobals;
#if USE_VARIABLE_REVERB != FALSE
    if (GM_IsReverbFixed() == FALSE)
	{
	    // Process all active voices in the full-featured variable reverb case.
	    someSoundActive = FALSE;
	    for (count = 0; count < (myMusicGlobals->MaxNotes+myMusicGlobals->MaxEffects); count++)
		{
		    pVoice = &myMusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    PV_ServeThisInstrument(pVoice, threadContext);
			    someSoundActive = TRUE;
			}
		}
	    GM_ProcessReverb();
	}
    else
#endif
	{
	    // Process active voices for the inexpensive reverb cases:
	    // Notes with reverb on are processed first, then the reverb unit, then the dry notes.
	    someSoundActive = FALSE;
	    for (count = 0; count < (myMusicGlobals->MaxNotes+myMusicGlobals->MaxEffects); count++)
		{
		    pVoice = &myMusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    if (pVoice->avoidReverb == FALSE)
				{
				    PV_ServeThisInstrument(pVoice, threadContext);
				    someSoundActive = TRUE;
				}
			}
		}
	    GM_ProcessReverb();
	    for (count = 0; count < (myMusicGlobals->MaxNotes+myMusicGlobals->MaxEffects); count++)
		{
		    pVoice = &myMusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    if (pVoice->avoidReverb != FALSE)
				{
				    PV_ServeThisInstrument(pVoice, threadContext);

				    someSoundActive = TRUE;
				}
			}
		}
	}
}
#endif	// USE_MONO_OUTPUT == TRUE

#if USE_STEREO_OUTPUT == TRUE
// Process active sample voices in stereo

// $$kk: 04.19.99
//INLINE static void PV_ServeStereoInstruments(void)
INLINE static void PV_ServeStereoInstruments(void *threadContext)
{
    register GM_Mixer	*myMusicGlobals;
    register LOOPCOUNT	count;
    register GM_Voice	*pVoice;
    XBOOL				someSoundActive;

    myMusicGlobals = MusicGlobals;

    // Process active voices
    someSoundActive = FALSE;
#if USE_VARIABLE_REVERB != FALSE
    if (GM_IsReverbFixed() == FALSE)
	{
	    for (count = 0; count < (myMusicGlobals->MaxNotes+myMusicGlobals->MaxEffects); count++)
		{
		    pVoice = &myMusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    PV_ServeThisInstrument(pVoice, threadContext);

			    someSoundActive = TRUE;
			}
		}
	    GM_ProcessReverb();

#if USE_NEW_EFFECTS
	    RunChorus(MusicGlobals->songBufferChorus, MusicGlobals->songBufferDry, MusicGlobals->One_Loop);
#endif
	}
    else
#endif
	{
	    for (count = 0; count < (myMusicGlobals->MaxNotes+myMusicGlobals->MaxEffects); count++)
		{
		    pVoice = &myMusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    if (pVoice->avoidReverb == FALSE)
				{
				    PV_ServeThisInstrument(pVoice, threadContext);

				    someSoundActive = TRUE;
				}
			}
		}
	    GM_ProcessReverb();

	    for (count = 0; count < (myMusicGlobals->MaxNotes+myMusicGlobals->MaxEffects); count++)
		{
		    pVoice = &myMusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    if (pVoice->avoidReverb != FALSE)
				{
				    PV_ServeThisInstrument(pVoice, threadContext);

				    someSoundActive = TRUE;
				}
			}
		}
	}
}
#endif	// USE_STEREO_OUTPUT == TRUE

#if USE_HAE_EXTERNAL_API == TRUE

#define GENERATE_TONE		0		// will generate a tone ranther than engine. Used for debugging

#if GENERATE_TONE == 1
// 32 samples of a sine wave to generate a pure tone, 16 bit stereo samples at 22.050 khz
static unsigned short tone[] = {
    0x0000, 0x0000, 0x1600, 0x1600, 0x2A00, 0x2A00, 0x4000, 0x4000, 0x4F00, 0x4F00, 0x5C00, 0x5C00, 0x6400, 0x6400, 0x5D00, 0x5D00,
    0x5600, 0x5600, 0x4C00, 0x4C00, 0x3E00, 0x3E00, 0x2C00, 0x2C00, 0x2000, 0x2000, 0x1200, 0x1200, 0x0400, 0x0400, 0xF300, 0xF300,
    0xE800, 0xE800, 0xDB00, 0xDB00, 0xCF00, 0xCF00, 0xC400, 0xC400, 0xB800, 0xB800, 0xAC00, 0xAC00, 0xA300, 0xA300, 0x9E00, 0x9E00,
    0xA000, 0xA000, 0xA600, 0xA600, 0xB000, 0xB000, 0xBC00, 0xBC00, 0xCA00, 0xCA00, 0xD800, 0xD800, 0xE400, 0xE400, 0xF400, 0xF400
};

static void PV_FillTone(void *pBuffer, UINT32 bufferByteSize)
{
    char	*pAudio;
    INT32	count, toneSize;

    pAudio = (char *)pBuffer;

    toneSize = (INT32)(sizeof(tone) / sizeof(unsigned short));
    for (count = 0; count < (bufferByteSize / toneSize); count++)
	{
	    XBlockMove(tone, pAudio + (toneSize * count), toneSize);
	}
}

#endif

// **** Audio Engine feedback functions. These functions are used to direct or get
//		information about the engine.
//
// NOTE:	This is an external function that your audio card code calls to process
//			a mixer buffer. You call this, don't define it.
//
// Based upon sample rate, channels, and bit size, build 11 milliseconds worth of mixer
// output, and store it in pAudioBuffer.
// bufferByteLength is assumed to be 256, 512, 1024, or 2048 only. The value is ignored
// at the moment. sampleFrames is how many sample frames. These two values should match
// ie. sampleFrames = bufferByteLength / channels / bitsize / 8
void HAE_BuildMixerSlice(void *threadContext, void *pAudioBuffer, INT32 bufferByteLength,
			 INT32 sampleFrames)
{
    GM_Mixer		*pMixer;
    UINT32	delta;

    pMixer = MusicGlobals;
    if (pMixer && pAudioBuffer && bufferByteLength && sampleFrames)
	{
	    delta = XMicroseconds();		// get current time

	    pMixer->insideAudioInterrupt = 1;	// busy

	    pMixer->syncCount += HAE_GetSliceTimeInMicroseconds();			// 11 milliseconds
	    pMixer->syncBufferCount++;

	    // Generate new audio samples, putting them directly
	    // into the output buffer.
	    PV_ProcessSampleFrame(threadContext, pAudioBuffer);

#if GENERATE_TONE == 1
	    PV_FillTone(pAudioBuffer, bufferByteLength);
#endif

	    if (pMixer->pTaskProc)
		{
		    (*pMixer->pTaskProc)(threadContext, pMixer->syncCount);
		}
	    if (pMixer->pOutputProc)
		{
		    (*pMixer->pOutputProc)(threadContext, pAudioBuffer,
					   (pMixer->generate16output) ? 2 : 1,
					   (pMixer->generateStereoOutput) ? 2 : 1,
					   sampleFrames);

		}
	    MusicGlobals->samplesWritten += sampleFrames;

	    GM_UpdateSamplesPlayed(HAE_GetDeviceSamplesPlayedPosition());
	    pMixer->insideAudioInterrupt = 0;	// free

	    pMixer->timeSliceDifference = XMicroseconds() - delta;
	}
}

// Return the number of microseconds of real time that will be generated when calling
// HAE_BuildMixerSlice.
UINT32 HAE_GetSliceTimeInMicroseconds(void)
{
#if 1
    return BUFFER_SLICE_TIME;
#else
    UINT32			time;

    time = BUFFER_SLICE_TIME;
    if (MusicGlobals)
	{
	    switch (MusicGlobals->outputQuality)
		{
		case Q_48K:
		case Q_24K:
		case Q_44K:
		case Q_22K_TERP_44K:
		case Q_22K:
		case Q_11K:
		case Q_11K_TERP_22K:
		case Q_8K:
		    break;
		}
	}
    return time;
#endif
}

// Get time in microseconds between calls to HAE_BuildMixerSlice
UINT32 GM_GetMixerUsedTime(void)
{
    UINT32	time;

    time = 0;
    if (MusicGlobals)
	{
	    time = MusicGlobals->timeSliceDifference;
	}
    return time;
}

// Get CPU load in percent. This function is realtime and assumes the mixer has been allocated
UINT32 GM_GetMixerUsedTimeInPercent(void)
{
    UINT32 load;

    load = 0;
    if (MusicGlobals)
	{
	    load = GM_GetMixerUsedTime() * 100;		// 100 %
	    load = (load / HAE_GetSliceTimeInMicroseconds());
	}
    return load;
}

// Return the maximumn number of samples for 11 milliseconds worth of whatever khz data.
// Typically this is 512. Use this in your calculation of audio buffers. Will return
// 0 if something is wrong.
short int HAE_GetMaxSamplePerSlice(void)
{
    if (MusicGlobals)
	{
	    return MusicGlobals->maxChunkSize;
	}
    return 0;
}
#endif	// X_PLATFORM

// This function will scan for voices that are ready to be started automatically.
// We do it here, because we are guaranteed that no one else is looking at these
// structures, and that the voices will start at exactly the same time.
static void PV_ProcessSyncronizedVoiceStart(void)
{
    GM_Voice		*pArrayToStart[MAX_VOICES];
    GM_Mixer		*pMixer;
    GM_Voice		*pVoice;
    void			*syncReference;
    LOOPCOUNT		count, max;
    UINT32	time;

    pMixer = MusicGlobals;

    max = pMixer->MaxNotes + pMixer->MaxEffects;
    // first, we scan for all voices that are ready to be started, then we
    // gather all voices that match a particular reference
    syncReference = NULL;
    for (count = 0; count < max; count++)
	{
	    pVoice = &pMixer->NoteEntry[count];
	    pArrayToStart[count] = NULL;	// clear
	    if (pVoice->voiceMode == VOICE_ALLOCATED_READY_TO_SYNC_START)
		{
		    if (syncReference == NULL)	// got to set the first voice reference
			{
			    syncReference = pVoice->syncVoiceReference;
			}
		    // does this voice match our reference?
		    if (pVoice->syncVoiceReference == syncReference)
			{
			    pArrayToStart[count] = pVoice;
			}
		}
	}
    time = XMicroseconds();
    // ok, now we have a list of voices that want to be started
    for (count = 0; count < max; count++)
	{
	    pVoice = pArrayToStart[count];
	    if (pVoice)
		{	// fire voice
		    pVoice->voiceStartTimeStamp = time;
		    pVoice->voiceMode = VOICE_SUSTAINING;
		    pVoice->syncVoiceReference = NULL;
		}
	}
}

void PV_ProcessSampleFrame(void *threadContext, void *destinationSamples)
{
    GM_Mixer		*myMusicGlobals;

    myMusicGlobals = MusicGlobals;
    // Set up the various procs:
    switch (myMusicGlobals->interpolationMode)
	{
#if USE_DROP_SAMPLE == TRUE
	case E_AMP_SCALED_DROP_SAMPLE:
	    if (myMusicGlobals->generateStereoOutput)
		{
		    myMusicGlobals->fullBufferProc		= PV_ServeStereoAmpFullBuffer;
		    myMusicGlobals->partialBufferProc	= PV_ServeStereoAmpPartialBuffer;
		    myMusicGlobals->fullBufferProc16	= PV_ServeStereoInterp2FullBuffer16;
		    myMusicGlobals->partialBufferProc16	= PV_ServeStereoInterp2PartialBuffer16;
		}
	    else
		{
		    myMusicGlobals->fullBufferProc		= PV_ServeDropSampleFullBuffer;
		    myMusicGlobals->partialBufferProc	= PV_ServeDropSamplePartialBuffer;
		    myMusicGlobals->fullBufferProc16    = PV_ServeDropSampleFullBuffer16;
		    myMusicGlobals->partialBufferProc16	= PV_ServeDropSamplePartialBuffer16;
		}
	    break;
#endif
#if USE_TERP1 == TRUE
	case E_2_POINT_INTERPOLATION:
	    if (myMusicGlobals->generateStereoOutput)
		{
		    myMusicGlobals->fullBufferProc		= PV_ServeStereoInterp1FullBuffer;
		    myMusicGlobals->partialBufferProc	= PV_ServeStereoInterp1PartialBuffer;
		    myMusicGlobals->fullBufferProc16	= PV_ServeStereoInterp2FullBuffer16;
		    myMusicGlobals->partialBufferProc16	= PV_ServeStereoInterp2PartialBuffer16;
		}
	    else
		{
		    myMusicGlobals->fullBufferProc		= PV_ServeInterp1FullBuffer;
		    myMusicGlobals->partialBufferProc	= PV_ServeInterp1PartialBuffer;
		    myMusicGlobals->fullBufferProc16	= PV_ServeInterp2FullBuffer16;
		    myMusicGlobals->partialBufferProc16	= PV_ServeInterp2PartialBuffer16;
		}
	    break;
#endif
#if USE_TERP2 == TRUE
	case E_LINEAR_INTERPOLATION:
	default:
	    if (myMusicGlobals->generateStereoOutput)
		{
		    myMusicGlobals->fullBufferProc		= PV_ServeStereoInterp2FullBuffer;
		    myMusicGlobals->partialBufferProc	= PV_ServeStereoInterp2PartialBuffer;
		    myMusicGlobals->fullBufferProc16	= PV_ServeStereoInterp2FullBuffer16;
		    myMusicGlobals->partialBufferProc16	= PV_ServeStereoInterp2PartialBuffer16;
		}
	    else
		{
		    myMusicGlobals->fullBufferProc		= PV_ServeInterp2FullBuffer;
		    myMusicGlobals->partialBufferProc	= PV_ServeInterp2PartialBuffer;
		    myMusicGlobals->fullBufferProc16	= PV_ServeInterp2FullBuffer16;
		    myMusicGlobals->partialBufferProc16	= PV_ServeInterp2PartialBuffer16;
		}
	    break;
#endif
	}

    if (myMusicGlobals->generateStereoOutput)
	{
	    myMusicGlobals->filterPartialBufferProc		= PV_ServeStereoInterp2FilterPartialBuffer;
	    myMusicGlobals->filterPartialBufferProc16	= PV_ServeStereoInterp2FilterPartialBuffer16;
	    myMusicGlobals->filterFullBufferProc		= PV_ServeStereoInterp2FilterFullBuffer;
	    myMusicGlobals->filterFullBufferProc16		= PV_ServeStereoInterp2FilterFullBuffer16;
		
	    /* $$fb 2002-01-06 added resample algorithms */
	    myMusicGlobals->resamplePartialBufferProc	= PV_ServeStereoResamplePartialBuffer;
	    myMusicGlobals->resamplePartialBufferProc16	= PV_ServeStereoResamplePartialBuffer16;
	    myMusicGlobals->resampleFullBufferProc		= PV_ServeStereoResampleFullBuffer;
	    myMusicGlobals->resampleFullBufferProc16	= PV_ServeStereoResampleFullBuffer16;
	}
    else
	{
	    myMusicGlobals->filterPartialBufferProc		= PV_ServeInterp2FilterPartialBuffer;
	    myMusicGlobals->filterPartialBufferProc16	= PV_ServeInterp2FilterPartialBuffer16;
	    myMusicGlobals->filterFullBufferProc		= PV_ServeInterp2FilterFullBuffer;
	    myMusicGlobals->filterFullBufferProc16		= PV_ServeInterp2FilterFullBuffer16;

	    /* $$fb 2002-01-06 added resample algorithms */
	    myMusicGlobals->resamplePartialBufferProc	= PV_ServeResamplePartialBuffer;
	    myMusicGlobals->resamplePartialBufferProc16	= PV_ServeResamplePartialBuffer16;
	    myMusicGlobals->resampleFullBufferProc		= PV_ServeResampleFullBuffer;
	    myMusicGlobals->resampleFullBufferProc16	= PV_ServeResampleFullBuffer16;
	}

    if (myMusicGlobals->systemPaused == FALSE)
	{
	    // clear output buffer before starting mix, and verb buffers if enabled
	    PV_ClearMixBuffers(myMusicGlobals->generateStereoOutput);

#if USE_MOD_API
	    // mix MOD output into our output stream before we translate it for final output
	    // we do it here first, if enabled, to allow MOD files to use our verb
	    if (myMusicGlobals->pModPlaying)
		{
		    if (myMusicGlobals->pModPlaying->enableReverb)
			{
			    PV_WriteModOutput(myMusicGlobals->outputQuality, myMusicGlobals->generateStereoOutput);
			}
		}
#endif

#if (X_PLATFORM == X_WEBTV)
	    // For WebTV, we keep MOD support simple. Verb is always enabled, and we call the Mod sample generation
	    // code. If at some time in the future you (WebTV) want to support enbled to disabled verb, this code
	    // will need to be duplicated here below and wrapped around a boolean for enabling or disabling verb.
	    PV_WriteModOutput(myMusicGlobals->outputQuality, myMusicGlobals->generateStereoOutput);
#endif

	    // ok, start any voices in sync that need it
	    PV_ProcessSyncronizedVoiceStart();

	    // process enabled voices, and add verb, and filter
	    if (myMusicGlobals->generateStereoOutput)
		{
#if USE_STEREO_OUTPUT == TRUE
		    // $$kk: 04.19.99
		    // PV_ServeStereoInstruments();
		    PV_ServeStereoInstruments(threadContext);
#endif
		}
	    else
		{
#if USE_MONO_OUTPUT == TRUE
		    // $$kk: 04.19.99
		    // PV_ServeMonoInstruments();
		    PV_ServeMonoInstruments(threadContext);
#endif
		}
#if USE_MOD_API
	    // mix MOD output into our output stream before we translate it for final output
	    // and mix again here in case verb is disabled
	    if (myMusicGlobals->pModPlaying)
		{
		    if (myMusicGlobals->pModPlaying->enableReverb == FALSE)
			{
			    PV_WriteModOutput(myMusicGlobals->outputQuality, myMusicGlobals->generateStereoOutput);
			}
		}
#endif

	    PV_ProcessSequencerEvents(threadContext);		// process all songs and external events
#if X_PLATFORM != X_WEBTV
	    // process sound effects fade

	    // $$kk: 04.19.99
	    // PV_ServeEffectsFades();
	    PV_ServeEffectsFades(threadContext);

	    // process sound effects callbacks
	    PV_ServeEffectCallbacks(threadContext);
#endif

#if USE_STREAM_API
	    // process stream fades
	    PV_ServeStreamFades();
#endif
	    // mix down to final output stage for output to speaker
	    if (myMusicGlobals->generate16output)
		{
		    /* Convert intermediate 16-bit sample format to 16 bit output samples:
		     */
		    if (myMusicGlobals->generateStereoOutput)
			{
#if (USE_16_BIT_OUTPUT == TRUE) && (USE_STEREO_OUTPUT == TRUE)
			    PV_Generate16outputStereo((OUTSAMPLE16 *)destinationSamples);
#endif
			}
		    else
			{
#if (USE_16_BIT_OUTPUT == TRUE) && (USE_MONO_OUTPUT == TRUE)
			    PV_Generate16outputMono((OUTSAMPLE16 *)destinationSamples);
#endif
			}
		}
	    else
		{
		    /* Convert intermediate 16-bit sample format to 8 bit output samples:
		     */
		    if (myMusicGlobals->generateStereoOutput)
			{
#if (USE_8_BIT_OUTPUT == TRUE) && (USE_STEREO_OUTPUT == TRUE)
			    PV_Generate8outputStereo((OUTSAMPLE8 *)destinationSamples);
#endif
			}
		    else
			{
#if (USE_8_BIT_OUTPUT == TRUE) && (USE_MONO_OUTPUT == TRUE)
			    PV_Generate8outputMono((OUTSAMPLE8 *)destinationSamples);
#endif
			}
		}
	}
}

void ServeMIDINote(GM_Song *pSong, INT16 the_instrument,
		   INT16 the_channel, INT16 the_track, INT16 notePitch, INT32 Volume)
{
    register GM_Mixer		*myMusicGlobals;
    register UBYTE			*pSample;
    register GM_Voice		*the_entry;
    register GM_Instrument	*theI;
    register GM_Instrument	*pInstrument;
    register GM_KeymapSplit	*k;
    register INT32			count;
    INT16					newPitch, playPitch;
    UINT16					splitCount;
    UINT32					loopstart, loopend;
    INT32					bestSlot, bestLevel;
    register INT32			i, j;
    INT32					volume32;
    INT32					sampleNumber;

    TRACE_STR("> ServeMIDINote\n");

    myMusicGlobals = MusicGlobals;

    // scale with default velocity curve
    Volume = PV_ModifyVelocityFromCurve(pSong, Volume);

    playPitch = notePitch;
    pInstrument = NULL;
    sampleNumber = 0;
    theI = pSong->instrumentData[pSong->remapArray[the_instrument]];
    if (theI)
	{
	    /*
	      // NOTE AGAIN:
	      // This code is commented out because it messes up some content. I'm not sure
	      // why its here in the first place. This may be a hack to get some content
	      // working with the new root key features for the BeOS. For the time being
	      // leave it commented out. -SH

	      // NOTE!! If useSoundModifierAsRootKey is TRUE, then we are using
	      // the Sound Modifier data blocks as a root key replacement for samples in
	      // the particular split
	      if (theI->useSoundModifierAsRootKey)
	      {
	      if (theI->masterRootKey)
	      {
	      playPitch = notePitch - theI->masterRootKey + 60;
	      }
	      playPitch = playPitch + 60 - theI->miscParameter1;
	      }
	      else
	    */
	    {
		// old way
		if (theI->masterRootKey)
		    {
			playPitch = notePitch - theI->masterRootKey + 60;
		    }
	    }
	    // keysplit?
	    if (theI->doKeymapSplit)
		{	// yes, find an instrument
		    splitCount = theI->u.k.KeymapSplitCount;
		    k = theI->u.k.keySplits;
		    for (count = 0; count < splitCount; count++)
			{
			    if ( (playPitch >= k->lowMidi) && (playPitch <= k->highMidi) )
				{
				    theI = k->pSplitInstrument;
				    if (theI)
					{
					    pInstrument = theI;
					    // NOTE!! If useSoundModifierAsRootKey is TRUE, then we are using
					    // the Sound Modifier data blocks as a root key replacement for samples in
					    // the particular split
					    if (theI->useSoundModifierAsRootKey)
						{
						    if (theI->masterRootKey)
							{
							    playPitch = notePitch - theI->masterRootKey + 60;
							}
						    playPitch = playPitch + 60 - theI->miscParameter1;
						    Volume = (Volume * theI->miscParameter2) / 100;		// scale volume based upon split
						}
					    else
						{
						    if (theI->masterRootKey)
							{
							    playPitch = notePitch - theI->masterRootKey + 60;
							}
						}
					    break;
					}
				}
			    k++;
			    sampleNumber++;
			}
		}
	    else
		{
		    pInstrument = theI;
		    if (theI->useSoundModifierAsRootKey)
			{
			    playPitch = playPitch + 60 - theI->miscParameter1;
			    Volume = (Volume * theI->miscParameter2) / 100;		// scale volume based upon split
			}
		}
	}
    if (pInstrument == NULL)
	{
	    //DEBUG_STR("\pInstrument not loaded");
	    TRACE_STR("< ServeMIDINote\n");
	    return;
	}
    loopstart = pInstrument->u.w.startLoop;
    loopend = pInstrument->u.w.endLoop;
    if ( (pInstrument->disableSndLooping) ||
	 (loopstart == loopend) ||
	 (loopstart > loopend) ||
	 (loopend > pInstrument->u.w.waveFrames) ||
	 (loopend - loopstart < MIN_LOOP_SIZE) )
	{
	    loopstart = 0;
	    loopend = 0;
	}

    // If the count of active notes exceeds the normal limits
    // for voices, then replace ACTIVE slots in their decay cycle first (to
    // reduce the voice load on the CPU.) If count's within normal limits, then
    // use EMPTY slots first to improve sound quality of the other notes by allowing
    // them to decay more completely before being killed.

    // Find a place for the new note
    the_entry = NULL;

    // These calculations are needed anyway, and also are used in the following prioritization code.
    volume32 = (Volume * MusicGlobals->scaleBackAmount) >> 8;
    // NOTE: This the only place we work with the scaleBackAmount. I'm not sure this is the right place for
    // this!!!
    volume32 = PV_ScaleVolumeFromChannelAndSong(pSong, the_channel, volume32);

    // Find an active slot (best), or
    // Terminate notes in one-shot decay (less desirable)
    // or notes naturally fading out (preferable)
    bestSlot = -1; bestLevel = 0x10000;
    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	{
	    if (myMusicGlobals->NoteEntry[count].voiceMode == VOICE_UNUSED)
		{
		    the_entry = &myMusicGlobals->NoteEntry[count];
		    goto EnterNote;
		}

#if 1
	    if (myMusicGlobals->NoteEntry[count].voiceMode != VOICE_SUSTAINING)
		{
		    if (bestLevel > 0x2000)
			{
			    bestLevel = 0x2000;
			    the_entry = &myMusicGlobals->NoteEntry[count];
			}
		}
	    else
		{
		    if (myMusicGlobals->NoteEntry[count].NoteProgram == the_instrument)
			{
			    if (myMusicGlobals->NoteEntry[count].NoteChannel == the_channel)
				{
				    if (myMusicGlobals->NoteEntry[count].volumeADSRRecord.sustainingDecayLevel < bestLevel)
					{
					    bestLevel = myMusicGlobals->NoteEntry[count].volumeADSRRecord.sustainingDecayLevel;
					    the_entry = &myMusicGlobals->NoteEntry[count];
					}
				}
			}
		}
#endif
	}

    if (bestLevel <= 0x2000)
	goto EnterNote;

#if 1
    // Now kill notes that are much lower in volume than the current note (less than 25% of the volume)
    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	{
	    if (myMusicGlobals->NoteEntry[count].NoteVolume < bestLevel)
		{
		    bestLevel = myMusicGlobals->NoteEntry[count].NoteVolume;
		    the_entry = &myMusicGlobals->NoteEntry[count];
		}
	}
    if ((bestLevel * 8) <  volume32)
	goto EnterNote;

    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	{
	    if ( bestLevel > ((myMusicGlobals->NoteEntry[count].NoteVolume *
			       myMusicGlobals->NoteEntry[count].NoteVolumeEnvelopeBeforeLFO) >> VOLUME_PRECISION_SCALAR))
		{
		    bestLevel = (myMusicGlobals->NoteEntry[count].NoteVolume *
				 myMusicGlobals->NoteEntry[count].NoteVolumeEnvelopeBeforeLFO) >> VOLUME_PRECISION_SCALAR;
		    the_entry = &myMusicGlobals->NoteEntry[count];
		}
	}

    if ((bestLevel * 8) <  volume32)
	goto EnterNote;

#endif

#if 1
    // Now kill notes that are in sustain pedal mode, are in same channel or instrument
    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	{
	    if (myMusicGlobals->NoteEntry[count].sustainMode != SUS_NORMAL)
		{
		    if (myMusicGlobals->NoteEntry[count].NoteProgram == the_instrument)
			{
			    if (myMusicGlobals->NoteEntry[count].NoteChannel == the_channel)
				{
				    the_entry = &myMusicGlobals->NoteEntry[count];
				    goto EnterNote;
				}
			}
		}
	}
#endif

    // gently terminate (euthanize) any notes of same pitch, instrument number, and channel:
#if 0
    // don't want to do this all the time. only certain instruments need this feature. We might
    // want to be able to flag this and let the instrument designer design this.
    // drums rolls don't sound right.
    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	if (myMusicGlobals->NoteEntry[count].voiceMode != VOICE_UNUSED)
	    if (myMusicGlobals->NoteEntry[count].NoteProgram == the_instrument)
		if (myMusicGlobals->NoteEntry[count].NoteMIDIPitch == notePitch)
		    if (myMusicGlobals->NoteEntry[count].NoteChannel == the_channel)
			{
			    if (the_channel != PERCUSSION_CHANNEL)
				{
				    myMusicGlobals->NoteEntry[count].volumeADSRRecord.mode = ADSR_TERMINATE;
				    myMusicGlobals->NoteEntry[count].volumeADSRRecord.currentPosition = 0;
				    myMusicGlobals->NoteEntry[count].volumeADSRRecord.ADSRLevel[0] = 0;
				    myMusicGlobals->NoteEntry[count].volumeADSRRecord.ADSRTime[0] = 1;
				    myMusicGlobals->NoteEntry[count].volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
				    myMusicGlobals->NoteEntry[count].NoteVolumeEnvelopeBeforeLFO = 0;		// so these notes can be reused
				}
			}
#endif
    return;
EnterNote:

    if (the_entry)
	{
	    the_entry->voiceMode = VOICE_ALLOCATED;
	    PV_CleanNoteEntry(the_entry);
	    the_entry->pInstrument = pInstrument;
	    the_entry->pSong = pSong;
	    the_entry->NoteVolumeEnvelopeBeforeLFO = VOLUME_PRECISION_SCALAR;
	    pSample = (UBYTE *)pInstrument->u.w.theWaveform;
	    the_entry->NotePtr = pSample;
	    the_entry->NotePtrEnd = pSample + pInstrument->u.w.waveFrames;
	    the_entry->NoteChannel = (SBYTE)the_channel;
	    the_entry->NoteTrack = (SBYTE)the_track;

	    // copy the volume ADSR record into the GM_Voice
	    the_entry->volumeADSRRecord = pInstrument->volumeADSRRecord;

	    // copy the sample-and-hold flag
	    the_entry->sampleAndHold = pInstrument->sampleAndHold;

	    // Copy the LFO record count
	    the_entry->LFORecordCount = pInstrument->LFORecordCount;

	    // If there are any LFO records, copy them into the GM_Voice.
	    if (the_entry->LFORecordCount)
		{
		    for (i = 0; i < the_entry->LFORecordCount; i++)
			{
			    the_entry->LFORecords[i] = pInstrument->LFORecords[i];
			}
		}

	    the_entry->avoidReverb = pInstrument->avoidReverb;	// use instrument default. in case instrument designer
	    the_entry->reverbLevel = pSong->channelReverb[the_channel];	// set current verb level
	    the_entry->chorusLevel = (INT16)PV_ModifyVelocityFromCurve(pSong, pSong->channelChorus[the_channel]);
	    // wants no verb enabled
	    if (GM_IsReverbFixed())
		{
		    // if the instrument defines reverb on or the channel has reverb on, then enable it.
		    // if the channel is off, but the instrument defines reverb then enable it
		    if (pSong->channelReverb[the_channel] < GM_GetReverbEnableThreshold())
			{
			    the_entry->avoidReverb = TRUE;		// force off
			}
		}
	    if (the_entry->avoidReverb)
		{
		    the_entry->reverbLevel = 0;
		    the_entry->chorusLevel = 0;
		}

	    // Setup playback pitch
	    if (pInstrument->playAtSampledFreq == FALSE)
		{
		    if (theI->useSoundModifierAsRootKey)
			{
			    newPitch = playPitch;	// already been modified
			}
		    else
			{
			    newPitch = playPitch + 60 - pInstrument->u.w.baseMidiPitch;
			}
		    while (newPitch < -24) { newPitch += 12; }
		    while (newPitch > 144) { newPitch -= 12; }
		    the_entry->ProcessedPitch = newPitch;
		    the_entry->NotePitch = majorPitchTable[newPitch+24];

		}
	    else
		{
		    the_entry->ProcessedPitch = 0;
		    the_entry->NotePitch = 0x10000;		// 1.0 step rate
		}

	    // factor in sample rate of sample, if enabled
	    if (pInstrument->useSampleRate)
		{
		    the_entry->noteSamplePitchAdjust = XFixedDivide(pInstrument->u.w.sampledRate >> 2, 22050L << 14);
		    the_entry->NotePitch = XFixedMultiply (the_entry->NotePitch, the_entry->noteSamplePitchAdjust);
		}
	    else
		{
		    the_entry->noteSamplePitchAdjust = 0x10000;
		}
	    if ((loopend - loopstart))
		{
		    the_entry->NoteLoopPtr = the_entry->NotePtr + FP_OFF(loopstart);
		    the_entry->NoteLoopEnd = the_entry->NotePtr + FP_OFF(loopend);
		}
	    else
		{
		    the_entry->NoteLoopPtr = 0;
		    the_entry->NoteLoopEnd = 0;
		}
	    the_entry->NoteDecay = 8;	// default note decay
	    the_entry->NoteNextSize = 0;
	    the_entry->NoteWave = 0;

	    the_entry->NoteProgram = the_instrument;
	    the_entry->NoteMIDIPitch = (SBYTE)notePitch;	// save note pitch unprocessed
	    the_entry->noteOffsetStart = (SBYTE)pSong->songPitchShift;
	    the_entry->NoteMIDIVolume = (INT16)Volume;	// save note volume unscaled

	    the_entry->NoteVolume = volume32;

	    // Resonant low-pass filter stuff
	    the_entry->LPF_base_frequency = pInstrument->LPF_frequency;
	    the_entry->LPF_base_resonance = pInstrument->LPF_resonance;
	    the_entry->LPF_base_lowpassAmount = pInstrument->LPF_lowpassAmount;

	    the_entry->LPF_frequency = pInstrument->LPF_frequency;
	    the_entry->LPF_resonance = pInstrument->LPF_resonance;
	    the_entry->LPF_lowpassAmount = pInstrument->LPF_lowpassAmount;

	    the_entry->NoteEndCallback = NULL;
	    the_entry->NoteLoopProc = NULL;
	    the_entry->NoteContext = NULL;
	    the_entry->NotePitchBend = pSong->channelBend[the_channel];
	    the_entry->LastPitchBend = 0;
	    the_entry->ModWheelValue = pSong->channelModWheel[the_channel];
	    the_entry->LastModWheelValue = 0;
	    the_entry->NoteLoopCount = 0;

	    // set the inital pan placement. If zero, then just use the current channel placement
	    if (pInstrument->panPlacement)
		{
		    the_entry->stereoPosition = pInstrument->panPlacement;
		}
	    else
		{
		    //			the_entry->stereoPosition = pSong->channelStereoPosition[the_channel];

		    // Grab the unchanged value then scale it for the mixer note setup
		    the_entry->stereoPosition = SetChannelStereoPosition(pSong, the_channel, pSong->channelStereoPosition[the_channel]);
		}
	    the_entry->bitSize = pInstrument->u.w.bitSize;
	    the_entry->channels = pInstrument->u.w.channels;

	    if (pSong->channelSustain[the_channel])
		{
		    the_entry->sustainMode = SUS_ON_NOTE_ON;
		}
	    else
		{
		    the_entry->sustainMode = SUS_NORMAL;
		}

	    if (pInstrument->curveRecordCount)
		{
		    for (i = 0; i < pInstrument->curveRecordCount; i++)
			{
			    INT32 scalar;
			    INT32 tieFromValue = 0;
			    INT32 curveCount = pInstrument->curve[i].curveCount;
			    switch (pInstrument->curve[i].tieFrom)
				{
				case PITCH_LFO:
				    //DEBUG_STR("\p tying to pitch");
				    tieFromValue = notePitch;
				    break;
				case VOLUME_LFO:
				    tieFromValue = Volume;
				    break;
				case SAMPLE_NUMBER:
				    tieFromValue = sampleNumber;
				    break;
				case MOD_WHEEL_CONTROL:
				    tieFromValue = pSong->channelModWheel[the_channel];
				    break;
				default:
				    //DEBUG_STR("\p invalid tie-from value");
				    break;
				}
			    scalar = tieFromValue;
			    for (count = 0; count < curveCount; count++)
				{
				    if (pInstrument->curve[i].from_Value[count] <= tieFromValue)
					{
					    if (pInstrument->curve[i].from_Value[count+1] >= tieFromValue)
						{
						    scalar = pInstrument->curve[i].to_Scalar[count];
						    if (pInstrument->curve[i].from_Value[count] != pInstrument->curve[i].from_Value[count+1])
							{
							    INT32 from_difference = pInstrument->curve[i].from_Value[count+1] - pInstrument->curve[i].from_Value[count];
							    INT32 to_difference = pInstrument->curve[i].to_Scalar[count+1] - pInstrument->curve[i].to_Scalar[count];
							    scalar += ((((tieFromValue - pInstrument->curve[i].from_Value[count]) << 8) / from_difference) * to_difference) >> 8;
							}
						}
					}
				}
			    switch (pInstrument->curve[i].tieTo)
				{
				case NOTE_VOLUME:
				    //DEBUG_STR("\p scaling note velocity");
				    the_entry->NoteVolume = (the_entry->NoteVolume * scalar) >> 8;
				    break;
				case SUSTAIN_RELEASE_TIME:
				    for (j = 0; j < ADSR_STAGES; j++)
					{
					    if (the_entry->volumeADSRRecord.ADSRFlags[j] == ADSR_SUSTAIN)
						{
						    if (the_entry->volumeADSRRecord.ADSRLevel[j] < 0)
							{
							    scalar = scalar >> 2;
							    if (the_entry->volumeADSRRecord.ADSRLevel[j]  < -50)
								the_entry->volumeADSRRecord.ADSRLevel[j] =
								    -(-(the_entry->volumeADSRRecord.ADSRLevel[j] * scalar) >> 6);
							    else
								the_entry->volumeADSRRecord.ADSRLevel[j] =
								    -((logLookupTable[-the_entry->volumeADSRRecord.ADSRLevel[j]] * scalar) >> 6);
							    break;	//j = ADSR_STAGES;
							}
						}
					}
				    break;
				case SUSTAIN_LEVEL:
				    for (j = 1; j < ADSR_STAGES; j++)
					{
					    if (the_entry->volumeADSRRecord.ADSRFlags[j] == ADSR_SUSTAIN)
						{
						    if (the_entry->volumeADSRRecord.ADSRLevel[j] > 0)
							the_entry->volumeADSRRecord.ADSRLevel[j] =
							    (the_entry->volumeADSRRecord.ADSRLevel[j] * scalar) >> 8;
						    else
							the_entry->volumeADSRRecord.ADSRLevel[j-1] =
							    (the_entry->volumeADSRRecord.ADSRLevel[j-1] * scalar) >> 8;
						    break;	//j = ADSR_STAGES;
						}
					}
				    break;
				case RELEASE_TIME:
				    for (j = 1; j < ADSR_STAGES; j++)
					{
					    if (the_entry->volumeADSRRecord.ADSRFlags[j] == ADSR_TERMINATE)
						{
						    if (the_entry->volumeADSRRecord.ADSRTime[j] > 0)
							the_entry->volumeADSRRecord.ADSRTime[j] =
							    (the_entry->volumeADSRRecord.ADSRTime[j] * scalar) >> 8;
						    else
							the_entry->volumeADSRRecord.ADSRTime[j-1] =
							    (the_entry->volumeADSRRecord.ADSRTime[j-1] * scalar) >> 8;
						    break;	//j = ADSR_STAGES;
						}
					}
				    break;
				case VOLUME_ATTACK_TIME:
				    if (the_entry->volumeADSRRecord.ADSRTime[0] != 0)
					the_entry->volumeADSRRecord.ADSRTime[0] = (the_entry->volumeADSRRecord.ADSRTime[0] * scalar) >> 8;
				    else
					the_entry->volumeADSRRecord.ADSRTime[1] = (the_entry->volumeADSRRecord.ADSRTime[1] * scalar) >> 8;
				    break;
				case VOLUME_ATTACK_LEVEL:
				    if (1)	//(*((long *) 0x17a))
					{
					    if (the_entry->volumeADSRRecord.ADSRLevel[0] >  the_entry->volumeADSRRecord.ADSRLevel[1])
						the_entry->volumeADSRRecord.ADSRLevel[0] = (the_entry->volumeADSRRecord.ADSRLevel[0] * scalar) >> 8;
					    else
						the_entry->volumeADSRRecord.ADSRLevel[1] = (the_entry->volumeADSRRecord.ADSRLevel[1] * scalar) >> 8;
					}
				    break;
				case WAVEFORM_OFFSET:
				    the_entry->NoteWave = scalar << STEP_BIT_RANGE;
				    break;
				case LOW_PASS_AMOUNT:
				    the_entry->LPF_base_lowpassAmount = (the_entry->LPF_base_lowpassAmount * scalar) >> 8;
				    break;
				case PITCH_LFO:
				    for (j = the_entry->LFORecordCount - 1; j >= 0; --j)
					if (the_entry->LFORecords[j].where_to_feed == PITCH_LFO)
					    {
						the_entry->LFORecords[j].level = (the_entry->LFORecords[j].level * scalar) >> 8;
						goto exit;
					    }
				    break;
				case VOLUME_LFO:
				    for (j = the_entry->LFORecordCount - 1; j >= 0; --j)
					if (the_entry->LFORecords[j].where_to_feed == VOLUME_LFO)
					    {
						the_entry->LFORecords[j].level = (the_entry->LFORecords[j].level * scalar) >> 8;
						goto exit;
					    }
				    break;
				case PITCH_LFO_FREQUENCY:
				    for (j = 0; j < the_entry->LFORecordCount; j++)
					if (the_entry->LFORecords[j].where_to_feed == PITCH_LFO)
					    {
						the_entry->LFORecords[j].period = (the_entry->LFORecords[j].period * scalar) >> 8;
						goto exit;
					    }
				    break;
				case VOLUME_LFO_FREQUENCY:
				    for (j = 0; j < the_entry->LFORecordCount; j++)
					if (the_entry->LFORecords[j].where_to_feed == VOLUME_LFO)
					    {
						the_entry->LFORecords[j].period = (the_entry->LFORecords[j].period * scalar) >> 8;
						goto exit;
					    }
				    break;
				}
			exit:;
			}
		}


	    // is there an initial volume level in the ADSR record that starts at time=0?  If so, don't interpolate the
	    // note's volume up from 0 to the first target level.  Otherwise, it's a traditional ramp-up from 0.
	    if (the_entry->volumeADSRRecord.ADSRTime[0] == 0)
		{
		    the_entry->volumeADSRRecord.currentLevel = the_entry->volumeADSRRecord.ADSRLevel[0];
		    the_entry->NoteVolumeEnvelope = (INT16)the_entry->volumeADSRRecord.ADSRLevel[0];
		    if (MusicGlobals->generateStereoOutput)
			{
			    PV_CalculateStereoVolume(the_entry, &the_entry->lastAmplitudeL, &the_entry->lastAmplitudeR);
			}
		    else
			{
			    the_entry->lastAmplitudeL = (the_entry->NoteVolume * the_entry->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
			}
		}
	    else
		{
		    the_entry->lastAmplitudeL = 0;
		    the_entry->lastAmplitudeR = 0;
		}

	    // This step is performed last.
	    the_entry->voiceStartTimeStamp = XMicroseconds();
	    the_entry->voiceMode = VOICE_SUSTAINING;
	}
    TRACE_STR("< ServeMIDINote\n");
}

void StopMIDINote(GM_Song *pSong, INT16 the_instrument, INT16 the_channel, INT16 the_track, INT16 notePitch)
{
    register LOOPCOUNT		count;
    register GM_Mixer		*myMusicGlobals;
    register INT16			decay;
    register GM_Voice		*pNote;
    INT16					realNote, compareNote;

    myMusicGlobals = MusicGlobals;
    the_track = the_track;
    the_instrument = the_instrument;
    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	{
	    pNote = &myMusicGlobals->NoteEntry[count];
	    if (pNote->voiceMode != VOICE_UNUSED)							// still playing
		{
		    if (pNote->pSong == pSong)						// same song
			{
			    //				if (pNote->NoteProgram == the_instrument)	// same program
			    {
				if (pNote->NoteChannel == the_channel)	// same channel
				    {
					realNote = pNote->NoteMIDIPitch - pNote->noteOffsetStart;
					compareNote = notePitch - pSong->songPitchShift;
					if (realNote == compareNote)		// same pitch
					    {
						// if pedal is on, put note into SUS_ON_NOTE_OFF, and don't kill note
						if (pSong->channelSustain[the_channel])
						    {
							pNote->sustainMode = SUS_ON_NOTE_OFF;
						    }
						else
						    {
							pNote->voiceMode = VOICE_RELEASING;	// put into release mode, decay
							decay = pNote->NoteDecay;
							if ( (decay > 500) || (decay < 0) )
							    {
								//DEBUG_STR("\pStopMIDINote:decay out of range!");
								pNote->NoteDecay = 1;
							    }
						    }
					    }
				    }
			    }
			}
		}
	}
}

// Stop or kill notes for song passed. If pSong is NULL, then all notes are processed otherwise
// just notes associated with the particular pSong.
// If kill is FALSE then the notes are put into release mode. If kill is TRUE, then the note is terminated
// without release. May cause clicks.
// Set useChannel to -1 if kill all notes, otherwise its a channel filter
static void PV_EndNotes(GM_Song *pSong, short int useChannel, XBOOL kill)
{
    register short int		count;
    register GM_Mixer		*myMusicGlobals;
    register GM_Voice		*pNote;

    myMusicGlobals = MusicGlobals;

    for (count = 0; count < myMusicGlobals->MaxNotes; count++)
	{
	    pNote = &myMusicGlobals->NoteEntry[count];
	    if ((pSong == NULL) || (pNote->pSong == pSong))
		{
		    if ((useChannel == -1) || (pNote->NoteChannel == useChannel))
			{
			    if (pNote->voiceMode != VOICE_UNUSED)
				{
				    if (kill)
					{
					    pNote->voiceMode = VOICE_UNUSED;
					    pNote->NoteDecay = 0;
					}
				    else
					{
					    pNote->voiceMode = VOICE_RELEASING;
					    pNote->NoteDecay = 2;
					}
				    pNote->volumeADSRRecord.mode = ADSR_TERMINATE;
				    pNote->volumeADSRRecord.currentPosition = 0;
				    pNote->volumeADSRRecord.ADSRLevel[0] = 0;
				    pNote->volumeADSRRecord.ADSRTime[0] = 1;
				    pNote->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
				    pNote->NoteVolumeEnvelopeBeforeLFO = 0;		// so these notes can be reused
				}
			}
		}
	}
}

// Stop notes for song passed. This will put the note into release mode.
void GM_EndSongNotes(GM_Song *pSong)
{
    PV_EndNotes(pSong, -1, FALSE);
}

// stop notes for a song and channel passed. This will put the note into release mode.
void GM_EndSongChannelNotes(GM_Song *pSong, short int channel)
{
    PV_EndNotes(pSong, channel, FALSE);
}


// Stop just midi notes. Note: This does not kill the notes anymore. It just puts them into release mode.
// Much more professional experience.
void GM_EndAllNotes(void)
{
    GM_EndSongNotes(NULL);
}

// kills notes for song passed. This will put the note into release mode.
void GM_KillSongNotes(GM_Song *pSong)
{
    PV_EndNotes(pSong, -1, TRUE);
}

// Stop just midi notes. Note: This kills the notes currently playing. It may result in clicks.
void GM_KillAllNotes(void)
{
    GM_KillSongNotes(NULL);
}


// Used to get the current frame of audio data that has been built. Useful for fun displays. Returns 16 bit information
// only. If generating 8 bit, then data output is converted. If mono data then right channel will be dead.
// This code is deliberately less efficient than the real output scaling code, for space conservation purposes.
INT16 GM_GetAudioSampleFrame(INT16 *pLeft, INT16 *pRight)
{
    register LOOPCOUNT		size, count;
    register INT32			*sourceL;
    register INT32			i, k8000;

    size = 0;
    if (MusicGlobals)
	{
	    k8000 = 0x8000;
	    sourceL = &MusicGlobals->songBufferDry[0];
	    size = MusicGlobals->One_Loop;

	    if (MusicGlobals->generateStereoOutput)
		{
		    for (count = 0; count < MusicGlobals->Four_Loop; count++)
			{
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pRight++ = (INT16)(i - k8000);

			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pRight++ = (INT16)(i - k8000);

			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pRight++ = (INT16)(i - k8000);

			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pRight++ = (INT16)(i - k8000);
			}
		}
	    else
		{
		    for (count = 0; count < MusicGlobals->Four_Loop; count++)
			{
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);

			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);

			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);

			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			    i = (*sourceL++ >> OUTPUT_SCALAR);
			    i += k8000;
			    if (i & 0xFFFF0000)
				{ if (i > 0) i = 0xFFFE; else i = 0;}
			    *pLeft++ = (INT16)(i - k8000);
			}
		}
	}
    return (INT16)size;
}


/* EOF of GenSynth.c
 */

