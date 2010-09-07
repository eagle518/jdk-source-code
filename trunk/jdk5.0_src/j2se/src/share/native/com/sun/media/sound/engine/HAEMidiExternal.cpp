/*
 * @(#)HAEMidiExternal.cpp	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
** "HAEMidiExternal.cpp"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**
** Modification History:
**
**	2/9/97		Created
**	6/18/97		Modified PV_MidiInputHandler to conform to Win32 API
**	6/20/97		Modified for CW 2.0. Name changes, etc
**  6/25/97     (ddz) Added Name Manager provider stuff
**	7/17/97		Added X_PLATFORM == X_WIN_HAE tests
**	7/22/97		Added the ability to selectivly connect with OMS and the Name Manager or not
**				Moved some OMS Mac stuff around to compile for intel platforms
**	8/18/97		Changed X_WIN_HAE to USE_HAE_EXTERNAL_API
**	11/10/97	Fixed a bug with PV_ConnectOMS in which the name manager was accessed, but 
**				the support drivers were not there, so it crashed.
**	11/14/97	Fixed some problems with CW 2.0
**	2/9/98		Fixed crashing OMS bug in which the HAE_UNIQUE_ID ID that is passed
**				into OMSCreateVirtualDestination caused OMS to crash. Also put in
**				some preflight memory allocations to make sure we have some extra
**				RAM before calling into OMS, because tight memory failures also
**				cause OMS to crash.
**
*/
/*****************************************************************************/

#include "HAE.h"
#include "HAEMidiExternal.h"

#include "X_API.h"
#include "X_Formats.h"
#include "GenPriv.h"
#include "GenSnd.h"

#if X_PLATFORM == X_MACINTOSH
#include <Types.h>
#include <Devices.h>
#include <Memory.h>
#include <Resources.h>
#include <Retrace.h>
#include <Sound.h>
#include <Timer.h>
#include <LowMem.h>
#include <Midi.h>
#include <Folders.h>
#include <Script.h>
#include <Gestalt.h>

#include "OMS.h"
#include "OMSNameMgr.h"

// Set global variables
#if GENERATING68K	
#pragma parameter __D0 GetA5
long GetA5(void) = 0x200D;		/* MOVE.L    A5,D0 */

#define GetGlobalsRegister()	GetA5()
#define SetGlobalsRegister(x)	SetA5((x))
#else
#define GetGlobalsRegister()	0
#define SetGlobalsRegister(x)	(x)
#endif

#endif

enum
{
    MIDI_PORT_BUFFER_SIZE	= 500,
    MIDI_INPUT_ID 			= 'inpt',
    MIDI_CLIENT_ID			= 'Igor',		// application ID
    NAMES_FILE_TYPE			= 'iNmz',
    HAE_UNIQUE_ID			= 0				// suppose to be a short
};

enum
{
    USE_OMS	= 0,
    USE_OMS_NAME,
    USE_MIDI_MANAGER
};

#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
#include <windows.h>
#include <mmsystem.h>
#define MAX_MIDI_INPUTS		32
#endif

struct XDeviceControl
{
    long				globalsVariable;
    HAEMidiExternal		*pDirect;
    HAEAudioMixer		*pMixer;

#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
    HMIDIIN			midiInputHandle[MAX_MIDI_INPUTS];	
#endif
};
typedef struct XDeviceControl	XDeviceControl;

#if X_PLATFORM == X_MACINTOSH

static XBOOL				externalMidiEnabled = FALSE;
static XBOOL				registeredMIDISignOn = FALSE;
static XBOOL				registeredMIDIinPort = FALSE;
static XBOOL				usedMIDIManager = FALSE;
static XBOOL				usedOMS = FALSE;
static XBOOL				usedOMSNameManager = FALSE;

static short int 			midiInputRef = 0;
static OMSAppHookUPP		omsAppHook = NULL;
static OMSReadHook2UPP		omsReadHook = NULL;
static OMSReadHookUPP		omsDrvrReadHook = NULL;		// hook for driver, so it works with OMS 1.x
static OMSUniqueID			omsUniqueID = HAE_UNIQUE_ID;
static short int			gChosenInputID;				// uniqueID of selected input; 0 means none
static short int			gCompatMode;

static MIDIReadHookUPP		mmReadHook = NULL;
static void					*omsGlobalReference = NULL;		// used for driver->hook communication

