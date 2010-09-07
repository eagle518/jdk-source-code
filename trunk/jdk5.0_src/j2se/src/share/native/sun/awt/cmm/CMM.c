/*
 * @(#)CMM.c	1.21 04/02/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* File:  CMM.c  @(#)CMM.c	1.7  11/03/97
 * Native functions for the Java CMM
 */

/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/


#include <stdio.h>
#include "sun_awt_color_CMM.h"
#include "sprofile.h"
#include "sprof-pr.h"


#define JAVAsearchCMM (1)
#define JAVAsearchVersion (2)
#define JAVAsearchClass (3)
#define JAVAsearchColorSpace (4)
#define JAVAsearchPCS (5)
#define JAVAsearchBeforeDate (6)
#define JAVAsearchOnDate (7)
#define JAVAsearchAfterDate (8)
#define JAVAsearchPlatform (9)
#define JAVAsearchFlags (10)
#define JAVAsearchManufacturer (11)
#define JAVAsearchModel (12)
#define JAVAsearchAttributes (13)
#define JAVAsearchRenderingIntent (14)
#define JAVAsearchIlluminant (15)
#define JAVAsearchCreator (16)

#define JAVAsearchSize (16)
#define JAVAsearchTrue (1)
#define JAVAsearchFalse (0)

#define JAVA_ICC_TransformAny (-1)

#define JAVA_TYPE_INT_RGB (1)
#define JAVA_TYPE_INT_ARGB (2)
#define JAVA_TYPE_INT_BGR (4)
#define JAVA_TYPE_3BYTE_BGR (5)
#define JAVA_TYPE_4BYTE_ABGR (6)

#define typeCompUByte (256)
#define typeCompUShort12 (257)
#define typeCompUShort (258)
#define typePelUByte (259)
#define typePelUShort12 (260)
#define typePelUShort (261)
#define typeIntRGBPacked (265)

#define SpTagHeader    SpTagIdConst('h', 'e', 'a', 'd')

/* Rendering Intents, used in the profile header */
typedef enum {
    icPerceptual            = 0,
    icRelativeColorimetric        = 1,
    icSaturation            = 2,
    icAbsoluteColorimetric        = 3,
    icMaxEnumIntent            = 0xFFFFFFFF    /* enum = 4 bytes max */
} icRenderingIntent;

typedef    jbyte* jbyte_p;
typedef    jshort* jshort_p;
typedef    jint* jint_p;
typedef    jlong* jlong_p;
typedef    SpXform_t* SpXform_p;
typedef    SpProfile_t* SpProfile_p;
typedef    SpPixelLayout_t* SpPixelLayout_p;

typedef union storeID_s {    /* store SProfile stuff in a Java Long */
    SpCallerId_t    cid;
    SpProfile_t        pf;
    SpXform_t        xf;
    jlong            j;
} storeID_t, *storeID_p;

typedef struct {
    int type; /* 0 for byte, 1 for short, 2 for int */
    jarray array;
    void* addr;
} jarrayinfo_t;

typedef struct {
    int count;
    jarrayinfo_t info[(SpMaxComponents + 1) * 2];
} jarraymap_t;

static SpCallerId_t getCallerID (JNIEnv*);
static SpStatus_t initImageLayouts(JNIEnv*, jobject, SpPixelLayout_p,
                                   jobject, SpPixelLayout_p, jarraymap_t*);
static void releaseArrayData(JNIEnv*, jarraymap_t*);
static void pfToID (JNIEnv*, jlongArray, SpProfile_p, KpInt32_t);
static SpStatus_t criteriaFromHeader (JNIEnv*, jbyteArray, jbyteArray, SpSearch_t*, SpSearchCriterion_t*);
static void returnInt (JNIEnv *env, jintArray theReturn, jint theInt);
static jint checkStatus (SpStatus_t theStatus);
static void setObjectID (JNIEnv *env, jobject theTransform, storeID_t theXform);
static SpStatus_t setClassID (JNIEnv *env, jclass theClass, SpCallerId_t theID);
static SpCallerId_t getClassID (JNIEnv *env, jclass theClass);

#define DIAG_PRINTF1(x) ((void)0)
#define DIAG_PRINTF2(x,y) ((void)0)
#define DIAG_PRINTF3(x,y,z) ((void)0)
#define DIAG_PRINTF4(x,y,z,a) ((void)0)
/* #define DIAG_PRINTF1 printf */
/* #define DIAG_PRINTF2 printf */
/* #define DIAG_PRINTF3 printf*/
/* #define DIAG_PRINTF4 printf*/
/* #define DIAG_PRINTF printf */
/* #define DIAG_PRINTF dummyPrintf */ /* */
/* static void dummyPrintf (const char * control, ...) {if (control) {}} */ /* */



/* Class:     sun_awt_color_CMM
 * Method:    cmmInit
 * Signature: ()I
 * static native int cmmInit();
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmInit (JNIEnv *env, jclass class)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmInit\n");

    theStatus = SpInitialize (&theCallerId, NULL, NULL);

    if (theStatus == SpStatSuccess) {
        theStatus = setClassID (env, class, theCallerId);
    }

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmTerminate
 * Signature: ()I
 * static native int cmmTerminate ();
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmTerminate (JNIEnv *env, jclass class)
{
SpStatus_t    theStatus = SpStatSuccess, aStatus;
SpCallerId_t    theCallerId;

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmTerminate\n");

    theCallerId = getClassID (env, class);
    if (theCallerId != NULL) {
        theStatus = SpTerminate (&theCallerId);
    }

    theCallerId = NULL;
    aStatus = setClassID (env, class, theCallerId);

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmLoadProfile
 * Signature: ([B[J)I
 * static native int cmmLoadProfile (byte[] data, long[] profileID);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmLoadProfile (JNIEnv *env, jobject obj, jbyteArray data, jlongArray profileID)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
jbyte_p        dataP;
jlong_p        longP;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmLoadProfile\n");

    theSProfile.pf = NULL;

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    } else if (data == NULL) {
        theStatus = SpStatBadProfile;
    } else {
        dataP = (*env)->GetByteArrayElements (env, data, 0);    /* get the data address */

        theStatus = SpProfileLoadFromBuffer (theCallerId, dataP, &theSProfile.pf);

        (*env)->ReleaseByteArrayElements (env, data, dataP, 0);    /* don't need to hold data */

        if (theStatus == SpStatSuccess) {
            DIAG_PRINTF2("theSProfile %x\n", theSProfile.pf);
        }
    }

    longP = (*env)->GetLongArrayElements (env, profileID, 0);

    longP[0] = theSProfile.j;                    /* return the profile ID in the first element */

    (*env)->ReleaseLongArrayElements (env, profileID, longP, 0);

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmFreeProfile
 * Signature: (J)I
 * static native int cmmFreeProfile (long profileID);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmFreeProfile (JNIEnv *env, jobject obj, jlong profileID)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmFreeProfile\n");

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theSProfile.j = profileID;

        theStatus = SpProfileFree (&theSProfile.pf);
    }

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmGetProfileSize
 * Signature: (J[I)I
 * static native int cmmGetProfileSize (long profileID, int[] size);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmGetProfileSize (JNIEnv *env, jobject obj, jlong profileID, jintArray size)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
KpUInt32_t    profileSize = 0;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmGetProfileSize\n");

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theSProfile.j = profileID;

        theStatus = SpProfileGetProfileSize (theSProfile.pf, &profileSize);    /* get size of this profile */

        DIAG_PRINTF3("theSProfile %x; size %d\n", theSProfile.pf, profileSize);
    }

    returnInt (env, size, profileSize);

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmGetProfileData
 * Signature: (J[B)I
 * static native int cmmGetProfileData (long profileID, byte[] data);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmGetProfileData (JNIEnv *env, jobject obj, jlong profileID, jbyteArray data)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
jbyte_p        dataP;
KpUInt32_t    bufferSize;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmGetProfileData\n");

    theCallerId = getCallerID (env);    /* not actually needed, but must be there */
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theSProfile.j = profileID;
        DIAG_PRINTF2("theSProfile = %x\n", theSProfile.pf);

        dataP = (*env)->GetByteArrayElements(env, data, 0);                /* get the data address */
        bufferSize = (KpUInt32_t) (*env)->GetArrayLength (env, data);    /* and size */

        theStatus = SpProfileSaveToBuffer (theSProfile.pf, (KpChar_h)&dataP, &bufferSize);

        (*env)->ReleaseByteArrayElements (env, data, dataP, 0);            /* don't need to hold data */
    }

    return checkStatus (theStatus);
}

