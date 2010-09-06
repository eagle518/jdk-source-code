/*
 * @(#)CMM.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
    Contains:    Java Color Management Module

            Created by gbp, October 1, 1997

    Copyright:    (C) 1997 by Eastman Kodak Company, all rights reserved.

    @(#)CMM.java	1.2    11/06/97
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



/* This is the interface to the native Color Managment Module.
 */


package sun.awt.color;

import java.awt.color.ColorSpace;
import java.awt.color.ICC_Profile;
import java.awt.color.CMMException;

public class CMM {
    private static long ID = 0;

    public static ColorSpace GRAYspace;       // These two fields allow access
    public static ColorSpace LINEAR_RGBspace; // to java.awt.color.ColorSpace
                                              // private fields from other
                                              // packages.  The fields are set
                                              // by java.awt.color.ColorSpace
                                              // and read by
                                              // java.awt.image.ColorModel.

    static final int cmmStatSuccess            = 0;
    static final int cmmStatBadProfile        = 503;
    static final int cmmStatBadTagData        = 504;
    static final int cmmStatBadTagType        = 505;
    static final int cmmStatBadTagId        = 506;
    static final int cmmStatBadXform        = 507;
    static final int cmmStatXformNotActive    = 508;
    static final int cmmStatOutOfRange        = 518;
    static final int cmmStatTagNotFound        = 519;

    /* methods invoked from ICC_Profile */
    public static native int cmmLoadProfile(byte[] data, long[] profileID);
    public static native int cmmFreeProfile(long profileID);
    public static native int cmmGetProfileSize(long profileID, int[] size);
    public static native int cmmGetProfileData(long profileID, byte[] data);
    public static native int cmmGetTagSize(long profileID, int tagSignature,
                                           int[] size);
    public static native int cmmGetTagData(long profileID, int tagSignature,
                                           byte[] data);
    public static native int cmmSetTagData(long profileID, int tagSignature,
                                           byte[] data);

    /* methods invoked from ICC_Transform */
    public static native int cmmGetTransform(ICC_Profile profile,
                                             int renderType,
                                             int transformType,
                                             ICC_Transform result);
    public static native int cmmCombineTransforms(long[] transforms,
                                                  ICC_Transform result);
    public static native int cmmFreeTransform(long transformID);
    public static native int cmmGetNumComponents(long transformID,
                                                 int[] nComps);
    public static native int cmmColorConvert(long transformID,
                                             CMMImageLayout src,
                                             CMMImageLayout dest);

    /* methods invoked from ICC_Search */
    public static native int cmmFindICC_Profiles(byte[] template,
                                                 byte[] options,
                                                 String profileDir,
                                                 long[] matchIDs,
                                                 int[] nMatch);
    public static native int cmmCullICC_Profiles(byte[] template,
                                                 byte[] options,
                                                 long[] sourceIDs,
                                                 long[] matchIDs,
                                                 int[] nMatch);

    /* methods invoked from this class */
    static native int cmmInit();
    static native int cmmTerminate ();


    /* the class initializer which loads the CMM */
     static {
	int    cmmStatus;

	java.security.AccessController.doPrivileged(
		  new sun.security.action.LoadLibraryAction("cmm"));
        cmmStatus = cmmInit ();
    }

    /**
     * Frees the resources associated with the native CMM.
     */
    protected void finalize () {
    int            cmmStatus;

        checkStatus (cmmTerminate ());
    }


    public static void checkStatus (    int    cmmStatus)
    {
        if (cmmStatus != cmmStatSuccess) {
            throw new CMMException (errorString(cmmStatus));
        }
    }


    static String errorString (int cmmError)
    {
        switch (cmmError) {
        case cmmStatSuccess:
            return ("Success");

        case cmmStatTagNotFound:
            return ("No such tag");

        case cmmStatBadProfile:
            return ("Invalid profile data");

        case cmmStatBadTagData:
            return ("Invalid tag data");

        case cmmStatBadTagType:
            return ("Invalid tag type");

        case cmmStatBadTagId:
            return ("Invalid tag signature");

        case cmmStatBadXform:
            return ("Invlaid transform");

        case cmmStatXformNotActive:
            return ("Transform is not active");

        case cmmStatOutOfRange:
            return ("Invalid image format");
        }

        return ("General CMM error" + cmmError);
    }
}