#define USING_DRIVER
#ifdef USING_DRIVER

#define kBeanikDriverUniqueIDSelector	'BtUq'
#define kBeatnikDriverSelector			'BtDv'

static Boolean gDriverAvailable = FALSE;

pascal OSErr PV_SelectorProc(OSType selector, long *response);

#endif

#endif	// X_PLATFORM == X_MACINTOSH

static XDeviceControl		*globalDeviceControl = NULL;

#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
static XDeviceControl		*pWinDevice = NULL;
#endif

#define BASE_TIME_OFFSET	25000

//
// Process MIDI Events
//
// This is the main enterance into the Synthesizer via an external source
//
//
//
static void PV_ProcessExternalMidiEvent(HAEMidiExternal *pDirect, short int commandByte, 
					short int data1Byte, short int data2Byte)
{
    unsigned long	time;

    time = XMicroseconds() + pDirect->GetTimeBaseOffset();
    pDirect->ParseMidiData((unsigned char)commandByte,
			   (unsigned char)data1Byte, (unsigned char)data2Byte,
			   0, time);
}

#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
/* PV_MidiInputHandler - Low-level callback function to handle MIDI input.
 *      Installed by midiInOpen().  The input handler takes incoming
 *      MIDI events and places them in the input buffer.   
 *
 * Param:   hMidiIn - Handle for the associated input device.
 *          wMsg - One of the MIM_***** messages.
 *          dwInstance - Points to CALLBACKINSTANCEDATA structure.
 *          dwParam1 - MIDI data.
 *          dwParam2 - Timestamp (in milliseconds)
 *
 * Return:  void
 */     