/*
 * Class:     sun_awt_color_CMM
 * Method:    cmmCullICC_Profiles
 * Signature: ([B[B[J[J[I)I
 * static native int cmmCullICC_Profiles (byte[] template, byte[] options, long[] sourceIDs, long[] matchIDs, int[] nMatch);
 */
JNIEXPORT jint JNICALL Java_sun_awt_color_CMM_cmmCullICC_1Profiles
  (JNIEnv *env, jobject obj, jbyteArray template, jbyteArray options, jlongArray sourceIDs, jlongArray matchIDs, jintArray nMatch)
{
SpStatus_t        theStatus;
SpCallerId_t    theCallerId;
storeID_t        theSProfile;
KpInt32_t        i1, srcSize;
jlong_p            javaIDlist;
SpProfile_p        srcProfiles;
KpInt32_t        foundCount = 0;
SpSearch_t        theSearch;
SpSearchCriterion_t    theCriteria [SPSEARCH_LIST];

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmCullICC_1Profiles\n");

    theCallerId = getCallerID (env);    /* not actually needed, but must be there */
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        srcSize = (KpUInt32_t) (*env)->GetArrayLength (env, sourceIDs);                    /* # of source profiles */
        srcProfiles = (SpProfile_p) allocBufferPtr (srcSize * sizeof (SpProfile_t));    /* get memory for it */

        if (srcProfiles == NULL) {
            theStatus = SpStatMemory;
        }
        else {
            javaIDlist = (*env)->GetLongArrayElements (env, sourceIDs, 0);

            DIAG_PRINTF1("srcProfiles =");
            for (i1 = 0; i1 < srcSize; i1++) {
                theSProfile.j = javaIDlist [i1];
                srcProfiles [i1] = theSProfile.pf;        /* get profile IDs from java IDs list */
                DIAG_PRINTF2(" %x", srcProfiles [i1]);
            }
            DIAG_PRINTF1("\n");
                
            (*env)->ReleaseLongArrayElements (env, sourceIDs, javaIDlist, 0);

            theStatus = criteriaFromHeader (env, template, options, &theSearch, theCriteria);

            if (theStatus == SpStatSuccess) {
                theStatus = SpProfileSearchRefine (&theSearch, srcProfiles, srcSize, &foundCount);

                pfToID (env, matchIDs, srcProfiles, foundCount);
            }

            freeBufferPtr ((KpGenericPtr_t) srcProfiles);            /* free profile list memory */
        }
    }

    returnInt (env, nMatch, foundCount);

    return checkStatus (theStatus);
}



/*
 * Class:     sun_awt_color_CMM
 * Method:    cmmFindICC_Profiles
 * Signature: (JLjava/lang/String;[J[I)I
 * static native int cmmFindICC_Profiles (byte[] template, byte[] options, String profileDir, long[] matchIDs, int[] nMatch);
 */
JNIEXPORT jint JNICALL Java_sun_awt_color_CMM_cmmFindICC_1Profiles
  (JNIEnv *env, jobject obj, jbyteArray template, jbyteArray options, jstring profileDir, jlongArray matchIDs, jintArray nMatch)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
KpUInt32_t    listSize;
SpProfile_p    profileList;
KpInt32_t    foundCount = 0;
SpDataBase_t        theDB;
SpDataBaseEntry_t    theDirectory;
SpSearch_t        theSearch;
SpSearchCriterion_t    theCriteria [SPSEARCH_LIST];

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmFindICC_1Profiles\n");

    theCallerId = getCallerID (env);        /* not actually needed, but must be there */
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        listSize = (KpUInt32_t) (*env)->GetArrayLength (env, matchIDs);                    /* get the max # of matched profiles */
        profileList = (SpProfile_p) allocBufferPtr (listSize * sizeof (SpProfile_t));    /* get memory for it */

        if (profileList == NULL) {
            theStatus = SpStatMemory;
        }
        else {
            /* make the DataBaseList */
            theDB.numEntries = 1;
            theDB.Entries = &theDirectory;
#if defined(KPMAC) || defined (KPMSMAC)
            theDirectory.fileProps.vRefNum = 0;
#endif
            theDirectory.dirName = (KpChar_p) (*env)->GetStringUTFChars(env, profileDir, 0);

            DIAG_PRINTF2("theDirectory.dirName = %s\n", &theDirectory.dirName);

            theStatus = criteriaFromHeader (env, template, options, &theSearch, theCriteria);

            if (theStatus == SpStatSuccess) {
                theStatus = SpProfileSearch (theCallerId, &theDB, &theSearch, profileList, listSize, &foundCount);

                pfToID (env, matchIDs, profileList, foundCount);
            }

            (*env)->ReleaseStringUTFChars (env, profileDir, theDirectory.dirName);    /* release directory name string */

            freeBufferPtr ((KpGenericPtr_t) profileList);                    /* free profile list memory */
        }
    }

    returnInt (env, nMatch, foundCount);

    return checkStatus (theStatus);
}



