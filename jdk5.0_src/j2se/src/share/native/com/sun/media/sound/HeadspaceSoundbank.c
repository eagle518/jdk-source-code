/*
 * @(#)HeadspaceSoundbank.c	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// STANDARD includes


// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/X_API.h"
#include "engine/X_Formats.h"

// UTILITY includes
#include "Utilities.h"																											  

// HeadspaceSoundbank includes
#include "com_sun_media_sound_HeadspaceSoundbank.h"


// $$kk: 04.11.99: we are never calling XFileClose!!
JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResource(JNIEnv* e, jobject thisObj, jstring path) 
{
    XFILE file = NULL;
    XFILENAME xfilename;

    const char *str = (*e)->GetStringUTFChars(e, path, 0);

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResource.\n");

    XConvertNativeFileToXFILENAME((void *)str, &xfilename);
    file = XFileOpenResource(&xfilename, TRUE);

    (*e)->ReleaseStringUTFChars(e, path, str);

    TRACE1("Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResource completed, returning %lu.\n", file);

    return (jlong) (INT_PTR) file;
}

JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResourceFromByteArray(JNIEnv* e, jobject thisObj, jbyteArray bankData, jint length) 
{
    XFILE file = NULL;
    void	*pData = NULL;

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResourceFromByteArray.\n");

    pData = XNewPtr(length); // can have memory error

    if (!pData)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResourceFromByteArray failed to allocate memory for soundbank\n");
	    return 0;
	}

    (*e)->GetByteArrayRegion(e, bankData, (jint)0, (jint)length, (jbyte*)pData);

    file = XFileOpenResourceFromMemory(pData, length, TRUE);

    if (!file)
	{
	    ERROR0("Failed to create resource file from data \n");
	    XDisposePtr(pData);
	    return 0;
	}

    TRACE1("Java_com_sun_media_sound_HeadspaceSoundbank_nOpenResourceFromByteArray completed, returning %lu.\n", file);

    return (jlong) (INT_PTR) file;
}

// $$kk: 04.11.99: we are never calling this!!
JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nCloseResource(JNIEnv* e, jobject thisObj, jlong id) 
{
    XFILE file = NULL;
    jboolean result;

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nCloseResource.\n");

    file = (XFILE) (INT_PTR) id;
    if (file)
	{
	    XFileClose(file);
	    result = (jboolean)TRUE;
	}
    else 
	{
	    result = (jboolean)FALSE;
	}

    TRACE1("Java_com_sun_media_sound_HeadspaceSoundbank_nCloseResource completed, returning %d.\n", result);
    return result;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nGetName(JNIEnv* e, jobject thisObj, jlong id)
{
    XFILE oldResourceFile = NULL;
    XFILE thisResourceFile = NULL;

    BankStatus	bank;
    char name[BANK_NAME_MAX_SIZE];
    name[0] = 0;

    oldResourceFile = XFileGetCurrentResourceFile();
    thisResourceFile = (XFILE) (INT_PTR) id;

    if (thisResourceFile)
	{
	    XFileUseThisResourceFile(thisResourceFile);
	    XGetBankStatus(&bank);

	    if ((UINT32)XStrLen(bank.bankName) == 0)
		{
		    XFileUseThisResourceFile(oldResourceFile);
		    return NULL;
		}
	    else
		{
		    XStrCpy(name, bank.bankName);
		    XFileUseThisResourceFile(oldResourceFile);
		    return (*e)->NewStringUTF(e, name);
		}
	}

    return NULL;
}



JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nGetVersionMajor(JNIEnv* e, jobject thisObj, jlong id)
{
    XFILE oldResourceFile = NULL;
    XFILE thisResourceFile = NULL;

    XVersion	vers;

    short int	versionMajor = 0;

    oldResourceFile = XFileGetCurrentResourceFile();
    thisResourceFile = (XFILE) (INT_PTR) id;

    if (thisResourceFile)
	{
	    XFileUseThisResourceFile(thisResourceFile);
	    XGetVersionNumber(&vers);
	    versionMajor = vers.versionMajor;
	    XFileUseThisResourceFile(oldResourceFile);
	}
		
    return (jint)versionMajor;
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nGetVersionMinor(JNIEnv* e, jobject thisObj, jlong id)
{
    XFILE oldResourceFile = NULL;
    XFILE thisResourceFile = NULL;

    XVersion	vers;

    short int	versionMinor = 0;

    oldResourceFile = XFileGetCurrentResourceFile();
    thisResourceFile = (XFILE) (INT_PTR) id;

    if (thisResourceFile)
	{
	    XFileUseThisResourceFile(thisResourceFile);
	    XGetVersionNumber(&vers);
	    versionMinor = vers.versionMinor;
	    XFileUseThisResourceFile(oldResourceFile);
	}
		
    return (jint)versionMinor;
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nGetVersionSubMinor(JNIEnv* e, jobject thisObj, jlong id)
{
    XFILE oldResourceFile = NULL;
    XFILE thisResourceFile = NULL;

    XVersion	vers;

    short int	versionSubMinor = 0;

    oldResourceFile = XFileGetCurrentResourceFile();
    thisResourceFile = (XFILE) (INT_PTR) id;

    if (thisResourceFile)
	{
	    XFileUseThisResourceFile(thisResourceFile);
	    XGetVersionNumber(&vers);
	    versionSubMinor = vers.versionSubMinor;
	    XFileUseThisResourceFile(oldResourceFile);
	}
		
    return (jint)versionSubMinor;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments(JNIEnv* e, jobject thisObj, jlong id, jobject instruments)
{
    int index = 0;			// current index as we iterate through the set of instruments in the bank
    int nameLength;

    // variables for vector operations
    jclass vectorClass;
    jmethodID addElementMethodID;

    // variables for java instrument manipulations
    jclass instrumentClass;
    jmethodID initMethodID;
    jobject newInstrumentObject;
    jstring newName;

    // variables for the XGetIndexedFileResource call
    XPTR		pData;
    INT32		instrumentSize;
    XLongResourceID	instrumentId;
    char		instrumentName[BANK_NAME_MAX_SIZE]; //$$kk: what should the size be??

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments\n");

	
    // get the vector stuff set up

    vectorClass = (*e)->GetObjectClass(e, instruments);

    if (vectorClass == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments: vectorClass is NULL\n");
	    return;
	}

    addElementMethodID = (*e)->GetMethodID(e, vectorClass, "addElement", "(Ljava/lang/Object;)V");

    if (addElementMethodID == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments: addElementMethodID is NULL\n");
	    return;
	}

	
    // get the HeadspaceInstrument class, init method id, etc.

    instrumentClass = (*e)->FindClass(e, "com/sun/media/sound/HeadspaceInstrument");

    if (instrumentClass == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments: instrumentClass is NULL\n");
	    return;
	}

    initMethodID = (*e)->GetMethodID(e, instrumentClass, "<init>", "(Lcom/sun/media/sound/HeadspaceSoundbank;Ljava/lang/String;II)V");

    if (initMethodID == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments: initMethodID is NULL\n");
	    return;
	}


    while (TRUE)
	{
	    // get the next instrument
	    pData = XGetIndexedFileResource((XFILE) (INT_PTR) id, ID_INST, &instrumentId, index, instrumentName, &instrumentSize);

	    if (pData == NULL)
		{
		    /*  done getting instruments */
		    break;
		}
		
	    XPtoCstr(instrumentName);
	    // fix for 4429762: Some instrument names in some soundbanks include bad extra characters
	    // since XPtoCstr() modifies the contents of instrumentName[], we can do so, too!
	    nameLength = 0;
	    while(instrumentName[nameLength]) nameLength++;
	    while(nameLength > 0 && instrumentName[nameLength-1] < 32) {
	    	instrumentName[nameLength-1] = 0;
	    	nameLength--;
	    }
	    newName = (*e)->NewStringUTF(e, instrumentName);

	    // create a HeadspaceInstrument object
	    newInstrumentObject = (*e)->NewObject(e, instrumentClass, initMethodID, thisObj, (jstring)newName, (jint)instrumentId, (jint)instrumentSize);

	    if (newInstrumentObject == NULL)
		{
		    ERROR1("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments: Failed to get instantiate HeadspaceInstrument object for instrument id %lu.\n", instrumentId);
		}
	    else
		{
		    // add it to the vector
		    (*e)->CallVoidMethod(e, instruments, addElementMethodID, newInstrumentObject);
		}

	    index++;
	}

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetInstruments completed\n");
    return;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples(JNIEnv* e, jobject thisObj, jlong id, jobject samples)
{
    int index = 0;			// current index as we iterate through the set of samples in the bank

    // variables for vector operations
    jclass vectorClass;
    jmethodID addElementMethodID;

    // variables for java sample manipulations
    jclass sampleClass;
    jmethodID initMethodID;
    jobject newSampleObject;
    jstring newName;

    // variables for the XGetIndexedFileResource call
    XPTR		pData;
    INT32		sampleSize;
    XLongResourceID	sampleId;
    char		sampleName[BANK_NAME_MAX_SIZE]; //$$kk: what should the size be??

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples\n");

	
    // get the vector stuff set up

    vectorClass = (*e)->GetObjectClass(e, samples);

    if (vectorClass == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples: vectorClass is NULL\n");
	    return;
	}

    addElementMethodID = (*e)->GetMethodID(e, vectorClass, "addElement", "(Ljava/lang/Object;)V");

    if (addElementMethodID == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples: addElementMethodID is NULL\n");
	    return;
	}

	
    // get the HeadspaceSample class, init method id, etc.

    sampleClass = (*e)->FindClass(e, "com/sun/media/sound/HeadspaceSample");

    if (sampleClass == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples: sampleClass is NULL\n");
	    return;
	}

    initMethodID = (*e)->GetMethodID(e, sampleClass, "<init>", "(Lcom/sun/media/sound/HeadspaceSoundbank;Ljava/lang/String;III)V");

    if (initMethodID == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples: initMethodID is NULL\n");
	    return;
	}


    while (TRUE)
	{
	    // get the next instrument
	    pData = XGetIndexedFileResource((XFILE) (INT_PTR) id, ID_INST, &sampleId, index, sampleName, &sampleSize);


	    // look for compressed version first
	    pData = XGetIndexedFileResource((XFILE) (INT_PTR) id, ID_CSND, &sampleId, index, sampleName, &sampleSize);
	    if (pData == NULL)
		{
		    // look for standard version
		    pData = XGetIndexedFileResource((XFILE) (INT_PTR) id, ID_SND, &sampleId, index, sampleName, &sampleSize);

		    if (pData == NULL)
			{
				// look for encrypted version
			    pData = XGetIndexedFileResource((XFILE) (INT_PTR) id, ID_ESND, &sampleId, index, sampleName, &sampleSize);
			}
		}

	    if (pData == NULL)
		{
		    /*  done getting instruments */
		    break;
		}
		
	    XPtoCstr(sampleName);
	    newName = (*e)->NewStringUTF(e, sampleName);

	    // create a HeadspaceSample object
	    newSampleObject = (*e)->NewObject(e, sampleClass, initMethodID, thisObj, (jstring)newName, (jint)index, (jint)sampleId, (jint)sampleSize);

	    if (newSampleObject == NULL)
		{
		    ERROR1("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples: Failed to get instantiate HeadspaceSample object for sample id %lu.\n", sampleId);
		}
	    else
		{
		    // add it to the vector
		    (*e)->CallVoidMethod(e, samples, addElementMethodID, newSampleObject);
		}

	    index++;
	}

    TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nGetSamples completed\n");
    return;
}


/*
  // set the soundbank
  // returns true for success, false for failure.
  JNIEXPORT jboolean JNICALL
  Java_com_sun_media_sound_HeadspaceSoundbank_nSetCurrentResource(JNIEnv* e, jobject thisObj, jlong id)
  {
  XFILE file = NULL;
  jboolean result;

  TRACE0("Java_com_sun_media_sound_HeadspaceSoundbank_nSetCurrentResource.\n");

  file = (XFILE)id;
  if (file)
  {
  XFileUseThisResourceFile(file);
  result = (jboolean)TRUE;
  }
  else 
  {
  result = (jboolean)FALSE;
  }

  TRACE1("Java_com_sun_media_sound_HeadspaceSoundbank_nSetCurrentResource completed, returning %d.\n", result);
  return result;
  }
*/
