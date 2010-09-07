/*
 * @(#)XMap.java	1.4 12/19/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.FontFormatException;
import java.awt.font.FontRenderContext;
import java.awt.geom.GeneralPath;
import java.awt.geom.Rectangle2D;
import java.util.HashMap;
import java.util.Locale;
import sun.io.CharToByteConverter;
import sun.io.MalformedInputException;

class XMap {

    private static HashMap xMappers = new HashMap();

    /* ConvertedGlyphs has unicode code points as indexes and values
     * are platform-encoded multi-bytes chars packed into java chars.
     * These platform-encoded characters are equated to glyph ids, although
     * that's not strictly true, as X11 only supports using chars.
     * The assumption carried over from the native implementation that
     * a char is big enough to hold an X11 glyph id (ie platform char).
     */
    char[] convertedGlyphs;

    static synchronized XMap getXMapper(String encoding) {
	XMap mapper = (XMap)xMappers.get(encoding);
	if (mapper == null) {
	    mapper = getXMapperInternal(encoding);
	    xMappers.put(encoding, mapper);
	}
	return mapper;
    }

    static final int SINGLE_BYTE = 1;
    static final int DOUBLE_BYTE = 2;

    private static XMap getXMapperInternal(String encoding) {

	String jclass = null;
	int nBytes = SINGLE_BYTE;
	int maxU = 0xffff;
	int minU = 0;
	boolean addAscii = false;
	boolean lowPartOnly = false;
	if (encoding.equals("dingbats")) {
	    jclass = "awt.motif.CharToByteX11Dingbats";
	    minU = 0x2701;
	    maxU = 0x27be;
	} else if (encoding.equals("symbol")){
	    jclass = "awt.CharToByteSymbol";
	    minU = 0x0391;
	    maxU = 0x22ef;
	} else if (encoding.equals("iso8859-1")) {
	    maxU = 0xff;
	} else if (encoding.equals("iso8859-2")) {
	    jclass = "io.CharToByteISO8859_2";
	} else if (encoding.equals("jisx0208.1983-0")) {
	    jclass = "awt.motif.CharToByteX11JIS0208";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals("jisx0201.1976-0")) {
	    jclass = "awt.motif.CharToByteX11JIS0201";
	    // this is mapping the latin supplement range 128->255 which
	    // doesn't exist in JIS0201. This needs examination.
	    // it was also overwriting a couple of the mappings of
	    // 7E and A5 which in JIS201 are different chars than in
	    // Latin 1. I have revised AddAscii to not overwrite chars
	    // which are already converted.
	    addAscii = true;
	    lowPartOnly = true;
	} else if (encoding.equals("jisx0212.1990-0")) {
	    jclass = "awt.motif.CharToByteX11JIS0212";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals("iso8859-4")) {
	    jclass = "io.CharToByteISO8859_4";
	} else if (encoding.equals("iso8859-5")) {
	    jclass = "io.CharToByteISO8859_5";
	} else if (encoding.equals("koi8-r")) {
	    jclass = "io.CharToByteKOI8_R";
	} else if (encoding.equals("ansi-1251")) {
	    jclass = "io.CharToByteCp1251";
	} else if (encoding.equals("iso8859-6")) {
	    jclass = "io.CharToByteISO8859_6";
	} else if (encoding.equals("iso8859-7")) {
	    jclass = "io.CharToByteISO8859_7";
	} else if (encoding.equals("iso8859-8")) {
	    jclass = "io.CharToByteISO8859_8";
	} else if (encoding.equals("iso8859-9")) {
	    jclass = "io.CharToByteISO8859_9";
	} else if (encoding.equals("iso8859-13")) {
	    jclass = "io.CharToByteISO8859_13";
	} else if (encoding.equals("iso8859-15")) {
	    jclass = "io.CharToByteISO8859_15";
	} else if (encoding.equals("ksc5601.1987-0")) {
	    jclass ="awt.motif.CharToByteX11KSC5601";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals( "ksc5601.1992-3")) {
	    jclass ="awt.motif.CharToByteX11Johab";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals( "ksc5601.1987-1")) {
	    jclass ="io.CharToByteEUC_KR";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals( "cns11643-1")) {
	    jclass = "awt.motif.CharToByteX11CNS11643P1";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals("cns11643-2")) {
	    jclass = "awt.motif.CharToByteX11CNS11643P2";
	    nBytes = DOUBLE_BYTE;
	} else if (encoding.equals("cns11643-3")) {
            jclass = "awt.motif.CharToByteX11CNS11643P3";
            nBytes = DOUBLE_BYTE;
	} else if (encoding.equals("gb2312.1980-0")) {
            jclass = "awt.motif.CharToByteX11GB2312";
            nBytes = DOUBLE_BYTE;
	} else if (encoding.indexOf("big5") >= 0) {
            jclass = "io.CharToByteBig5";
            nBytes = DOUBLE_BYTE;
            addAscii = true;
	} else if (encoding.equals("tis620.2533-0")) {
            jclass = "io.CharToByteTIS620";
	} else if (encoding.equals("gbk-0")) {
            jclass = "awt.motif.CharToByteX11GBK";
            nBytes = DOUBLE_BYTE;
	} else if (encoding.indexOf("sun.unicode-0") >= 0) {
            jclass = "awt.motif.CharToByteX11SunUnicode_0";
            nBytes = DOUBLE_BYTE;
	} else if (encoding.indexOf("gb18030.2000-1") >= 0) {
            jclass = "awt.motif.CharToByteX11GB18030_1";
            nBytes = DOUBLE_BYTE;
	} else if (encoding.indexOf( "gb18030.2000-0") >= 0) {
            jclass = "awt.motif.CharToByteX11GB18030_0";
            nBytes = DOUBLE_BYTE;
	} else if (encoding.indexOf("hkscs") >= 0) {
            jclass = "io.CharToByteHKSCS";
            nBytes = DOUBLE_BYTE;
	}
	if (jclass != null) {
	    jclass = "sun."+jclass;
	}

	return new XMap(jclass, minU, maxU, nBytes, addAscii, lowPartOnly);
    }

    private XMap(String className, int minU, int maxU, int nBytes,
		 boolean addAscii, boolean lowPartOnly) {

	Class converterClass = null;
	CharToByteConverter charConverter = null;

	if (className != null) {
	    try {
		converterClass = Class.forName(className);
	    } catch (ClassNotFoundException e) {
	    }
	}

	if (converterClass == null) {
	    convertedGlyphs = new char[256];
	    for (int i=0; i<256; i++) {
		convertedGlyphs[i] = (char)i;
	    }
	    return;
	} else {
	    try {
		charConverter =
		    (CharToByteConverter)converterClass.newInstance();
		charConverter.setSubstitutionBytes(new byte[nBytes]);
	    } catch (Exception e) {
		convertedGlyphs = new char[256];
		for (char i=0; i<256; i++) {
		    convertedGlyphs[i] = i;
		}
		return;
	    }

	    /* chars is set to the unicode values to convert,
	     * bytes is where the X11 character codes will be output.
	     * Finally we pack the byte pairs into chars.
	     */

	    int count = maxU - minU + 1;
	    byte[] bytes = new byte[count*nBytes];
	    char[] chars  = new char[count];
	    for (int i=0; i<count; i++) {
		chars[i] = (char)(minU+i);
	    }
	    int startCharIndex = 0;
	    /* For multi-byte encodings, single byte chars should be skipped */
	    if (nBytes > SINGLE_BYTE && minU < 256) {
		startCharIndex = 256-minU;
	    }
	    boolean done = false;

	    while (!done) {
		try {
		    int ret=charConverter.convert(chars, startCharIndex, count,
					  bytes, startCharIndex*nBytes,
					  count*nBytes);
		    done = true;
		} catch (MalformedInputException e) {
		    startCharIndex = charConverter.nextCharIndex()+1;
		} catch (Exception e) {
		    done = true;
		}
	    }
	    convertedGlyphs = new char[65536];
	    for (int i=0; i<count; i++) {
		if (nBytes == 1) {
		    convertedGlyphs[i+minU] = (char)(bytes[i]&0xff);
		} else {
		    convertedGlyphs[i+minU] =
			(char)(((bytes[i*2]&0xff) << 8) + (bytes[i*2+1]&0xff));
		}
	    }
	}
	
	int max = (lowPartOnly) ? 128 : 256;
	if (addAscii && convertedGlyphs.length >= 256) {
	    for (int i=0;i<max;i++) {
		if (convertedGlyphs[i] == 0) {
		    convertedGlyphs[i] = (char)i;
		}
	    }
	}
    }
}