/* convert a profile's header into a list of criteria */
static SpStatus_t
    criteriaFromHeader (    JNIEnv*            env,
                jbyteArray        template,
                jbyteArray        options,
                SpSearch_t*        theSearch,
                SpSearchCriterion_t*    theCriteria)
{
SpStatus_t    theStatus;
SpHeader_t    theHeader;
KpInt32_t    critCount = 0;
jbyte_p        dataP, control;

    dataP = (*env)->GetByteArrayElements (env, template, 0);            /* get the template data */

    theStatus = SpHeaderToPublic ((KpInt8_p)dataP, HEADER_SIZE, &theHeader);    /* turn it into a header */

    (*env)->ReleaseByteArrayElements (env, template, dataP, 0);

    control = (*env)->GetByteArrayElements (env, options, 0);            /* get the search control data */

    if (theStatus == SpStatSuccess) {
        if (control[JAVAsearchCMM] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_PREFERREDCMM;
            theCriteria [critCount].SearchValue.Signature = theHeader.CMMType;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_PREFERREDCMM; %.4s\n", &theHeader.CMMType);
        }

        if (control[JAVAsearchVersion] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_VERSION;
            theCriteria [critCount].SearchValue.Value = theHeader.ProfileVersion;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_VERSION; %x\n", theHeader.ProfileVersion);
        }

        if (control[JAVAsearchClass] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_PROFILECLASS;
            theCriteria [critCount].SearchValue.Signature = theHeader.DeviceClass;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_PROFILECLASS; %.4s\n", &theHeader.DeviceClass);
        }

        if (control[JAVAsearchColorSpace] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_DEVICECOLORSPACE;
            theCriteria [critCount].SearchValue.Signature = theHeader.DataColorSpace;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_DEVICECOLORSPACE; %.4s\n", &theHeader.DataColorSpace);
        }

        if (control[JAVAsearchPCS] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_CONNECTIONSPACE;
            theCriteria [critCount].SearchValue.Signature = theHeader.InterchangeColorSpace;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_CONNECTIONSPACE; %.4s\n", &theHeader.InterchangeColorSpace);
        }

        if (control[JAVAsearchBeforeDate] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_BEFOREDATE;
            theCriteria [critCount].SearchValue.Date = theHeader.DateTime;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_BEFOREDATE; %lx\n", theHeader.DateTime);
        }

        if (control[JAVAsearchOnDate] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_ONDATE;
            theCriteria [critCount].SearchValue.Date = theHeader.DateTime;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_ONDATE; %lx\n", theHeader.DateTime);
        }

        if (control[JAVAsearchAfterDate] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_AFTERDATE;
            theCriteria [critCount].SearchValue.Date = theHeader.DateTime;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_AFTERDATE; %lx\n", theHeader.DateTime);
        }

        if (control[JAVAsearchPlatform] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_PLATFORM;
            theCriteria [critCount].SearchValue.Signature = theHeader.Platform;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_PLATFORM; %.4s\n", &theHeader.Platform);
        }

        if (control[JAVAsearchFlags] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_PROFILEFLAGS;
            theCriteria [critCount].SearchValue.Value = theHeader.Flags;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_PROFILEFLAGS; %x\n", theHeader.Flags);
        }

        if (control[JAVAsearchManufacturer] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_DEVICEMFG;
            theCriteria [critCount].SearchValue.Signature = theHeader.DeviceManufacturer;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_DEVICEMFG; %.4s\n", &theHeader.DeviceManufacturer);
        }

        if (control[JAVAsearchModel] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_DEVICEMODEL;
            theCriteria [critCount].SearchValue.Signature = theHeader.DeviceModel;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_DEVICEMODEL; %.4s\n", &theHeader.DeviceModel);
        }

        if (control[JAVAsearchAttributes] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_DEVICEATTRIBUTESHI;
            theCriteria [critCount].SearchValue.Value = theHeader.DeviceAttributes.hi;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_DEVICEATTRIBUTESHI; %x\n", theHeader.DeviceAttributes.hi);

            theCriteria [critCount].SearchElement = SPSEARCH_DEVICEATTRIBUTESLO;
            theCriteria [critCount].SearchValue.Value = theHeader.DeviceAttributes.lo;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_DEVICEATTRIBUTESLO; %x\n", theHeader.DeviceAttributes.lo);
        }

        if (control[JAVAsearchRenderingIntent] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_RENDERINGINTENT;
            theCriteria [critCount].SearchValue.Signature = theHeader.RenderingIntent;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_RENDERINGINTENT; %.4s\n", &theHeader.RenderingIntent);
        }

        if (control[JAVAsearchIlluminant] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_ILLUMINANT;
            theCriteria [critCount].SearchValue.XYZ.X = theHeader.Illuminant.X;
            theCriteria [critCount].SearchValue.XYZ.Y = theHeader.Illuminant.Y;
            theCriteria [critCount].SearchValue.XYZ.Z = theHeader.Illuminant.Z;
            critCount++;
            DIAG_PRINTF4("SPSEARCH_ILLUMINANT; %x %x %x\n", theHeader.Illuminant.X, theHeader.Illuminant.Y, theHeader.Illuminant.Z);
        }

        if (control[JAVAsearchCreator] == JAVAsearchTrue) {
            theCriteria [critCount].SearchElement = SPSEARCH_ORIGINATOR;
            theCriteria [critCount].SearchValue.Signature = theHeader.Originator;
            critCount++;
            DIAG_PRINTF2("SPSEARCH_ORIGINATOR; %.4s\n", &theHeader.Originator);
        }
    }

    (*env)->ReleaseByteArrayElements (env, options, control, 0);

    theSearch->critCount = critCount;
    theSearch->criterion = theCriteria;
    theSearch->critSize = SPSEARCH_LIST;

    return theStatus;
}


/* copy a list of profile IDs into a Java list of IDs */
static void pfToID (JNIEnv *env, jlongArray javaIDs, SpProfile_p profileList, KpInt32_t nIDs)
{
KpInt32_t    i1;
storeID_t    theSProfile;
jlong_p        javaIDlist;

    javaIDlist = (*env)->GetLongArrayElements (env, javaIDs, 0);

    DIAG_PRINTF1("match profiles =");
    for (i1 = 0; i1 < nIDs; i1++) {
        theSProfile.pf = profileList [i1];        /* copy profile IDs into java IDs list */
        javaIDlist [i1] = theSProfile.j;
        DIAG_PRINTF2(" %x", theSProfile.j);
    }
    DIAG_PRINTF1("\n");
            
    (*env)->ReleaseLongArrayElements (env, javaIDs, javaIDlist, 0);
}