extern "C" void CALLBACK PV_MidiInputHandler(HMIDIIN hMidiIn, UINT wMsg, 
					     DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
											
void CALLBACK PV_MidiInputHandler(HMIDIIN hMidiIn, UINT wMsg, 
				  DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    //	XDeviceControl	*pDevice;
    char			*pBytes;

    hMidiIn = hMidiIn;
    dwInstance = dwInstance;
    dwParam2 = dwParam2;
    //	pDevice = (XDeviceControl *)dwInstance;
    if (globalDeviceControl)
	{
	    switch(wMsg)
		{
		case MIM_OPEN:
		case MIM_ERROR:
		default:
		    break;

		    /* The only error possible is invalid MIDI data, so just pass
		     * the invalid data on so we'll see it.
		     */
		case MIM_DATA:   
				/* Send the MIDI event to the MIDI Mapper, put it in the
				 * circular input buffer, and notify the application that
				 * data was received.
				 */
		    pBytes = (char *)&dwParam1;
		    PV_ProcessExternalMidiEvent(globalDeviceControl->pDirect, pBytes[0], pBytes[1], pBytes[2]);
		    break;
		}
	}
}
#endif

#if X_PLATFORM == X_MACINTOSH
static pascal void OMS_AppHook(OMSAppHookMsg *pkt, long theRefCon)
{
    XDeviceControl	*pDevice;

    pDevice = (XDeviceControl *)theRefCon;
    if (pDevice)
	{
	    theRefCon = SetGlobalsRegister(pDevice->globalsVariable);		/* Set current A5 */

	    switch (pkt->msgType)
		{
		case omsMsgModeChanged:
				/* Respond to compatibility mode having changed */
		    gCompatMode = pkt->u.modeChanged.newMode;
				/* this will cause side effects in the event loop */
		    break;
		    /*
		      case omsMsgDestDeleted:
		      if (gChosenOutputID == pkt->u.nodeDeleted.uniqueID) 
		      {
		      gOutNodeRefNum = -1;	// invalid
		      }
		      break;
		      case omsMsgNodesChanged:
		      gNodesChanged = TRUE;
		      break;
		    */
		}
	    theRefCon = SetGlobalsRegister(theRefCon);		/* restore previous A5 */
	}
}

// OMSMIDIPacket
// MIDIPacket

static pascal void OMS_ReadHook(OMSPacket *pkt, long theRefCon)
{
    XDeviceControl	*pDevice;

    if (!theRefCon)	// indicates we were called from Beatnik Driver (assumes PowerPC/CFM)
	{
	    pDevice = (XDeviceControl *)omsGlobalReference;
	}
    else
	{
	    pDevice = (XDeviceControl *)theRefCon;
	}
    if (pDevice)
	{
	    theRefCon = SetGlobalsRegister(pDevice->globalsVariable);		/* Set current A5 */
	    PV_ProcessExternalMidiEvent(pDevice->pDirect, pkt->data[0], pkt->data[1], pkt->data[2]);
	    theRefCon = SetGlobalsRegister(theRefCon);		/* restore previous A5 */
	}
}

static pascal void OMS_ReadHook2(OMSMIDIPacket *pkt, long theRefCon)
{
    XDeviceControl	*pDevice;

    if (!theRefCon)	// indicates we were called from Beatnik Driver (assumes PowerPC/CFM)
	{
	    pDevice = (XDeviceControl *)omsGlobalReference;
	}
    else
	{
	    pDevice = (XDeviceControl *)theRefCon;
	}
    if (pDevice)
	{
	    theRefCon = SetGlobalsRegister(pDevice->globalsVariable);		/* Set current A5 */

	    /*	
		Process the MIDI packet as efficiently as possible.  It is guaranteed to be MIDI
		data, not some high-level event.  The applicationÕs refCon (appRefCon) that was 
		passed to OMSOpenConnection is in the low order word of pkt->tStamp. 
		A convenient way for an application to determine the source of the MIDI data is for 
		it to pass a number identifying the source as the appRefCon to OMSOpenConnection.
		The high-order word of pkt->tStemp is the sourceÕs ioRefNum (not its uniqueID); 
		applications can also look at this to determine the source of the MIDI data. 
	    */

	    PV_ProcessExternalMidiEvent(pDevice->pDirect, pkt->data[0], pkt->data[1], pkt->data[2]);
	    theRefCon = SetGlobalsRegister(theRefCon);		/* restore previous A5 */
	}
}

pascal OSErr PV_SelectorProc(OSType selector, long *response)
{
    selector = selector;
    *response = (long)omsDrvrReadHook;
    return 0;
}

static short PV_WriteNamesFile(OMSUniqueID deviceID, FSSpec *outFileSpec)
{
    short prefVRefNum;
    long prefDirID,count;
    static Byte *namesFileName = "\pBeatnik Names";
    Byte *filler = "\pFiller";
    short i,theErr, refNum, nameCount;
    Str255 resName,pnBuf;
    HAEAudioMixer	*pMixer;

    // assume the Folder Manager is around, put the name file in the Preferences folder
    theErr = FindFolder(kOnSystemDisk,kPreferencesFolderType,FALSE,&prefVRefNum,&prefDirID);
    if (!theErr) {
	outFileSpec->vRefNum = prefVRefNum;
	outFileSpec->parID = prefDirID;
	XBlockMove(namesFileName, outFileSpec->name, namesFileName[0] + 1);
	theErr = FSpCreate(outFileSpec,MIDI_CLIENT_ID,NAMES_FILE_TYPE,smRoman);
	if (theErr && theErr != dupFNErr)
	    return theErr;
	theErr = FSpOpenDF(outFileSpec,fsCurPerm,&refNum);
	if (theErr)
	    return theErr;
		
	// write file, uniqueID followed by count of names
	count = sizeof(OMSUniqueID);
	FSWrite(refNum,&count,&deviceID);
	count = sizeof(short);
	nameCount = MAX_INSTRUMENTS; // eventually more
	//		nameCount = MAX_INSTRUMENTS * MAX_BANKS; // use this one to get all names
	FSWrite(refNum,&count,&nameCount);
	for (i=0; i < 32; i++)
	    pnBuf[i] = 0;
	// stupid test format: nameCount 32-byte pascal strings with zero padding
		
	// now pull all the names from the currently open instrument file from the mixer
	if (globalDeviceControl)
	    {
		pMixer = globalDeviceControl->pMixer;
		if (pMixer)
		    {
			for (i = 0; i < nameCount; i++) 
			    {
				// get name in "C" string format
				if (pMixer->GetInstrumentNameFromAudioFileFromID((char *)resName, i) == HAE_NO_ERROR)
				    {
					XCtoPstr(resName);
					if (resName[0] > 31)
					    {
						resName[0] = 31;
					    }
					BlockMove(resName, pnBuf, resName[0] + 1);
				    }
				else
				    {
					BlockMove(filler, pnBuf, filler[0] + 1);
				    }
				count = 32;		// always write 32 bytes
				FSWrite(refNum,&count,pnBuf);
			    }
		    }
	    }
	// close out the file
	GetFPos(refNum,&count);
	SetEOF(refNum,count);
	FSClose(refNum);
	theErr = noErr;
    }
    return theErr;
}

static short int PV_ConnectOMS(void *reference, void *omsPortName, Boolean connectWithNames)
{
    short int			theErr;
    OMSConnectionParams	conn;
    OMSIDListH			theList;
    OMSFile				nameFileSpec;
    Boolean				oms20Avail;
    OMSReadHook2UPP		*val;
    long				drvrUniqueID;
    void				*preFlight;

    preFlight = (void *)NewPtr(1024 * 256);	// try and allocate 256k of memory
    if (preFlight == NULL)
	{	// OMS will fail, but doesn't report memory failures safely
	    return HAE_MEMORY_ERR;
	}
    else
	{
	    DisposePtr((char *)preFlight);
	}
    theErr = noErr;
    LinkToOMSGlue();
    if (OMSVersion())
	{
	    // Use OMS
	    omsAppHook = NewOMSAppHook(OMS_AppHook);

	    usedOMS = TRUE;
	    gCompatMode = TRUE;
	    gChosenInputID = 0;
	    gDriverAvailable = FALSE;
	    usedOMSNameManager = FALSE;
	    oms20Avail = ((OMSVersion() >> 16L) >= 0x200L) ? TRUE : FALSE;
	    theErr = OMSSignIn(MIDI_CLIENT_ID, (long)reference, LMGetCurApName(), omsAppHook, &gCompatMode);
	    if (theErr == omsNoErr)
		{
		    // check and see if the driver got installed
		    theErr = Gestalt(kBeatnikDriverSelector,(long *)&val);
		    if (theErr == noErr) 
			{
			    gDriverAvailable = TRUE;
			    omsGlobalReference = reference;
			    omsDrvrReadHook = NewOMSReadHook(OMS_ReadHook);
			    *val = (OMSReadHook2UPP)omsDrvrReadHook;
			    Gestalt(kBeanikDriverUniqueIDSelector,&drvrUniqueID);
			    omsUniqueID = drvrUniqueID;
			}
		    registeredMIDISignOn = TRUE;
		    midiInputRef = 0;
		    /*	Add an input port */
		    if (oms20Avail == FALSE)	// pre 2.0 OMS, so ask for a IAC port
			{
			    omsReadHook = (OMSReadHook2UPP)NewOMSReadHook(OMS_ReadHook);
				// if no driver, we'll do the IAC port
			    if (gDriverAvailable == FALSE) 
				{
				    theErr = OMSAddPort(MIDI_CLIENT_ID, MIDI_INPUT_ID, omsPortTypeInput, omsReadHook, 
							(long)reference, &midiInputRef);
				    if (theErr == omsNoErr)
					{
					    InitCursor();	// set arrow cursor for pending dialog box
					    theList = OMSChooseNodes(NULL, "\pSelect one OMS input device:", (OMSBool)FALSE, 
								     omsIncludeOutputs + omsIncludeReal + omsIncludeVirtual + omsIncludeSecret,
								     NULL); 
					    if (theList)
						{
						    gChosenInputID = (**theList).id[0];
						    conn.nodeUniqueID = gChosenInputID;
						    conn.appRefCon = 0;
						    theErr = OMSOpenConnections(MIDI_CLIENT_ID, MIDI_INPUT_ID, 1, &conn, FALSE);
						    DisposeHandle((Handle)theList);
						}
					}
				    else
					{
					    goto error;
					}
				}
			}
		    else
			{
			    if (connectWithNames)
				{
				    // sign into the Name Manager
				    theErr = OMSNSignIn(MIDI_CLIENT_ID);
				    if (theErr != omsNoErr)
					{
					    goto error;
					}
				    usedOMSNameManager = TRUE;
				}
			    if (gDriverAvailable == FALSE) 
				{  // if no Beatnik driver, create a virtual node
				    omsReadHook = NewOMSReadHook2(OMS_ReadHook2);
				    theErr = OMSCreateVirtualDestination(MIDI_CLIENT_ID, MIDI_INPUT_ID,
									 (OMSStringPtr)omsPortName,
									 &omsUniqueID,
									 &midiInputRef,
									 omsReadHook,
									 (long)reference, 0,
									 0, NULL);
				    if (theErr != omsNoErr)
					{
					    goto error;
					}
				}

				// write out a file containing the patch names and uniqueID
			    if (connectWithNames)
				{
				    theErr = PV_WriteNamesFile(omsUniqueID,&nameFileSpec); // additional args later
				
				    // tell OMS about this file
				    theErr = OMSNProviderDocSaved(omsUniqueID,&nameFileSpec);
				    if (gDriverAvailable)
					{
					    theErr = OMSNProvideDevice(omsUniqueID, MIDI_CLIENT_ID, &nameFileSpec, omsUniqueID);
					}
				}
			}
		}
	}
    else
	{
	error:
	    theErr = HAE_OMS_FAILED;
	}
    return theErr;
}

static void PV_CleanupOMS(void)
{
    OMSConnectionParams	conn;
    long *val;
    short theErr;
	
    if (registeredMIDISignOn)
	{
	    if (gDriverAvailable == FALSE)
		{
		    if ( (OMSVersion() >> 16L) < 0x200L)	// pre 2.0 OMS, so ask for a IAC port
			{
			    conn.nodeUniqueID = gChosenInputID;
			    conn.appRefCon = 0;		/* not used in this example */
			    OMSCloseConnections(MIDI_CLIENT_ID, MIDI_INPUT_ID, 1, &conn);
			}
		    else
			{
			    OMSDeleteVirtualNode(MIDI_CLIENT_ID, MIDI_INPUT_ID);
			}
		} 
	    else 
		{
		    theErr = Gestalt(kBeatnikDriverSelector,(long *)&val);
		    if (theErr == noErr)
			{
			    *val = NULL;  // clear out driver
			}
		    if (omsDrvrReadHook)
			{
			    DisposeRoutineDescriptor((RoutineDescriptor *)omsDrvrReadHook);
			    omsDrvrReadHook = NULL;
			}
		    gDriverAvailable = FALSE;
		}
	    if (usedOMSNameManager)
		{
		    OMSNSignOut(MIDI_CLIENT_ID);
		}
	    OMSSignOut(MIDI_CLIENT_ID);
	    if (omsAppHook)
		{
		    DisposeRoutineDescriptor((RoutineDescriptor *)omsAppHook);
		}
	    if (omsReadHook)
		{
		    DisposeRoutineDescriptor((RoutineDescriptor *)omsReadHook);
		}
	}
}

// Connection through the MIDI Manager
static pascal short PV_MidiManagerReadHook(MIDIPacketPtr pkt, long theRefCon)
{
    XDeviceControl	*pDevice;

    pDevice = (XDeviceControl *)theRefCon;
    if (pDevice)
	{
	    theRefCon = SetGlobalsRegister(pDevice->globalsVariable);		/* Set current A5 */
	    PV_ProcessExternalMidiEvent(pDevice->pDirect, pkt->data[0], pkt->data[1], pkt->data[2]);
	    theRefCon = SetGlobalsRegister(theRefCon);		/* restore previous A5 */
	}
    return midiMorePacket;
}

static short int PV_ConnectMidiManager(void *reference, char *portName)
{
    MIDIPortParams	theInit;
    short int		theErr;

    theErr = HAE_NO_ERROR;

    theErr = MIDISignIn(MIDI_CLIENT_ID, (long)reference, GetResource('ICN#', 128), LMGetCurApName());
    if (theErr == HAE_NO_ERROR)
	{
	    usedMIDIManager = TRUE;
	    registeredMIDISignOn = TRUE;
	    XSetMemory(&theInit, (long)sizeof(MIDIPortParams), 0);

	    theInit.portID = MIDI_INPUT_ID;
	    theInit.portType = midiPortTypeInput;
	    theInit.timeBase = 0;
	    theInit.offsetTime = midiGetCurrent;

	    mmReadHook = NewMIDIReadHookProc(PV_MidiManagerReadHook);
	    theInit.readHook = mmReadHook;
	    //		theInit.readHook = (MIDIReadHookProcPtr)mmReadHook;
	    theInit.initClock.syncType = midiInternalSync;

	    theInit.refCon = (long)reference;
	    theInit.initClock.curTime = 0;
	    theInit.initClock.format = midiFormatMSec;
	    BlockMoveData(portName, (void *)&theInit.name, (long)portName[0]+1);
	    midiInputRef = 0;
	    theErr = MIDIAddPort(MIDI_CLIENT_ID, MIDI_PORT_BUFFER_SIZE, &midiInputRef, &theInit);
	    if (theErr == noErr)
		{
		    registeredMIDIinPort = TRUE;
		}
	    else
		{
		    theErr = HAE_MIDI_MANAGER_FAILED;
		}
	}
    else
	{
	    theErr = HAE_MIDI_MANAGER_FAILED;
	}
    return theErr;
}

static void PV_CleanupMidiManager(void)
{
    if (registeredMIDIinPort)
	{
	    MIDIRemovePort(midiInputRef);
	}
    if (registeredMIDISignOn)
	{
	    MIDISignOut(MIDI_CLIENT_ID);
	}
    if (mmReadHook)
	{
	    DisposeRoutineDescriptor(mmReadHook);
	}
    mmReadHook = NULL;
}

static void PV_CleanupExternalMidi(void)
{
    if (externalMidiEnabled)
	{
	    externalMidiEnabled = FALSE;
	    if (usedMIDIManager)
		{
		    PV_CleanupMidiManager();
		}
	    if (usedOMS)
		{
		    PV_CleanupOMS();
		}
	}
    registeredMIDIinPort = FALSE;
    registeredMIDISignOn = FALSE;
    usedMIDIManager = FALSE;
    usedOMS = FALSE;
}


// Connect system to an external midi source. One of these types:
//		USE_OMS					-- use OMS
//		USE_OMS_NAME			-- use OMS and name manager
//		USE_MIDI_MANAGER		-- use Midi Manager

static short int PV_SetupExternalMidi(void *reference, short int connectionType)
{
    short int	theErr;
    char		portName[256];
    static char	midiManagerPortName[] = "\pInput";
    static char	defaultOMSPortName[] = "\pBeatnik";
    Handle		externalName;
    Boolean		connectWithNames;

    if (externalMidiEnabled)
	{
	    PV_CleanupExternalMidi();
	}
    usedMIDIManager = FALSE;
    usedOMS = FALSE;
    registeredMIDISignOn = FALSE;
    registeredMIDIinPort = FALSE;
    externalMidiEnabled = TRUE;
    theErr = HAE_MIDI_MANGER_NOT_THERE;
    if ((connectionType == USE_OMS) || (connectionType == USE_OMS_NAME))
	{
	    connectWithNames = (connectionType == USE_OMS_NAME) ? TRUE : FALSE;
	    externalName = GetResource('STR ', 32000);
	    if (externalName)
		{
		    HLock(externalName);
		    XBlockMove(*externalName, portName, *externalName[0] + 1);
		    HUnlock(externalName);
		}
	    else
		{
		    XStrCpy(portName, defaultOMSPortName);
		}			

	    theErr = PV_ConnectOMS(reference, portName, connectWithNames);
	}
    if (connectionType == USE_MIDI_MANAGER)
	{
	    externalName = GetResource('STR ', 32001);
	    if (externalName)
		{
		    HLock(externalName);
		    XBlockMove(*externalName, portName, *externalName[0] + 1);
		    HUnlock(externalName);
		}
	    else
		{
		    XStrCpy(portName, midiManagerPortName);
		}			
	    theErr = PV_ConnectMidiManager(reference, portName);
	}
    if (theErr)
	{
	    PV_CleanupExternalMidi();
	}
    return theErr;
}
#endif	// X_PLATFORM == X_MACINTOSH



// Class implemention for HAEMidiExternal

//#pragma mark (HAEMidiExternal class)

HAEMidiExternal::HAEMidiExternal(HAEAudioMixer *pHAEAudioMixer,
				 char *pName,
				 void * userReference):
    HAEMidiDirect(pHAEAudioMixer, 
		  pName, userReference)
{
    XDeviceControl	*pDevice;

    //	SetTimeBaseOffset((XMicroseconds() - GM_GetSyncTimeStamp()) + BASE_TIME_OFFSET);
    SetTimeBaseOffset(BASE_TIME_OFFSET);

    // most of the real variables get setup when the HAEMidiDirect
    // gets created
    controlVariables = XNewPtr((long)sizeof(XDeviceControl));
    pDevice = (XDeviceControl *)controlVariables;
    if (pDevice)
	{
#if X_PLATFORM == X_MACINTOSH
	    pDevice->globalsVariable = GetGlobalsRegister();
#endif
	    pDevice->pDirect = this;
	    pDevice->pMixer = pHAEAudioMixer;
	}
    globalDeviceControl = pDevice;
    MusicGlobals->enableDriftFixer = TRUE;
}

HAEMidiExternal::~HAEMidiExternal()
{
    Stop();
    XDisposePtr(controlVariables);
    globalDeviceControl = NULL;
    MusicGlobals->enableDriftFixer = FALSE;
}

HAEErr HAEMidiExternal::Start(HAE_BOOL loadInstruments,
			      HAEConnection connect,
			      long midiReference)
{
    HAEErr			theErr;
    short			pv_connect;
    short int		moreError;
    XDeviceControl *pDevice;

    pDevice = (XDeviceControl *)controlVariables;
    pv_connect = -1;
    switch (connect)
	{
	case HAE_OMS_NAME_CONNECT:
	case HAE_OMS_CONNECT:
	    pv_connect = USE_OMS;
	    break;
	    //		case HAE_OMS_NAME_CONNECT:
	    //			pv_connect = USE_OMS_NAME;
	    //			break;
	case HAE_MIDI_MANAGER_CONNECT:
	    pv_connect = USE_MIDI_MANAGER;
	    break;
	}
    theErr = HAE_PARAM_ERR;
    if (pv_connect != -1)
	{
	    theErr = Open(loadInstruments);
	    reference = midiReference;
	    SetQueue(TRUE);
	    if (theErr == HAE_NO_ERROR)
		{
#if X_PLATFORM == X_MACINTOSH
		    if (externalMidiEnabled == FALSE)
			{
			    moreError = PV_SetupExternalMidi(controlVariables, pv_connect);
			    theErr = (HAEErr)moreError;
			}
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    short int	wNumDevices, count, validMidi;
		    MIDIINCAPS	theCaps;
		   
		    theErr = (HAEErr)HAE_MIDI_MANAGER_FAILED;
		    wNumDevices = midiInGetNumDevs();
		    if (wNumDevices)
			{
			    if (wNumDevices > MAX_MIDI_INPUTS)
				{
				    wNumDevices = MAX_MIDI_INPUTS;
				}

			    validMidi = FALSE;
				// cycle through midi input devices and open them all
			    for (count = 0; count < wNumDevices; count++)
				{
				    moreError = midiInGetDevCaps(count, (LPMIDIINCAPS) &theCaps, sizeof(MIDIINCAPS));
				    if (moreError == 0)
					{
					    moreError = midiInOpen(&pDevice->midiInputHandle[count],
								   count,					// use default mapping
								   (DWORD)PV_MidiInputHandler,
								   (DWORD)this,
								   CALLBACK_FUNCTION);
					}
				}
				// start midi input and reset time stamps
			    for (count = 0; count < wNumDevices; count++)
				{
				    moreError = midiInStart(pDevice->midiInputHandle[count]); 
				}
			    theErr = HAE_NO_ERROR;
			}
#endif
		}
	}
    return theErr;
}

void HAEMidiExternal::Stop(void)
{
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
    XDeviceControl *pDevice;
    short int		count, devs;

    pDevice = (XDeviceControl *)controlVariables;

    devs = midiInGetNumDevs();
    if (devs)
	{
	    for (count = 0; count < devs; count++)
		{
		    midiInClose(pDevice->midiInputHandle[count]);
		}
	}	
#endif
#if X_PLATFORM == X_MACINTOSH
    if (externalMidiEnabled)
	{
	    PV_CleanupExternalMidi();
	}
#endif
    globalDeviceControl = NULL;
    Close();
}

long HAEMidiExternal::GetMidiReference(void)
{
    return reference;
}

long HAEMidiExternal::GetTimeBaseOffset(void)
{
    return timeBaseOffset;
}


void HAEMidiExternal::SetTimeBaseOffset(long newTimeBase)
{
    timeBaseOffset = newTimeBase;
}


// EOF of HAEMidiExternal.cpp

