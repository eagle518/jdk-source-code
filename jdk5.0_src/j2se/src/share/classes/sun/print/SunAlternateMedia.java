/*
 * @(#)SunAlternateMedia.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import javax.print.attribute.PrintRequestAttribute;
import javax.print.attribute.standard.Media;

/*
 * An implementation class used by services which can distinguish media
 * by size and media by source. Values are expected to be MediaTray
 * instances, but this is not enforced by API. 
 */
public class SunAlternateMedia implements PrintRequestAttribute {

    private static final long serialVersionUID = -8878868345472850201L;

    private Media media;

    public SunAlternateMedia(Media altMedia) {
        media = altMedia;
    }

    public Media getMedia() {
        return media;
    }

    public final Class getCategory() {
        return SunAlternateMedia.class;
    }

    public final String getName() {
        return "sun-alternate-media";
    }
 
    public String toString() {
       return "alternatate-media: " + media.toString();
    }

    /**
     * Returns a hash code value for this enumeration value. The hash code is
     * just this enumeration value's integer value.
     */
    public int hashCode() {
        return media.hashCode();
    }
}