/* Class:     sun_awt_color_CMM
 * Method:    cmmGetTagData
 * Signature: (JI[B)I
 * static native int cmmGetTagData (long profileID, int tagSignature, byte[] data);
 */
JNIEXPORT jint JNICALL Java_sun_awt_color_CMM_cmmGetTagData
  (JNIEnv *env, jobject obj, jlong profileID, jint tagSignature, jbyteArray data)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
KpHandle_t    sDataH;
jbyte_p        jDataP;
KpInt8_p    sDataP;
KpInt32_t    i1, sBufSize, jBufSize;
SpTagId_t    TagId;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmGetTagData\n");

    theCallerId = getCallerID (env);        /* not actually needed, but must be there */
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theSProfile.j = profileID;
        DIAG_PRINTF3("theSProfile = %x; tag = %.4s\n", theSProfile.pf, &tagSignature);

        TagId = (SpTagId_t)(size_t) tagSignature;

        jBufSize = (KpUInt32_t) (*env)->GetArrayLength(env, data);    /* get dest size */
        jDataP = (*env)->GetByteArrayElements (env, data, 0);        /* get the data address */

        if (TagId == SpTagHeader) {
            theStatus = SpRawHeaderGet (theSProfile.pf, jBufSize, jDataP);
        }
        else {
            theStatus = SpRawTagDataGet (theSProfile.pf, TagId, (KpUInt32_p)&sBufSize, &sDataH);
            if (theStatus == SpStatSuccess) {
                if (jBufSize < sBufSize) {
                    theStatus = SpStatBufferTooSmall;
                }
                else {
                    sDataP = (KpInt8_p) lockBuffer (sDataH);

                    for (i1 = 0; i1 < sBufSize; i1++) {
                        jDataP [i1] = sDataP [i1];
                    }

                    unlockBuffer (sDataH);
                }
            }
        }

        (*env)->ReleaseByteArrayElements (env, data, jDataP, 0);
    }

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmGetTagSize
 * Signature: (JI[I)I
 * static native int cmmGetTagSize (long profileID, int tagSignature, int[] size);
 */
JNIEXPORT jint JNICALL Java_sun_awt_color_CMM_cmmGetTagSize
  (JNIEnv *env, jobject obj, jlong profileID, jint tagSignature, jintArray size)
{
SpStatus_t    theStatus = SpStatBadCallerId;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
KpUInt32_t    TagDataSize = 0;
SpTagId_t    TagId;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmGetTagSize\n");

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theSProfile.j = profileID;
        DIAG_PRINTF3("theSProfile = %x; tag = %.4s\n", theSProfile.pf, &tagSignature);

        TagId = (SpTagId_t) (size_t)tagSignature;

        if (TagId == SpTagHeader) {
            TagDataSize = HEADER_SIZE;
            theStatus = SpStatSuccess;
        }
        else {
            theStatus = SpRawTagDataGetSize (theSProfile.pf, TagId, &TagDataSize);    /* get size of this tag */
        }
    }

    returnInt (env, size, TagDataSize);

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmSetTagData
 * Signature: (JI[B)I
 * static native int cmmSetTagData (long profileID, int tagSignature, byte[] data);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmSetTagData (JNIEnv *env, jobject obj, jlong profileID, jint tagSignature, jbyteArray data)
{
SpStatus_t    theStatus = SpStatBadCallerId;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
jbyte_p        dataP;
KpUInt32_t    TagDataSize;
SpTagId_t    TagId;
SpHeader_t    thePublicHeader;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmSetTagData\n");

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    } else if (data == NULL) {
        theStatus = SpStatBadTagData;
    } else {
        theSProfile.j = profileID;
        DIAG_PRINTF3("theSProfile = %x; tag = %.4s\n", theSProfile.pf, &tagSignature);

        TagId = (SpTagId_t) (size_t)tagSignature;

        dataP = (*env)->GetByteArrayElements (env, data, 0);            /* get the data address */
        TagDataSize = (KpUInt32_t) (*env)->GetArrayLength(env, data);    /* and size */

        if (TagId == SpTagHeader) {
            theStatus = SpHeaderToPublic ((KpInt8_p)dataP, TagDataSize, &thePublicHeader);
            if (theStatus == SpStatSuccess) {
                theStatus = SpProfileSetHeader (theSProfile.pf, &thePublicHeader);
            }
        }
        else {
            theStatus = SpRawTagDataSet (theSProfile.pf, TagId, TagDataSize, (KpInt8_p)dataP);    /* set tag data */
        }
        
        (*env)->ReleaseByteArrayElements (env, data, dataP, 0);            /* don't hold data */
    }

    return checkStatus (theStatus);
}


/*
 * Class:     sun_awt_color_CMM
 * Method:    cmmGetTransform
 * Signature: (Ljava/awt/color/ICC_Profile;IILjava/awt/color/ICC_Transform;)I
 * static native int cmmGetTransform
        (ICC_Profile profile, int renderType, int transformType, ICC_Transform result);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmGetTransform (JNIEnv *env, jobject obj, jobject profile, jint renderType, jint transformType, jobject result)
{
SpStatus_t    theStatus = SpStatSuccess;
SpCallerId_t    theCallerId;
storeID_t    theSProfile;
storeID_t    theXform;
KpInt32_t    whichRender;
jclass        cls;
jfieldID    fid;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmGetTransform\n");
    theXform.xf = NULL;

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        cls = (*env)->GetObjectClass (env, profile);    /* get the profile ID */

        fid = (*env)->GetFieldID (env, cls, "ID", "J");
        if (fid != 0) {
            theSProfile.j = (*env)->GetLongField (env, profile, fid);
        }

        switch (renderType) {
        case JAVA_ICC_TransformAny:
            whichRender = SpTransRenderAny;
            break;
        
        case icPerceptual:
            whichRender = SpTransRenderPerceptual;
            break;
        
        case icRelativeColorimetric:
            whichRender = SpTransRenderColormetric;
            break;

        case icSaturation:
            whichRender = SpTransRenderSaturation;
            break;

        case icAbsoluteColorimetric:
            whichRender = SpTransRenderAbsColormetric;
            break;
            
        default:
            theStatus = SpStatOutOfRange;
            whichRender = -1;    /* avoid warning */
        }

        if (theStatus == SpStatSuccess) {
            theStatus = SpXformGet (theSProfile.pf, whichRender, transformType, &theXform.xf);

            DIAG_PRINTF3("theSProfile = %x; xForm = %x\n", theSProfile.pf, theXform.xf);
        }
    }

    setObjectID (env, result, theXform);        /* set ID in the transform object */

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmFreeTransform
 * Signature: (J)I
 * static native int cmmFreeTransform (long transformID);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmFreeTransform (JNIEnv *env, jobject obj, jlong transformID)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theXform;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmFreeTransform\n");

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theXform.j = transformID;

        theStatus = SpXformFree (&theXform.xf);
    }

    return checkStatus (theStatus);
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmGetNumComponents
 * Signature: (J[I)I
 * static native int cmmGetNumComponents (long transformID, int[] nComps);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmGetNumComponents (JNIEnv *env, jobject obj, jlong transformID, jintArray nComps)
{
SpStatus_t    theStatus = SpStatBadCallerId;
SpCallerId_t    theCallerId;
storeID_t    theXform;
jint_p        dataP;
KpInt32_t    nIn = 0, nOut = 0;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmGetNumComponents\n");

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theXform.j = transformID;

        theStatus = SpXformGetChannels (theXform.xf, &nIn, &nOut);    /* get number of components of this transform */

        DIAG_PRINTF4("theXform = %x; nIn = %d; nOut = %d\n", theXform.xf, nIn, nOut);
    }

    dataP = (*env)->GetIntArrayElements (env, nComps, 0);            /* get the data address */

    dataP[0] = nIn;        /* return the number of input components */
    dataP[1] = nOut;    /* and the number of output components */

    (*env)->ReleaseIntArrayElements (env, nComps, dataP, 0);        /* don't hold data */

    return checkStatus (theStatus);
}



