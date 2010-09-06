/*
 * @(#)ProfileDeferralInfo.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.color;

import java.io.InputStream;
import java.io.IOException;


/**
 * A class to pass information about a profile to be loaded from
 * a file to the static getInstance(InputStream) method of
 * ICC_Profile.  Loading of the profile data and initialization
 * of the CMM is to be deferred as long as possible.
 */
public class ProfileDeferralInfo extends InputStream {

    public int colorSpaceType, numComponents, profileClass;
    public String filename;

    public ProfileDeferralInfo(String fn, int type, int ncomp, int pclass) {

        super();
        filename = fn;
        colorSpaceType = type;
        numComponents = ncomp;
        profileClass = pclass;
    }


    /**
     * Implements the abstract read() method of InputStream.
     */
    public int read() throws IOException {

        return 0;
    }

}
