/*
 * @(#)HAEPrivate.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "HAEPrivate.h"
**
**	Private access to HAE internals.
**
** Modification History:
**
**	5/7/96		Created
**				Moved HAE_TranslateOPErr from HAE.cpp
**	7/17/98		Added HAE_UseThisFile
**	11/19/98	Added new parameter to HAE_UseThisFile
**	3/2/99		Added HAE_TranslateHAErr
*/
/*****************************************************************************/

#ifndef HAE_AUDIO_PRIVATE
#define HAE_AUDIO_PRIVATE

#ifndef HAE_AUDIO
#include "HAE.h"
#endif

#ifndef G_PRIVATE
#include "GenPriv.h"
#endif

// Translate from OPErr to HAEErr
HAEErr HAE_TranslateOPErr(OPErr theErr);

// Translate from HAEErr to OPErr
OPErr HAE_TranslateHAErr(HAEErr theErr);

// translate reverb types from HAEReverbMode to ReverbMode
ReverbMode HAE_TranslateFromHAEReverb(HAEReverbMode igorVerb);
// translate reverb types to HAEReverbMode from ReverbMode
HAEReverbMode HAE_TranslateToHAEReverb(ReverbMode r);

// Change audio file to use the passed in XFILE
HAEErr HAE_UseThisFile(XFILE audioFile, XBOOL closeOldFile);

#endif	// HAE_AUDIO_PRIVATE