/* Class:     sun_awt_color_CMM
 * Method:    cmmCombineTransforms
 * Signature: ([JLjava/awt/color/ICC_Transform;)I
 * static native int cmmCombineTransforms (long[] transforms, ICC_Transform result);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmCombineTransforms (JNIEnv *env, jobject obj, jlongArray transforms, jobject result)
{
SpStatus_t    theStatus;
SpCallerId_t    theCallerId;
storeID_t    theXform;
SpXform_p    xformIDs;
KpInt32_t    nXforms, FailingXform, i1;
storeID_p    dataP;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmCombineTransforms\n");

    theXform.xf = NULL;

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        nXforms = (*env)->GetArrayLength (env, transforms);                        /* number of transforms */

        xformIDs = (SpXform_p) allocBufferPtr (nXforms * sizeof (SpXform_t));
        
        dataP = (storeID_p) (*env)->GetLongArrayElements (env, transforms, 0);    /* get the array address */

        DIAG_PRINTF1("xformIDs =");
        for (i1 = 0; i1 < nXforms; i1++) {
            xformIDs[i1] = dataP[i1].xf;        /* get tranform IDs */
            DIAG_PRINTF2(" %x", xformIDs[i1]);
        }

        (*env)->ReleaseLongArrayElements(env, transforms, (jlong_p) dataP, 0);    /* don't need to hold data */

        theStatus = SpConnectSequence (nXforms, xformIDs, &theXform.xf, &FailingXform, NULL, NULL);

        DIAG_PRINTF2("\ntheXform = %x\n", theXform.xf);

        freeBufferPtr ((KpGenericPtr_t) xformIDs);
    }

    setObjectID (env, result, theXform);            /* set ID in the transform object */

    return checkStatus (theStatus);
}


static int recordArray(KpInt32_t layoutType, jarray array,
                       jarraymap_t *arrayMap) {
    int i1, count;
    count = arrayMap->count;
    for (i1 = 0; i1 < count; i1++) {
        if (arrayMap->info[i1].array == array) {
            return i1;
        }
    }
    switch (layoutType) {
        case JAVA_TYPE_INT_RGB:
        case JAVA_TYPE_INT_ARGB:
        case JAVA_TYPE_INT_BGR:
        case typeIntRGBPacked:
            arrayMap->info[count].type = 2;
            break;
        case typeCompUShort:
            arrayMap->info[count].type = 1;
            break;
        case typeCompUByte:
        case JAVA_TYPE_3BYTE_BGR:
        case JAVA_TYPE_4BYTE_ABGR:
            arrayMap->info[count].type = 0;
            break;
        default:
            arrayMap->info[count].type = -1;
            break;
    }
    arrayMap->info[count].array = array;
    arrayMap->info[count].addr = NULL;
    arrayMap->count = count + 1;
    return count;
}


/* Class:     sun_awt_color_CMM
 * Method:    cmmColorConvert
 * Signature: (JLjava/awt/color/imageLayout;Ljava/awt/color/imageLayout;)I
 * static native int cmmColorConvert (long transformID, imageLayout src, imageLayout dest);
 */
JNIEXPORT jint JNICALL
    Java_sun_awt_color_CMM_cmmColorConvert (JNIEnv *env, jobject obj, jlong transformID, jobject src, jobject dest)
{
SpCallerId_t    theCallerId;
storeID_t    theXform;
SpStatus_t    theStatus;
SpPixelLayout_t    SrcLayout;
SpPixelLayout_t    DestLayout;
jarraymap_t        arrayMap;

    if (obj) {}        /* avoid compiler warnings */

    DIAG_PRINTF1("Java_sun_awt_color_CMM_cmmColorConvert\n");

    theXform.xf = NULL;

    theCallerId = getCallerID (env);
    if (theCallerId == NULL) {
        theStatus = SpStatBadCallerId;
    }
    else {
        theXform.j = transformID;  /* get the transform ID */
        DIAG_PRINTF2("theXform = %x\n", theXform.xf);

        arrayMap.count = 0;

        theStatus = initImageLayouts(env, src, &SrcLayout,
                                     dest, &DestLayout, &arrayMap);
        
        if (theStatus == SpStatSuccess) {
            theStatus = SpEvaluate(theXform.xf, &SrcLayout, &DestLayout,
                                   NULL, NULL);
        }

        releaseArrayData(env, &arrayMap);
    }

    return checkStatus (theStatus);
}


static int needLayoutFidInit = 1;
static jfieldID typeFid;
static jfieldID numColsFid;
static jfieldID numRowsFid;
static jfieldID offsetColumnFid;
static jfieldID offsetRowFid;
static jfieldID numChannelsFid;
static jfieldID chanDataFid;
static jfieldID dataOffsetsFid;
static jfieldID sampleInfoFid;

