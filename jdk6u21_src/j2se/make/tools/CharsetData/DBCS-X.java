/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// -- This file was mechanically generated: Do not edit! -- //

package $PACKAGE$;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.util.Arrays;
import sun.nio.cs.StandardCharsets;
import sun.nio.cs.HistoricallyNamedCharset;
import static sun.nio.cs.CharsetMapping.*;
import sun.nio.cs.ext.$DBCS_BASE$;

public class $NAME_CLZ$ extends Charset
                        $IMPLEMENTS$
{
    public $NAME_CLZ$() {
        super("$NAME_CS$", $NAME_ALIASES$);
    }

    $HISTORICALNAME$

    public boolean contains(Charset cs) {
        $CONTAINS$;
    }

    public CharsetDecoder newDecoder() {
        return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
        return new Encoder(this);
    }

    static class Decoder extends $DBCS_BASE$.Decoder {
        $B2CTABLE$

        Decoder(Charset cs) {
            super(cs);
        }

        char decodeSingle(int b) {
            return SB.charAt(b);
        }

        char decodeDouble(int b1, int b2) {
            int seg = (b1 >> 4);
            if (DB[seg] != null && b2 >= $B2MIN$ && b2 <= $B2MAX$) {
                return DB[seg].charAt((b1 & 0xf) * ($B2MAX$ - $B2MIN$ + 1) + (b2 - $B2MIN$));
            }
            return UNMAPPABLE_DECODING;
        }
    }

    static class Encoder extends $DBCS_BASE$.Encoder {
        private final static char[] c2b = new char[$C2BLENGTH$];
        private final static char[] c2bIndex = new char[0x100];
        private static volatile boolean initialized = false;

        Encoder(Charset cs) {
            super(cs);
            init();
        }

        int encodeChar(char ch) {
            return c2b[c2bIndex[ch >> 8] + (ch & 0xff)];
        }

        // init the c2b and c2bIndex tables from b2c tables.
        $NONROUNDTRIP_B2C$
        $NONROUNDTRIP_C2B$

        private static void init() {
            if (initialized)
                return;
            synchronized (Encoder.class) {
                if (initialized)
                    return;
                Arrays.fill(c2b, (char)UNMAPPABLE_ENCODING);
                int off = 0x100;    // The first 256 for "unmappable"
    
                String SB = Decoder.SB;
                for (int i = 0; i < SB.length(); i++) {  // SingleByte
                    char c = SB.charAt(i);
                    if (c == UNMAPPABLE_DECODING)
                        continue;
                    int index = c2bIndex[c >> 8];
                    if (index == 0) {
                        index = off;
                        off += 0x100;
                        c2bIndex[c >> 8] = (char)index;
                    }
                    c2b[index + (c & 0xff)] = (char)i;
                }

                for (int i = 0; i < Decoder.DB.length; i++) {  // DoubleByte
                    if (Decoder.DB[i] == null)
                        continue;
                    char[] db = Decoder.DB[i].toCharArray();
                    if (b2cNR != null) {    // we have b->c non-roundtrip entries
                        int j = 0;          // remove them
                        while (j < b2cNR.length()) {
                            char b  = b2cNR.charAt(j);
                            j += 2;         // ignore the "c" in <b,c> pair
			    if ((b >> 12) == i) {
                                db[((b >> 8) & 0xf) * ($B2MAX$ - $B2MIN$ + 1)
                                   + (b & 0xff) - $B2MIN$]
                                = UNMAPPABLE_ENCODING;
                            }
                        }
                    }
                    int sOff = 0;
                    int start = i << 4;
                    for (int b1 = start; b1 <= (start + 0xf); b1++) {
                        for (int b2 = $B2MIN$; b2 <= $B2MAX$; b2++) {
                            char c = db[sOff++];
                            if (c == UNMAPPABLE_DECODING)
                                continue;
                            int index = c2bIndex[c >> 8];
                            if (index == 0) {
                                index = off;
                                off += 0x100;
                                c2bIndex[c >> 8] = (char)index;
                            }
                            c2b[index + (c & 0xff)] = (char)((b1 << 8) | b2);
                        }
                    }
                }

                if (c2bNR != null) {        // c->b non-roundtrip entries
                    int i = 0;
                    while (i < c2bNR.length()) {
                        char b = c2bNR.charAt(i++);
                        char c = c2bNR.charAt(i++);
                        int index = c2bIndex[c >> 8];
                        if (index == 0) {
                            index = off;
                            off += 0x100;
                            c2bIndex[c >> 8] = (char)index;
                        }
                        c2b[index + (c & 0xff)] = (char)b;
                    }
                }
                initialized = true;
    	    }
        }
    }
}
    