static SpStatus_t initLayoutFids(JNIEnv *env, jobject src) {
jclass       cls;

    cls = (*env)->GetObjectClass(env, src);

    typeFid = (*env)->GetFieldID(env, cls, "Type", "I");
    if (typeFid == 0) {
        return SpStatFailure;
    }

    numColsFid = (*env)->GetFieldID(env, cls, "NumCols", "I");
    if (numColsFid == 0) {
        return SpStatFailure;
    }

    numRowsFid = (*env)->GetFieldID(env, cls, "NumRows", "I");
    if (numRowsFid == 0) {
        return SpStatFailure;
    }

    offsetColumnFid = (*env)->GetFieldID(env, cls, "OffsetColumn", "I");
    if (offsetColumnFid == 0) {
        return SpStatFailure;
    }

    offsetRowFid = (*env)->GetFieldID(env, cls, "OffsetRow", "I");
    if (offsetRowFid == 0) {
        return SpStatFailure;
    }

    numChannelsFid = (*env)->GetFieldID(env, cls, "NumChannels", "I");
    if (numChannelsFid == 0) {
        return SpStatFailure;
    }

    chanDataFid =
        (*env)->GetFieldID(env, cls, "chanData", "[Ljava/lang/Object;");
    if (chanDataFid == 0) {
        return SpStatFailure;
    }

    dataOffsetsFid = (*env)->GetFieldID(env, cls, "DataOffsets", "[I");
    if (dataOffsetsFid == 0) {
        return SpStatFailure;
    }

    sampleInfoFid = (*env)->GetFieldID(env, cls, "sampleInfo", "[I");
    if (sampleInfoFid == 0) {
        return SpStatFailure;
    }

    needLayoutFidInit = 0;
    return SpStatSuccess;
}


static SpStatus_t finishLayoutInit(SpPixelLayout_t *theLayout,
                                   KpInt32_t theType, int* theOffsets,
                                   int* theOptInfo, int theNumArrays,
                                   int* theArrayIndices, void** theAlphaPtrP,
                                   jarraymap_t *arrayMap) {
KpInt32_t    i1;
KpUInt8_p    dataP;
SpStatus_t   theStatus = SpStatSuccess;

    switch (theType) {
        case JAVA_TYPE_INT_RGB:
        case JAVA_TYPE_INT_ARGB:
        case JAVA_TYPE_INT_BGR:
            theLayout->SampleType = SpSampleType_UByte;

            for (i1 = 0; i1 < 3; i1++) {
#if defined(KPLSBFIRST)
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1] + (3 - theOptInfo[i1]);
#else
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1] + theOptInfo[i1];
#endif
                theLayout->BaseAddrs[i1] = (SpHugeBuffer_t) dataP;
            }

            if (theType == JAVA_TYPE_INT_ARGB) {
#if defined(KPLSBFIRST)
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[3]].addr) +
                    theOffsets[3] + (3 - theOptInfo[3]);
#else
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[3]].addr) +
                    theOffsets[3] + theOptInfo[3];
#endif
            }

            break;

        case JAVA_TYPE_3BYTE_BGR:
        case JAVA_TYPE_4BYTE_ABGR:
            theLayout->SampleType = SpSampleType_UByte;

            for (i1 = 0; i1 < 3; i1++) {
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1];
                theLayout->BaseAddrs[i1] = (SpHugeBuffer_t) dataP;
            }

            if (theType == JAVA_TYPE_4BYTE_ABGR) {
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[3]].addr) +
                    theOffsets[3];
            }

            break;

        case typeCompUByte:
            theLayout->SampleType = SpSampleType_UByte;

            for (i1 = 0; i1 < theLayout->NumChannels; i1++) {
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1];
                theLayout->BaseAddrs[i1] = (SpHugeBuffer_t) dataP;
            }

            if (theNumArrays > theLayout->NumChannels) {
                /* there is an alpha channel */
                i1 = theLayout->NumChannels;
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr) +
                    theOffsets[i1];
            }

            break;

        case typeCompUShort:
            theLayout->SampleType = SpSampleType_UShort;

            for (i1 = 0; i1 < theLayout->NumChannels; i1++) {
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1];
                theLayout->BaseAddrs[i1] = (SpHugeBuffer_t) dataP;
            }

            if (theNumArrays > theLayout->NumChannels) {
                /* there is an alpha channel */
                i1 = theLayout->NumChannels;
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr) +
                    theOffsets[i1];
            }

            break;

        case typeIntRGBPacked:
            theLayout->SampleType = SpSampleType_UByte;

            for (i1 = 0; i1 < 3; i1++) {
#if defined(KPLSBFIRST)
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1] + (3 - theOptInfo[i1]);
#else
                dataP = ((KpUInt8_p) arrayMap->info[theArrayIndices[i1]].addr)
                        + theOffsets[i1] + theOptInfo[i1];
#endif
                theLayout->BaseAddrs[i1] = (SpHugeBuffer_t) dataP;
            }

            if (theNumArrays > theLayout->NumChannels) {
#if defined(KPLSBFIRST)
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[3]].addr) +
                    theOffsets[3] + (3 - theOptInfo[3]);
#else
                *theAlphaPtrP =
                    ((KpUInt8_p) arrayMap->info[theArrayIndices[3]].addr) +
                    theOffsets[3] + theOptInfo[3];
#endif
            }

            break;

        default:
            theStatus = SpStatFailure;
            theLayout->SampleType = 0;
    }

    return theStatus;
}



void handleAlpha(SpPixelLayout_t *srcLayout, void *srcAlphaPtr,
                 SpPixelLayout_t *dstLayout, void *dstAlphaPtr) {
    int x, y;
    int sxinc, syinc, dxinc, dyinc;
    if (srcAlphaPtr == NULL) {
        /* no src alpha, fill dst alpha with alpha = 1.0 */
        if (dstLayout->SampleType == SpSampleType_UByte) {
            /* dst is UByte */
            KpUInt8_p dptr, dptr1;
            dptr1 = (KpUInt8_p) dstAlphaPtr;
            dxinc = dstLayout->OffsetColumn;
            dyinc = dstLayout->OffsetRow;
            for (y = dstLayout->NumRows; y > 0; y--) {
                dptr = dptr1;
                for (x = dstLayout->NumCols; x > 0; x--) {
                    *dptr = 0xff;
                    dptr += dxinc;
                }
                dptr1 += dyinc;
            }
        } else {
            /* dst is UShort */
            KpUInt16_p dptr, dptr1;
            dptr1 = (KpUInt16_p) dstAlphaPtr;
            dxinc = dstLayout->OffsetColumn / 2;
            dyinc = dstLayout->OffsetRow / 2;
            for (y = dstLayout->NumRows; y > 0; y--) {
                dptr = dptr1;
                for (x = dstLayout->NumCols; x > 0; x--) {
                    *dptr = 0xffff;
                    dptr += dxinc;
                }
                dptr1 += dyinc;
            }
        }
    } else if (srcLayout->SampleType == SpSampleType_UByte) {
        if (dstLayout->SampleType == SpSampleType_UByte) {
            /* src is UByte, dst is UByte */
            KpUInt8_p sptr, sptr1;
            KpUInt8_p dptr, dptr1;
            if (srcAlphaPtr == dstAlphaPtr) {
                /* same array, so no need to copy */
                return;
            }
            sptr1 = (KpUInt8_p) srcAlphaPtr;
            dptr1 = (KpUInt8_p) dstAlphaPtr;
            sxinc = srcLayout->OffsetColumn;
            syinc = srcLayout->OffsetRow;
            dxinc = dstLayout->OffsetColumn;
            dyinc = dstLayout->OffsetRow;
            for (y = dstLayout->NumRows; y > 0; y--) {
                sptr = sptr1;
                dptr = dptr1;
                for (x = dstLayout->NumCols; x > 0; x--) {
                    *dptr = *sptr;
                    sptr += sxinc;
                    dptr += dxinc;
                }
                sptr1 += syinc;
                dptr1 += dyinc;
            }
        } else {
            /* src is UByte, dst is UShort */
            KpUInt8_p sptr, sptr1;
            KpUInt16_p dptr, dptr1;
            unsigned int datum;
            sptr1 = (KpUInt8_p) srcAlphaPtr;
            dptr1 = (KpUInt16_p) dstAlphaPtr;
            sxinc = srcLayout->OffsetColumn;
            syinc = srcLayout->OffsetRow;
            dxinc = dstLayout->OffsetColumn / 2;
            dyinc = dstLayout->OffsetRow / 2;
            for (y = dstLayout->NumRows; y > 0; y--) {
                sptr = sptr1;
                dptr = dptr1;
                for (x = dstLayout->NumCols; x > 0; x--) {
                    datum = *sptr;
                    /* multiply 8-bit alpha by 257 to get 16-bit alpha */
                    *dptr = (datum << 8) | datum;
                    sptr += sxinc;
                    dptr += dxinc;
                }
                sptr1 += syinc;
                dptr1 += dyinc;
            }
        }
    } else {
        if (dstLayout->SampleType == SpSampleType_UByte) {
            /* src is UShort, dst is UByte */
            KpUInt16_p sptr, sptr1;
            KpUInt8_p dptr, dptr1;
            unsigned int datum;
            sptr1 = (KpUInt16_p) srcAlphaPtr;
            dptr1 = (KpUInt8_p) dstAlphaPtr;
            sxinc = srcLayout->OffsetColumn / 2;
            syinc = srcLayout->OffsetRow / 2;
            dxinc = dstLayout->OffsetColumn;
            dyinc = dstLayout->OffsetRow;
            for (y = dstLayout->NumRows; y > 0; y--) {
                sptr = sptr1;
                dptr = dptr1;
                for (x = dstLayout->NumCols; x > 0; x--) {
                    datum = *sptr;
                    /* divide 16-bit alpha by 257 to get 8-bit alpha */
                    /* N/257 = N/256 * 256/257 */
                    /* 256/257 = 0x0.FF00FF00... */
                    /* so, N/257 = (N * 0xFF00.FF) / (2^24), approximately */
                    /* we round the result */
                    /* first, multiply by 0xFF */
                    datum = (datum << 8) - datum;
                    /* now, multiply by 0x100.01, add the rounding term, */
                    /* and divide by 2^24 */
                    *dptr = ((datum << 8) + (datum >> 8) + (1 << 23)) >> 24;
                    sptr += sxinc;
                    dptr += dxinc;
                }
                sptr1 += syinc;
                dptr1 += dyinc;
            }
        } else {
            /* src is UShort, dst is UShort */
            KpUInt16_p sptr, sptr1;
            KpUInt16_p dptr, dptr1;
            if (srcAlphaPtr == dstAlphaPtr) {
                /* same array, so no need to copy */
                return;
            }
            sptr1 = (KpUInt16_p) srcAlphaPtr;
            dptr1 = (KpUInt16_p) dstAlphaPtr;
            sxinc = srcLayout->OffsetColumn / 2;
            syinc = srcLayout->OffsetRow / 2;
            dxinc = dstLayout->OffsetColumn / 2;
            dyinc = dstLayout->OffsetRow / 2;
            for (y = dstLayout->NumRows; y > 0; y--) {
                sptr = sptr1;
                dptr = dptr1;
                for (x = dstLayout->NumCols; x > 0; x--) {
                    *dptr = *sptr;
                    sptr += sxinc;
                    dptr += dxinc;
                }
                sptr1 += syinc;
                dptr1 += dyinc;
            }
        }
    }
}



/* get image layouts */
static SpStatus_t initImageLayouts(JNIEnv *env,
                                   jobject src, SpPixelLayout_t *srcLayout,
                                   jobject dst, SpPixelLayout_t *dstLayout,
                                   jarraymap_t *arrayMap) {
KpInt32_t    i1;
KpInt32_t    srcType = -1;
KpInt32_t    dstType = -1;
jobjectArray srcChanData = NULL;
jobjectArray dstChanData = NULL;
jintArray    srcDataOffsets = NULL;
int          srcOffsets[SpMaxComponents + 1];
jintArray    dstDataOffsets = NULL;
int          dstOffsets[SpMaxComponents + 1];
jintArray    srcSampleInfo = NULL;
int          srcSampleInfoData[SpMaxComponents + 1];
int*         srcOptInfo = NULL;
jintArray    dstSampleInfo = NULL;
int          dstSampleInfoData[SpMaxComponents + 1];
int*         dstOptInfo = NULL;
int          srcArrayIndices[SpMaxComponents + 1];
int          dstArrayIndices[SpMaxComponents + 1];
int          srcNumArrays;
int          dstNumArrays;
void*        srcAlphaPtr = NULL;
void*        dstAlphaPtr = NULL;
SpStatus_t   theStatus = SpStatSuccess;


    if (needLayoutFidInit) {
        SpStatus_t initStatus = initLayoutFids(env, src);
        if (initStatus != SpStatSuccess) {
            return SpStatFailure;
        }
    }

    srcType = (KpInt32_t) (*env)->GetIntField(env, src, typeFid);
    srcLayout->NumCols = (KpInt32_t) (*env)->GetIntField(env, src, numColsFid);
    srcLayout->NumRows = (KpInt32_t) (*env)->GetIntField(env, src, numRowsFid);
    srcLayout->OffsetColumn = (KpInt32_t) (*env)->GetIntField(env, src,
                                                              offsetColumnFid);
    srcLayout->OffsetRow = (KpInt32_t) (*env)->GetIntField(env, src, 
                                                           offsetRowFid);
    srcLayout->NumChannels = (KpInt32_t) (*env)->GetIntField(env, src,
                                                             numChannelsFid);
    srcChanData = (jobject) (*env)->GetObjectField(env, src, chanDataFid);
    srcNumArrays = (int) (*env)->GetArrayLength(env, (jarray) srcChanData);
    srcDataOffsets =
        (jintArray) (*env)->GetObjectField(env, src, dataOffsetsFid);
    (*env)->GetIntArrayRegion(env, srcDataOffsets, 0, srcNumArrays,
                              srcOffsets);
    srcSampleInfo =
        (jintArray) (*env)->GetObjectField(env, src, sampleInfoFid);
    if (srcSampleInfo != NULL) {
        (*env)->GetIntArrayRegion(env, srcSampleInfo, 0, srcNumArrays,
                                  srcSampleInfoData);
        srcOptInfo = srcSampleInfoData;
    }

    dstType = (KpInt32_t) (*env)->GetIntField(env, dst, typeFid);
    dstLayout->NumCols = (KpInt32_t) (*env)->GetIntField(env, dst, numColsFid);
    dstLayout->NumRows = (KpInt32_t) (*env)->GetIntField(env, dst, numRowsFid);
    dstLayout->OffsetColumn = (KpInt32_t) (*env)->GetIntField(env, dst,
                                                              offsetColumnFid);
    dstLayout->OffsetRow = (KpInt32_t) (*env)->GetIntField(env, dst, 
                                                           offsetRowFid);
    dstLayout->NumChannels = (KpInt32_t) (*env)->GetIntField(env, dst,
                                                             numChannelsFid);
    dstChanData = (jobject) (*env)->GetObjectField(env, dst, chanDataFid);
    dstNumArrays = (int) (*env)->GetArrayLength(env, (jarray) dstChanData);
    dstDataOffsets =
        (jintArray) (*env)->GetObjectField(env, dst, dataOffsetsFid);
    (*env)->GetIntArrayRegion(env, dstDataOffsets, 0, dstNumArrays,
                              dstOffsets);
    dstSampleInfo =
        (jintArray) (*env)->GetObjectField(env, dst, sampleInfoFid);
    if (dstSampleInfo != NULL) {
        (*env)->GetIntArrayRegion(env, dstSampleInfo, 0, dstNumArrays,
                                  dstSampleInfoData);
        dstOptInfo = dstSampleInfoData;
    }

    for (i1 = 0; i1 < srcNumArrays; i1++) {
        jarray array = (jarray) (*env)->GetObjectArrayElement(env,
            srcChanData, i1);
        srcArrayIndices[i1] = recordArray(srcType, array, arrayMap);
    }

    for (i1 = 0; i1 < dstNumArrays; i1++) {
        jarray array = (jarray) (*env)->GetObjectArrayElement(env,
            dstChanData, i1);
        dstArrayIndices[i1] = recordArray(dstType, array, arrayMap);
    }

    for (i1 = 0; i1 < arrayMap->count; i1++) {
        void* addr;
        addr = (*env)->GetPrimitiveArrayCritical(env, arrayMap->info[i1].array,
                                                 0);
        arrayMap->info[i1].addr = addr;
        if (addr == NULL) {
            return SpStatFailure;
        }
    }

    theStatus = finishLayoutInit(srcLayout, srcType, srcOffsets,
                                 srcOptInfo, srcNumArrays, srcArrayIndices,
                                 &srcAlphaPtr, arrayMap);
    if (theStatus != SpStatFailure) {
        theStatus = finishLayoutInit(dstLayout, dstType, dstOffsets,
                                     dstOptInfo, dstNumArrays, dstArrayIndices,
                                     &dstAlphaPtr, arrayMap);
    }
    if ((theStatus != SpStatFailure) &&
        (dstNumArrays != dstLayout->NumChannels)) {
        /* need to set dst alpha samples, since there is a dst alpha channel */
        handleAlpha(srcLayout, srcAlphaPtr, dstLayout, dstAlphaPtr);
    }

    return theStatus;
}


/* release data used for image processing */
static void releaseArrayData(JNIEnv *env, jarraymap_t *arrayMap) {

KpInt32_t    i1;

    for (i1 = arrayMap->count - 1; i1 >= 0; i1--) {
        void* addr;
        addr = arrayMap->info[i1].addr;
        if (addr != NULL) {
            (*env)->ReleasePrimitiveArrayCritical(env,
                arrayMap->info[i1].array, addr, 0);
        }
    }

}


/* get the caller ID */

static SpCallerId_t getCallerID (JNIEnv *env)
{
jclass    cls;

    cls = (*env)->FindClass (env, "sun/awt/color/CMM");

    return getClassID (env, cls);
}


static SpCallerId_t getClassID (JNIEnv *env, jclass theClass)
{
jfieldID    fid;
storeID_t    theCallerId;

    fid = (*env)->GetStaticFieldID (env, theClass, "ID", "J");
    if (fid != 0) {
        theCallerId.j = (*env)->GetStaticLongField (env, theClass, fid);
    }
    else {
        DIAG_PRINTF1("SpStatBadCallerId\n");
        theCallerId.cid = NULL;
    }

    return theCallerId.cid;
}



/* set the ID of a class */
static SpStatus_t setClassID (JNIEnv *env, jclass theClass, SpCallerId_t theID)
{
jfieldID    fid;
SpStatus_t    theStatus;
storeID_t    theJId;

    theJId.cid = theID;
    
    fid = (*env)->GetStaticFieldID (env, theClass, "ID", "J");
    if (fid == 0) {
        theStatus = SpStatFailure;
    }
    else {
        theJId.cid = theID;
    
        (*env)->SetStaticLongField (env, theClass, fid, theJId.j);
        theStatus = SpStatSuccess;
    }
    
    return theStatus;
}



/* set the ID of an  object */
static void setObjectID (JNIEnv *env, jobject theObject, storeID_t theID)
{
jclass        cls;
jfieldID    fid;

    cls = (*env)->GetObjectClass (env, theObject);

    fid = (*env)->GetFieldID (env, cls, "ID", "J");
    if (fid != 0) {
        (*env)->SetLongField (env, theObject, fid, theID.j);
    }
}


/* store an int into the first element of an int array */
static void returnInt (JNIEnv *env, jintArray theReturn, jint theInt)
{ 
jint_p        dataP;

    dataP = (*env)->GetIntArrayElements (env, theReturn, 0);

    dataP[0] = theInt;                    /* return the number of matches in the first element */

    (*env)->ReleaseIntArrayElements (env, theReturn, dataP, 0);
}


/* check for success, print error if not */
static jint checkStatus (SpStatus_t theStatus)
{
    if (theStatus != SpStatSuccess) {
        DIAG_PRINTF2("theStatus = %d\n", theStatus);
    }

    return (jint) theStatus;
}

