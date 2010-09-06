/*
 * @(#)NormalizerImpl.java	1.4 03/12/19
 *
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *******************************************************************************
 * (C) Copyright IBM Corp. 1996-2003 - All Rights Reserved                     *
 *                                                                             *
 * The original version of this source code and documentation is copyrighted   *
 * and owned by IBM, These materials are provided under terms of a License     *
 * Agreement between IBM and Sun. This technology is protected by multiple     *
 * US and International patents. This notice and attribution to IBM may not    *
 * to removed.                                                                 *
 *******************************************************************************
 */

 
package sun.text;

import java.io.IOException;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.ByteArrayInputStream;

/**
 * @version     1.0
 * @author  Ram Viswanadha
 */
public final class NormalizerImpl 
{
    // Static block for the class to initialize its own self 
    static final NormalizerImpl IMPL; 
    
    static
    {
        try {
            IMPL = new NormalizerImpl();
        } catch (Exception e) {
	    RuntimeException ex = new RuntimeException(e.getMessage());
	    ex.initCause(e);
	    throw ex;
        }
    }
    
    static final int UNSIGNED_BYTE_MASK =0xFF;
    static final long UNSIGNED_INT_MASK = 0xffffffffL;
    /**
     * This new implementation of the normalization code loads its data from
     * unorm.icu, which is generated with the gennorm tool.
     * The format of that file is described at the end of this file.
     */
    private static final String DATA_FILE_NAME = "resources/unorm.icu";
    
    //--------------- norm32 value constants ----------------
    /** 
     * quick check flags 0..3 set mean "no" for their forms
     */ 
    static final int QC_NFC=0x11;          // no|maybe
    static final int QC_NFKC=0x22;         // no|maybe 
    static final int QC_NFD=4;             // no 
    static final int QC_NFKD=8;            // no
    static final int QC_MASK=0x3f; 
    /// problem: public data access changed to private
    private static final int QC_ANY_NO=0xf;

    
    private static final int COMBINES_FWD=0x40;
    private static final int COMBINES_BACK=0x80;
    private static final int COMBINES_ANY=0xc0;

    // UnicodeData.txt combining class in bits 15.   
    private static final int CC_SHIFT=8;  
    static final int CC_MASK=0xff00;

    // 16 bits for the index to UChars and other extra data
    private static final int EXTRA_SHIFT=16;
    

    //norm32 value constants using >16 bits
    private static final long  MIN_SPECIAL    =  (long)(0xfc000000 & UNSIGNED_INT_MASK);
    private static final long  SURROGATES_TOP =  (long)(0xfff00000 & UNSIGNED_INT_MASK);
    private static final long  MIN_HANGUL     =  (long)(0xfff00000 & UNSIGNED_INT_MASK);
    private static final long  JAMO_V_TOP     =  (long)(0xfff30000 & UNSIGNED_INT_MASK);

    //indexes[] value names
    //number of bytes in normalization trie
    private static final int INDEX_TRIE_SIZE          = 0;
    //number of chars in extra data
    private static final int INDEX_CHAR_COUNT         = 1;    
    //number of uint16_t words for combining data
    private static final int INDEX_COMBINE_DATA_COUNT = 2;
    //number of code points that combine forward
    private static final int INDEX_COMBINE_FWD_COUNT  = 3;
    // number of code points that combine forward and backward 
    private static final int INDEX_COMBINE_BOTH_COUNT = 4;
    //number of code points that combine backward
    private static final int INDEX_COMBINE_BACK_COUNT = 5;     
    //first code point with quick check NFC NO/MAYBE
    static final int INDEX_MIN_NFC_NO_MAYBE   = 6;
    //first code point with quick check NFKC NO/MAYBE
    static final int INDEX_MIN_NFKC_NO_MAYBE  = 7;
    //first code point with quick check NFD NO/MAYBE
    static final int INDEX_MIN_NFD_NO_MAYBE   = 8;
    //first code point with quick check NFKD NO/MAYBE
    static final int INDEX_MIN_NFKD_NO_MAYBE  = 9;     
    //number of bytes in FCD trie
    private static final int INDEX_FCD_TRIE_SIZE      = 10;
    //number of bytes in the auxiliary trie
    private static final int INDEX_AUX_TRIE_SIZE      = 11;
    //number of uint16_t in the array of serialized USet
    private static final int INDEX_CANON_SET_COUNT    = 12;    
    //changing this requires a new formatVersion
    private static final int INDEX_TOP                = 32;    
    
    private static final int MAX_BUFFER_SIZE                    = 20;

    // Wrappers for Trie implementations
    private static final class NormTrieImpl implements Trie.DataManipulate{
        static IntTrie normTrie= null;
       /**
        * Called by com.ibm.icu.util.Trie to extract from a lead surrogate's 
        * data the index array offset of the indexes for that lead surrogate.
        * @param property data value for a surrogate from the trie, including 
        *         the folding offset
        * @return data offset or 0 if there is no data for the lead surrogate
        */
        // normTrie: 32-bit trie result may contain a special extraData index 
        // with the folding offset
        public int getFoldingOffset(int value){
            return  BMP_INDEX_LENGTH+
                    ((value>>(EXTRA_SHIFT-SURROGATE_BLOCK_BITS))&
                    (0x3ff<<SURROGATE_BLOCK_BITS)); 
        }
        
    }
    
    private static final class FCDTrieImpl implements Trie.DataManipulate{
        static CharTrie fcdTrie=null;
       /**
        * Called by com.ibm.icu.util.Trie to extract from a lead surrogate's 
        * data the index array offset of the indexes for that lead surrogate.
        * @param property data value for a surrogate from the trie, including
        *         the folding offset
        * @return data offset or 0 if there is no data for the lead surrogate
        */
        // fcdTrie: the folding offset is the lead FCD value itself
        public int getFoldingOffset(int value){
            return value;
        }
    }
    
    private static FCDTrieImpl fcdTrieImpl;
    private static NormTrieImpl normTrieImpl;
    private static int[] indexes;
    private static char[] combiningTable;
    private static char[] extraData;
    
    private static boolean isDataLoaded;
    
    /**
     * Default buffer size of datafile
     */
    private static final int DATA_BUFFER_SIZE = 25000;
    
    /**
     * FCD check: everything below this code point is known to have a 0 
     * lead combining class 
     */
    static final int MIN_WITH_LEAD_CC=0x300;

    /**
     * Bit 7 of the length byte for a decomposition string in extra data is
     * a flag indicating whether the decomposition string is
     * preceded by a 16-bit word with the leading and trailing cc
     * of the decomposition (like for A-umlaut);
     * if not, then both cc's are zero (like for compatibility ideographs).
     */
    private static final int DECOMP_FLAG_LENGTH_HAS_CC=0x80;
    /**
     * Bits 6..0 of the length byte contain the actual length.
     */
    private static final int DECOMP_LENGTH_MASK=0x7f;   
    
    /** 
     * Length of the BMP portion of the index (stage 1) array.
     */
    private static final int BMP_INDEX_LENGTH=0x10000>>Trie.INDEX_STAGE_1_SHIFT_;
    /** 
     * Number of bits of a trail surrogate that are used in index table 
     * lookups. 
     */
    private static final int SURROGATE_BLOCK_BITS=10-Trie.INDEX_STAGE_1_SHIFT_;

    // public utility
    static int getFromIndexesArr(int index){
        return indexes[index];
    }
   
    // private constructor ---------------------------------------------
    
    /**
    * Constructor
    */
    private NormalizerImpl() throws Exception{
        //data should be loaded only once
        if(!isDataLoaded){
            Object ret = java.security.AccessController.doPrivileged(
			      new java.security.PrivilegedAction() {
	    public Object run() {            
	        try {
                // jar access
                InputStream i = getClass().getResourceAsStream(DATA_FILE_NAME);
                BufferedInputStream b = new BufferedInputStream(i,DATA_BUFFER_SIZE);
                NormalizerDataReader reader = new NormalizerDataReader(b);
            
                // read the indexes            
                indexes = reader.readIndexes(NormalizerImpl.INDEX_TOP);
            
                byte[] normBytes = new byte[indexes[NormalizerImpl.INDEX_TRIE_SIZE]];
            
                int combiningTableTop = indexes[NormalizerImpl.INDEX_COMBINE_DATA_COUNT];
                combiningTable = new char[combiningTableTop];

                int extraDataTop = indexes[NormalizerImpl.INDEX_CHAR_COUNT];
                extraData = new char[extraDataTop];

                byte[] fcdBytes = new byte[indexes[NormalizerImpl.INDEX_FCD_TRIE_SIZE]];

                fcdTrieImpl = new FCDTrieImpl();
                normTrieImpl = new NormTrieImpl();
                        
                // load the rest of the data data and initialize the data members
                reader.read(normBytes, fcdBytes, null, extraData, combiningTable, null);

                NormTrieImpl.normTrie = new IntTrie(new ByteArrayInputStream(normBytes), normTrieImpl);
                FCDTrieImpl.fcdTrie   = new CharTrie(new ByteArrayInputStream(fcdBytes), fcdTrieImpl);

                // we reached here without any exceptions so the data is fully 
                // loaded set the variable to true
                isDataLoaded = true;
            
                b.close();
                i.close();
                return null;
                } catch (Exception e) {
                    return e;
                }
	    }});
            if (ret instanceof Exception) {
	        throw (Exception)ret;
            }
	}
    }
        
    // Korean Hangul and Jamo constants
    
    private static final int JAMO_L_BASE=0x1100;     // "lead" jamo
    private static final int JAMO_V_BASE=0x1161;     // "vowel" jamo 
    private static final int JAMO_T_BASE=0x11a7;     // "trail" jamo 
    
    private static final int HANGUL_BASE=0xac00;
    
    private static final int JAMO_L_COUNT=19;
    private static final int JAMO_V_COUNT=21;
    private static final int JAMO_T_COUNT=28;
    private static final int HANGUL_COUNT=
                                        JAMO_L_COUNT*JAMO_V_COUNT*JAMO_T_COUNT;
    
    private static boolean isHangulWithoutJamoT(char c) {
        c-=HANGUL_BASE;
        return c<HANGUL_COUNT && c%JAMO_T_COUNT==0;
    }
    
    // norm32 helpers

    /** 
     * is this a norm32 with a regular index?
     */  
    private static boolean isNorm32Regular(long norm32) {
        return norm32<MIN_SPECIAL;
    }
    
    /** 
     * is this a norm32 with a special index for a lead surrogate?
     */
    private static boolean isNorm32LeadSurrogate(long norm32) {
        return MIN_SPECIAL<=norm32 && norm32<SURROGATES_TOP;
    }
    
    /** 
     * is this a norm32 with a special index for a Hangul syllable or a Jamo?
     */
    // sherman private static boolean isNorm32HangulOrJamo(long norm32) {
    static boolean isNorm32HangulOrJamo(long norm32) {
        return norm32>=MIN_HANGUL;
    }
    
    /**
     * Given norm32 for Jamo V or T,
     * is this a Jamo V?
     */
    private static boolean isJamoVTNorm32JamoV(long norm32) {
        return norm32<JAMO_V_TOP;
    }

    // data access primitives -----------------------------------------------
    static long getNorm32(char c) {
        return ((UNSIGNED_INT_MASK) & (NormTrieImpl.normTrie.getLeadValue(c)));
    }

    static long getNorm32FromSurrogatePair(long norm32, char c2) {
        // the surrogate index in norm32 stores only the number of the 
        // surrogate index block see gennorm/store.c/getFoldedNormValue()
        return ((UNSIGNED_INT_MASK) & 
                    NormTrieImpl.normTrie.getTrailValue((int)norm32, c2));
    }

    private static long getNorm32(int c)
    {
        return (UNSIGNED_INT_MASK&(NormTrieImpl.normTrie.getCodePointValue(c)));
    }
    
    /**
     * get a norm32 from text with complete code points
     * (like from decompositions)
     */
    private static long getNorm32(char[] p,int start, int mask) {
        long norm32= getNorm32(p[start]);
        if(((norm32&mask)>0) && isNorm32LeadSurrogate(norm32)) {
            // *p is a lead surrogate, get the real norm32 
            norm32=getNorm32FromSurrogatePair(norm32, p[start+1]);
        }
        return norm32;
    }

    public static char getFCD16(char c) {
        return  FCDTrieImpl.fcdTrie.getLeadValue(c);
    }
    
    public static char getFCD16FromSurrogatePair(char fcd16, char c2) {
        // the surrogate index in fcd16 is an absolute offset over the 
        // start of stage 1
        return FCDTrieImpl.fcdTrie.getTrailValue(fcd16, c2);
    }
        
    private static int getExtraDataIndex(long norm32) {
        return (int)(norm32>>EXTRA_SHIFT);
    }
    
    private static final class DecomposeArgs{
        int cc;
        int trailCC;
        int length;
    }
    
    /**
     * get the canonical or compatibility decomposition for one character 
     * 
     * @return index into the extraData array
     */
    private static int decompose(long norm32, int qcMask, DecomposeArgs args) 
    {
        int p= getExtraDataIndex(norm32);
        args.length=extraData[p++];
    
        if((norm32&qcMask&QC_NFKD)!=0 && args.length>=0x100) {
            // use compatibility decomposition, skip canonical data 
            p+=((args.length>>7)&1)+(args.length&DECOMP_LENGTH_MASK);
            args.length>>=8;
        }
    
        if((args.length&DECOMP_FLAG_LENGTH_HAS_CC)>0) {
            // get the lead and trail cc's 
            char bothCCs=extraData[p++];
            args.cc=(UNSIGNED_BYTE_MASK) & (bothCCs>>8);
            args.trailCC=(UNSIGNED_BYTE_MASK) & bothCCs;
        } else {
            // lead and trail cc's are both 0 
            args.cc=args.trailCC=0;
        }
    
        args.length&=DECOMP_LENGTH_MASK;
        return p;
    }
        
    /**
     * get the canonical decomposition for one character 
     * @return index into the extraData array
     */
    private static int decompose(long norm32, 
                                 DecomposeArgs args) {
                                
        int p= getExtraDataIndex(norm32);
        args.length=extraData[p++];
        
        if((args.length&DECOMP_FLAG_LENGTH_HAS_CC)>0) {
            // get the lead and trail cc's 
            char bothCCs=extraData[p++];
            args.cc=(UNSIGNED_BYTE_MASK) & (bothCCs>>8);
            args.trailCC=(UNSIGNED_BYTE_MASK) & bothCCs;
        } else {
            // lead and trail cc's are both 0 
            args.cc=args.trailCC=0;
        }
        
        args.length&=DECOMP_LENGTH_MASK;
        return p;
    }
    
    private static final class NextCCArgs{
        char[] source;
        int next;
        int limit;
        char c;
        char c2;
    }
    
    /**
     * get the combining class of (c, c2)= args.source[args.next++]
     * before: args.next<args.limit  after: args.next<=args.limit
     * if only one code unit is used, then c2==0
     */
    private static int getNextCC(NextCCArgs args) {
        long norm32;
    
        args.c=args.source[args.next++];
        
        norm32= getNorm32(args.c);
        if((norm32 & CC_MASK)==0) {
            args.c2=0;
            return 0;
        } else {
            if(!isNorm32LeadSurrogate(norm32)) {
                args.c2=0;
            } else {
                /* c is a lead surrogate, get the real norm32 */
                if(args.next!=args.limit && 
                        Character.isLowSurrogate(args.c2=args.source[args.next])){
                    ++args.next;
                    norm32=getNorm32FromSurrogatePair(norm32, args.c2);
                } else {
                    args.c2=0;
                    return 0;
                }
            }
    
            return (int)((UNSIGNED_BYTE_MASK) & (norm32>>CC_SHIFT));
        }
    }

    private static final class PrevArgs{
        char[] src;
        int start;
        int current;
        char c;
        char c2;
    }
    
    /**
     * read backwards and get norm32
     * return 0 if the character is <minC
     * if c2!=0 then (c2, c) is a surrogate pair (reversed - c2 is first 
     * surrogate but read second!)
     */
    private static long getPrevNorm32(PrevArgs args, int minC, int mask) {
        long norm32;
    
        args.c=args.src[--args.current];
        args.c2=0;
    
        // check for a surrogate before getting norm32 to see if we need to 
        // predecrement further 
        if(args.c<minC) {
            return 0;
	// sherman/Note: need isSurrogate()
	//} else if(!UTF16.isSurrogate(args.c)) {
        } else if(args.c < Character.MIN_HIGH_SURROGATE ||
                  args.c > Character.MAX_LOW_SURROGATE) {
            return getNorm32(args.c);
        } else if(Character.isHighSurrogate(args.c)) {
            // unpaired first surrogate 
            return 0;
        } else if(args.current!=args.start && 
                    Character.isHighSurrogate(args.c2=args.src[args.current-1])) {
            --args.current;
            norm32=getNorm32(args.c2);
    
            if((norm32&mask)==0) {
                // all surrogate pairs with this lead surrogate have 
                // only irrelevant data
                return 0;
            } else {
                // norm32 must be a surrogate special
                return getNorm32FromSurrogatePair(norm32, args.c);
            }
        } else {
            // unpaired second surrogate
            args.c2=0;
            return 0;
        }
    }
    
    /**
     * get the combining class of (c, c2)=*--p
     * before: start<p  after: start<=p
     */
    private static int getPrevCC(PrevArgs args) {

        return (int)((UNSIGNED_BYTE_MASK)&(getPrevNorm32(args, MIN_WITH_LEAD_CC,
                                                         CC_MASK)>>CC_SHIFT));
    }

    /**
     * is this a safe boundary character for NF*D?
     * (lead cc==0)
     */
    static boolean isNFDSafe(long norm32, int ccOrQCMask, int decompQCMask) {
        if((norm32&ccOrQCMask)==0) {
            return true; /* cc==0 and no decomposition: this is NF*D safe */
        }
    
        /* inspect its decomposition - maybe a Hangul but not a surrogate here*/
        if(isNorm32Regular(norm32) && (norm32&decompQCMask)!=0) {
            DecomposeArgs args=new DecomposeArgs();
            /* decomposes, get everything from the variable-length extra data */
            decompose(norm32, decompQCMask, args);
            return args.cc==0;
        } else {
            /* no decomposition (or Hangul), test the cc directly */
            return (norm32&CC_MASK)==0;
        }
    }
    
    /**
     * is this (or does its decomposition begin with) a "true starter"?
     * (cc==0 and NF*C_YES)
     */
    static boolean isTrueStarter(long norm32, int ccOrQCMask, int decompQCMask) 
    {
        if((norm32&ccOrQCMask)==0) {
            return true; // this is a true starter (could be Hangul or Jamo L)
        }
    
        // inspect its decomposition - not a Hangul or a surrogate here
        if((norm32&decompQCMask)!=0) {
            int p; // index into extra data array 
            DecomposeArgs args=new DecomposeArgs();
            // decomposes, get everything from the variable-length extra data
            p=decompose(norm32, decompQCMask, args);
          
            if(args.cc==0) {
                int qcMask=ccOrQCMask&QC_MASK;
    
                // does it begin with NFC_YES?
                if((getNorm32(extraData,p, qcMask)&qcMask)==0) {
                    // yes, the decomposition begins with a true starter
                    return true;
                }
            }
        }
        return false;
    }

    // reorder UTF-16 in-place ---------------------------------------------- 
    /**
     * simpler, single-character version of mergeOrdered() -
     * bubble-insert one single code point into the preceding string
     * which is already canonically ordered
     * (c, c2) may or may not yet have been inserted at src[current]..src[p]
     *
     * it must be p=current+lengthof(c, c2) i.e. p=current+(c2==0 ? 1 : 2)
     *
     * before: src[start]..src[current] is already ordered, and
     *         src[current]..src[p]     may or may not hold (c, c2) but
     *                          must be exactly the same length as (c, c2)
     * after: src[start]..src[p] is ordered
     *
     * @return the trailing combining class
     */
    private static int insertOrdered(char[] source, int start, int current, 
                                     int p, char c, char c2, int cc) {
        int back, preBack;
        int r;
        int prevCC, trailCC=cc;

        if(start<current && cc!=0) {
            // search for the insertion point where cc>=prevCC 
            preBack=back=current;
            PrevArgs prevArgs = new PrevArgs();
            prevArgs.current  = current;
            prevArgs.start    = start;
            prevArgs.src      = source;
            // get the prevCC 
            prevCC=getPrevCC(prevArgs);
            preBack = prevArgs.current;
            
            if(cc<prevCC) {
                // this will be the last code point, so keep its cc 
                trailCC=prevCC;
                back=preBack;
                while(start<preBack) {
                    prevCC=getPrevCC(prevArgs);
                    preBack=prevArgs.current;
                    if(cc>=prevCC) {
                        break;
                    }
                    back=preBack;
                }
    
                // this is where we are right now with all these indicies:
                // [start]..[pPreBack] 0..? code points that we can ignore
                // [pPreBack]..[pBack] 0..1 code points with prevCC<=cc
                // [pBack]..[current] 0..n code points with >cc, move up to insert (c, c2)
                // [current]..[p]         1 code point (c, c2) with cc
                 
                // move the code units in between up 
                r=p;
                do {
                    source[--r]=source[--current];
                } while(back!=current);
            }
        }
    
        // insert (c, c2) 
        source[current]=c;
        if(c2!=0) {
            source[(current+1)]=c2;
        }
    
        // we know the cc of the last code point 
        return trailCC;
    }

    /**
     * merge two UTF-16 string parts together
     * to canonically order (order by combining classes) their concatenation
     *
     * the two strings may already be adjacent, so that the merging is done 
     * in-place if the two strings are not adjacent, then the buffer holding the
     * first one must be large enough
     * the second string may or may not be ordered in itself
     *
     * before: [start]..[current] is already ordered, and
     *         [next]..[limit]    may be ordered in itself, but
     *                          is not in relation to [start..current[
     * after: [start..current+(limit-next)[ is ordered
     *
     * the algorithm is a simple bubble-sort that takes the characters from 
     * src[next++] and inserts them in correct combining class order into the 
     * preceding part of the string
     *
     * since this function is called much less often than the single-code point
     * insertOrdered(), it just uses that for easier maintenance
     *
     * @return the trailing combining class
     */
    private static int mergeOrdered(char[] source, int start, int current,
                                    char[] data, int next, int limit, 
                                    boolean isOrdered) {
        int r;
        int cc, trailCC=0;
        boolean adjacent;
    
        adjacent= current==next;
        NextCCArgs ncArgs = new NextCCArgs();
        ncArgs.source = data;
        ncArgs.next   = next;
        ncArgs.limit  = limit;
        
        if(start!=current || !isOrdered) {
                
            while(ncArgs.next<ncArgs.limit) {
                cc=getNextCC(ncArgs);
                if(cc==0) {
                    // does not bubble back 
                    trailCC=0;
                    if(adjacent) {
                        current=ncArgs.next;
                    } else {
                        data[current++]=ncArgs.c;
                        if(ncArgs.c2!=0) {
                            data[current++]=ncArgs.c2;
                        }
                    }
                    if(isOrdered) {
                        break;
                    } else {
                        start=current;
                    }
                } else {
                    r=current+(ncArgs.c2==0 ? 1 : 2);
                    trailCC=insertOrdered(source,start, current, r, 
                                          ncArgs.c, ncArgs.c2, cc);
                    current=r;
                }
            }
        }
    
        if(ncArgs.next==ncArgs.limit) {
            // we know the cc of the last code point 
            return trailCC;
        } else {
            if(!adjacent) {
                // copy the second string part 
                do {
                    source[current++]=data[ncArgs.next++];
                } while(ncArgs.next!=ncArgs.limit);
                ncArgs.limit=current;
            }
            PrevArgs prevArgs = new PrevArgs();
            prevArgs.src   = data;
            prevArgs.start = start;
            prevArgs.current =  ncArgs.limit;
            return getPrevCC(prevArgs);
        }

    }
    
    private static int mergeOrdered(char[] source, int start, int current,
                                    char[] data, final int next, 
                                    final int limit) {
        return mergeOrdered(source,start,current,data,next,limit,true);
    } 
      
    /// problem: public method access changed to package private
    static boolean checkFCD(char[] src,int srcStart, int srcLimit, 
                            int options) {
        char fcd16,c,c2;
        int prevCC=0, cc;
        int i =srcStart, length = srcLimit;
    
        for(;;) {
            for(;;) {
                if(i==length) {
                    return true;
                } else if((c=src[i++])<MIN_WITH_LEAD_CC) {
                    prevCC=(int)-c;
                } else if((fcd16=getFCD16(c))==0) {
                    prevCC=0;
                } else {
                    break;
                }
            }

            // check one above-minimum, relevant code unit 
            if(Character.isHighSurrogate(c)) {
                // c is a lead surrogate, get the real fcd16 
                if(i!=length && Character.isLowSurrogate(c2=src[i])) {
                    ++i;
                    fcd16=getFCD16FromSurrogatePair(fcd16, c2);
                } else {
                    c2=0;
                    fcd16=0;
		}
            }else{
                c2=0;
            }
            
            if(nx_contains(options, c, c2)) {
                prevCC=0; /* excluded: fcd16==0 */
                continue;
            }

            // prevCC has values from the following ranges:
            // 0..0xff -the previous trail combining class
            // <0      -the negative value of the previous code unit;
            //          that code unit was <MIN_WITH_LEAD_CC and its getFCD16()
            //          was deferred so that average text is checked faster
            //
    
            // check the combining order 
            cc=(int)(fcd16>>8);
            if(cc!=0) {
                if(prevCC<0) {
                    // the previous character was <_NORM_MIN_WITH_LEAD_CC, 
                    // we need to get its trail cc 
                    //
                    if(!nx_contains(options, (int)-prevCC)) {
                        prevCC=(int)(FCDTrieImpl.fcdTrie.getBMPValue(
                                             (char)-prevCC)&0xff
                                             ); 
                    } else {
                        prevCC=0; /* excluded: fcd16==0 */
                    }
                                      
                }
    
                if(cc<prevCC) {
                    return false;
                }
            }
            prevCC=(int)(fcd16&0xff);
        }
    }
    
    static Normalizer.QuickCheckResult quickCheck(char[] src, int srcStart, 
                                                  int srcLimit, int minNoMaybe,
                                                  int qcMask,
                                                  boolean allowMaybe,
                                                  int options){
        int ccOrQCMask;
        long norm32;
        char c, c2;
        char cc, prevCC;
        long qcNorm32;
        Normalizer.QuickCheckResult result;
        ComposePartArgs args = new ComposePartArgs();
        char[] buffer ;
        int start = srcStart;
        
        if(!isDataLoaded) {
            return Normalizer.MAYBE;
        }
        // initialize 
        ccOrQCMask=CC_MASK|qcMask;
        result=Normalizer.YES;
        prevCC=0;
                
        for(;;) {
            for(;;) {
                if(srcStart==srcLimit) {
                    return result;
                } else if((c=src[srcStart++])>=minNoMaybe && 
                                  (( norm32=getNorm32(c)) & ccOrQCMask)!=0) {
                    break;
                }
                prevCC=0;
            }
            
    
            // check one above-minimum, relevant code unit 
            if(isNorm32LeadSurrogate(norm32)) {
                // c is a lead surrogate, get the real norm32 
                if(srcStart!=srcLimit&& Character.isLowSurrogate(c2=src[srcStart])) {
                    ++srcStart;
                    norm32=getNorm32FromSurrogatePair(norm32,c2);
                } else {
                    norm32=0;
                    c2=0;
                }
            }else{
                c2=0;
            }
            if(nx_contains(options, c, c2)) {
                /* excluded: norm32==0 */
                norm32=0;
            }
    
            // check the combining order 
            cc=(char)((norm32>>CC_SHIFT)&0xFF);
            if(cc!=0 && cc<prevCC) {
                return Normalizer.NO;
            }
            prevCC=cc;
    
            // check for "no" or "maybe" quick check flags 
            qcNorm32 = norm32 & qcMask;
            if((qcNorm32& QC_ANY_NO)>=1) {
                result= Normalizer.NO;
                break;
            } else if(qcNorm32!=0) {
                // "maybe" can only occur for NFC and NFKC 
                if(allowMaybe){
                    result=Normalizer.MAYBE;
                }else{
                    // normalize a section around here to see if it is really 
                    // normalized or not 
                    int prevStarter;
                    int/*unsigned*/ decompQCMask;
    
                    decompQCMask=(qcMask<<2)&0xf; // decomposition quick check mask 
    
                    // find the previous starter 
                    
                    // set prevStarter to the beginning of the current character 
                    prevStarter=srcStart-1; 
                    if(Character.isLowSurrogate(src[prevStarter])) {
                        // safe because unpaired surrogates do not result 
                        // in "maybe"
                        --prevStarter; 
                    }
                    prevStarter=findPreviousStarter(src, start, prevStarter,
                                                    ccOrQCMask, decompQCMask,
                                                    (char)minNoMaybe);
    
                    // find the next true starter in [src..limit[ - modifies 
                    // src to point to the next starter 
                    srcStart=findNextStarter(src,srcStart, srcLimit, qcMask, 
                                             decompQCMask,(char) minNoMaybe);
                    
                    //set the args for compose part
                    args.prevCC = prevCC;
                       
                    // decompose and recompose [prevStarter..src[ 
                    buffer = composePart(args,prevStarter,src,srcStart,srcLimit,qcMask,options);
    
                    // compare the normalized version with the original 
                    if(0!=strCompare(buffer,0,args.length,src,prevStarter,(srcStart-prevStarter), false)) {
                        result=Normalizer.NO; // normalization differs 
                        break;
                    }
    
                    // continue after the next starter 
                }
            }
        }
        return result;
    } 

    //------------------------------------------------------ 
    // special method for Collation 
    //------------------------------------------------------
    private static boolean needSingleQuotation(char c) {
        return (c >= 0x0009 && c <= 0x000D) ||
               (c >= 0x0020 && c <= 0x002F) ||
               (c >= 0x003A && c <= 0x0040) ||
               (c >= 0x005B && c <= 0x0060) ||
               (c >= 0x007B && c <= 0x007E);
    }

    public static String canonicalDecomposeWithSingleQuotation(String string) {
        char[] src = string.toCharArray();
        int    srcIndex = 0;
        int    srcLimit = src.length;
        char[] dest = new char[src.length * 3];  //MAX_BUF_SIZE_DECOMPOSE = 3
        int    destIndex = 0; 
        int    destLimit = dest.length;

        char[] buffer = new char[3];
        int prevSrc;
        long norm32;
        int ccOrQCMask; 
        int qcMask = QC_NFD;
        int reorderStartIndex, length;
        char c, c2;
        char minNoMaybe = (char)indexes[INDEX_MIN_NFD_NO_MAYBE];
        int cc, prevCC, trailCC;
        char[] p;
        int pStart;

    
        // initialize
        ccOrQCMask = CC_MASK | qcMask;
        reorderStartIndex = 0;
        prevCC = 0;
        norm32 = 0;
        c = 0;
        pStart = 0;
        
        cc = trailCC = -1; // initialize to bogus value
        for(;;) {
            prevSrc=srcIndex;
            //quick check (1)less than minNoMaybe (2)no decomp (3)hangual
            while (srcIndex != srcLimit &&
                   (( c = src[srcIndex]) < minNoMaybe || 
		    ((norm32 = getNorm32(c)) & ccOrQCMask) == 0 ||
                    ( c >= '\uac00' && c <= '\ud7a3'))){

                prevCC = 0;
                ++srcIndex;
            }

            // copy these code units all at once 
            if (srcIndex != prevSrc) {
                length = (int)(srcIndex - prevSrc);
                if ((destIndex + length) <= destLimit) {
                    System.arraycopy(src,prevSrc,dest,destIndex,length);
                }
              
                destIndex += length;
                reorderStartIndex = destIndex;
            }
    
            // end of source reached? 
            if(srcIndex == srcLimit) {
                break;
            }
            // c already contains *src and norm32 is set for it, increment src
            ++srcIndex;

            if(isNorm32Regular(norm32)) {
                c2 = 0;
                length = 1;
            } else {
                // c is a lead surrogate, get the real norm32 
                if(srcIndex != srcLimit && 
		    Character.isLowSurrogate(c2 = src[srcIndex])) {
		        ++srcIndex;
                        length = 2;
                        norm32 = getNorm32FromSurrogatePair(norm32, c2);
		} else {
                    c2 = 0;
                    length = 1;
                    norm32 = 0;
                }
	    }
    
            // get the decomposition and the lead and trail cc's 
            if((norm32 & qcMask) == 0) {
                // c does not decompose
                cc = trailCC = (int)((UNSIGNED_BYTE_MASK) & (norm32 >> CC_SHIFT));
                p = null;
                pStart = -1;
	    } else {
                DecomposeArgs arg = new DecomposeArgs();
                // c decomposes, get everything from the variable-length 
                // extra data
                pStart = decompose(norm32, qcMask, arg);
                p = extraData;
                length = arg.length;
                cc = arg.cc;
                trailCC = arg.trailCC;
                if(length == 1) {
                    // fastpath a single code unit from decomposition 
                    c = p[pStart];
                    c2 = 0;
                    p = null;
                    pStart = -1;
                }
	    }

            if((destIndex + length * 3) >= destLimit) {  // 2 SingleQuotations 
                // buffer overflow 
                char[] tmpBuf = new char[destLimit * 2];
                System.arraycopy(dest, 0, tmpBuf, 0, destIndex);            
                dest = tmpBuf;
                destLimit = dest.length;
            }
            // append the decomposition to the destination buffer, assume length>0
	    {
                int reorderSplit = destIndex;
                if(p == null) {
                    // fastpath: single code point
                    if (needSingleQuotation(c)) {
		        //if we need single quotation, no need to consider "prevCC"
                        //and it must NOT be a supplementary pair
                        dest[destIndex++] = '\'';
                        dest[destIndex++] = c;
                        dest[destIndex++] = '\'';
                        trailCC = 0;
                    } else if(cc != 0 && cc < prevCC) {
                        // (c, c2) is out of order with respect to the preceding
                        //  text 
                        destIndex += length;
                        trailCC = insertOrdered(dest,reorderStartIndex, 
                                                reorderSplit, destIndex, c, c2, cc);
                    } else {
                        // just append (c, c2)
                        dest[destIndex++] = c;
                        if(c2 != 0) {
                            dest[destIndex++] = c2;
                        }
                    }
                } else {
                    // general: multiple code points (ordered by themselves) 
                    // from decomposition 
                    if (needSingleQuotation(p[pStart])) {
                        dest[destIndex++] = '\'';
                        dest[destIndex++] = p[pStart++];
                        dest[destIndex++] = '\'';
                        length--;
			do {
                            dest[destIndex++] = p[pStart++];
                        } while(--length > 0);
		    } else
                    if(cc != 0 && cc < prevCC) {
                        destIndex += length;
                        trailCC = mergeOrdered(dest,reorderStartIndex, 
                                               reorderSplit,p, pStart,pStart+length);
                    } else {
                        // just append the decomposition 
			do {
                            dest[destIndex++] = p[pStart++];
                        } while(--length > 0);
                    }
                }
            }
            prevCC = trailCC;
            if(prevCC == 0) {
                reorderStartIndex = destIndex;
            }
        }
        return new String(dest, 0, destIndex);
    }


    //------------------------------------------------------ 
    // make NFD & NFKD 
    //------------------------------------------------------

    static int decompose(char[] src,int srcIndex,int srcLimit, char[] dest,
                         int destIndex,int destLimit, boolean compat,
                         int[] outTrailCC, int options) {
        char[] buffer = new char[3];
        int prevSrc;
        long norm32;
        int ccOrQCMask, qcMask;
        int reorderStartIndex, length;
        char c, c2, minNoMaybe;
        int cc, prevCC, trailCC;
        char[] p;
        int pStart;
        if(!compat) {
            minNoMaybe=(char)indexes[INDEX_MIN_NFD_NO_MAYBE];
            qcMask=QC_NFD;
        } else {
            minNoMaybe=(char)indexes[INDEX_MIN_NFKD_NO_MAYBE];
            qcMask=QC_NFKD;
        }
    
        // initialize
        ccOrQCMask=CC_MASK|qcMask;
        reorderStartIndex=0;
        prevCC=0;
        norm32=0;
        c=0;
        pStart=0;
        
        cc=trailCC=-1; // initialize to bogus value
        
        for(;;) {
            // count code units below the minimum or with irrelevant data for 
            // the quick check 
            prevSrc=srcIndex;

            while(srcIndex!=srcLimit &&((c=src[srcIndex])<minNoMaybe || 
                                        ((norm32=getNorm32(c))&ccOrQCMask)==0)){
                prevCC=0;
                ++srcIndex;
            }

            // copy these code units all at once 
            if(srcIndex!=prevSrc) {
                length=(int)(srcIndex-prevSrc);
                if((destIndex+length)<=destLimit) {
                    System.arraycopy(src,prevSrc,dest,destIndex,length);
                }
              
                destIndex+=length;
                reorderStartIndex=destIndex;
            }
    
            // end of source reached? 
            if(srcIndex==srcLimit) {
                break;
            }
            // c already contains *src and norm32 is set for it, increment src
            ++srcIndex;

            // check one above-minimum, relevant code unit
            /*
             * generally, set p and length to the decomposition string
             * in simple cases, p==NULL and (c, c2) will hold the length code 
             * units to append in all cases, set cc to the lead and trailCC to 
             * the trail combining class
             *
             * the following merge-sort of the current character into the 
             * preceding, canonically ordered result text will use the 
             * optimized insertOrdered()
             * if there is only one single code point to process;
             * this is indicated with p==NULL, and (c, c2) is the character to 
             * insert
             * ((c, 0) for a BMP character and (lead surrogate, trail surrogate)
             * for a supplementary character)
             * otherwise, p[length] is merged in with _mergeOrdered()
             */
            if(isNorm32HangulOrJamo(norm32)) {
                if(nx_contains(options, c)) {
                    c2=0;
                    p=null;
                    length=1;
                } else {
                    // Hangul syllable: decompose algorithmically 
                    p=buffer;
                    pStart=0;
                    cc=trailCC=0;
    
                    c-=HANGUL_BASE;
    
                    c2=(char)(c%JAMO_T_COUNT);
                    c/=JAMO_T_COUNT;
                    if(c2>0) {
                        buffer[2]=(char)(JAMO_T_BASE+c2);
                        length=3;
                    } else {
                        length=2;
                    }
    
                    buffer[1]=(char)(JAMO_V_BASE+c%JAMO_V_COUNT);
                    buffer[0]=(char)(JAMO_L_BASE+c/JAMO_V_COUNT);
                }
            } else {
                if(isNorm32Regular(norm32)) {
                    c2=0;
                    length=1;
                } else {
                    // c is a lead surrogate, get the real norm32 
                    if(srcIndex!=srcLimit && 
                                    Character.isLowSurrogate(c2=src[srcIndex])) {
                        ++srcIndex;
                        length=2;
                        norm32=getNorm32FromSurrogatePair(norm32, c2);
                    } else {
                        c2=0;
                        length=1;
                        norm32=0;
                    }
                }
    
                // get the decomposition and the lead and trail cc's 
                if(nx_contains(options, c, c2)) {
                    // excluded: norm32==0 
                    cc=trailCC=0;
                    p=null;
                } else if((norm32&qcMask)==0) {
                    // c does not decompose
                    cc=trailCC=(int)((UNSIGNED_BYTE_MASK) & (norm32>>CC_SHIFT));
                    p=null;
                    pStart=-1;
                } else {
                    DecomposeArgs arg = new DecomposeArgs();
                    // c decomposes, get everything from the variable-length 
                    // extra data
                    pStart=decompose(norm32, qcMask, arg);
                    p=extraData;
                    length=arg.length;
                    cc=arg.cc;
                    trailCC=arg.trailCC;
                    if(length==1) {
                        // fastpath a single code unit from decomposition 
                        c=p[pStart];
                        c2=0;
                        p=null;
                        pStart=-1;
                    }
                }
	    }
            // append the decomposition to the destination buffer, assume 
            // length>0
            if((destIndex+length)<=destLimit) {
                int reorderSplit=destIndex;
                if(p==null) {
                    // fastpath: single code point
                    if(cc!=0 && cc<prevCC) {
                        // (c, c2) is out of order with respect to the preceding
                        //  text 
                        destIndex+=length;
                        trailCC=insertOrdered(dest,reorderStartIndex, 
                                            reorderSplit, destIndex, c, c2, cc);
                    } else {
                        // just append (c, c2)
                        dest[destIndex++]=c;
                        if(c2!=0) {
                            dest[destIndex++]=c2;
                        }
                    }
                } else {
                    // general: multiple code points (ordered by themselves) 
                    // from decomposition 
                    if(cc!=0 && cc<prevCC) {
                        // the decomposition is out of order with respect to 
                        // the preceding text
                        destIndex+=length;
                        trailCC=mergeOrdered(dest,reorderStartIndex, 
                                          reorderSplit,p, pStart,pStart+length);
                    } else {
                        // just append the decomposition 
                        do {
                            dest[destIndex++]=p[pStart++];
                        } while(--length>0);
                    }
                }
            } else {
                // buffer overflow 
                // keep incrementing the destIndex for preflighting 
                destIndex+=length;
            }
    
            prevCC=trailCC;
            if(prevCC==0) {
                reorderStartIndex=destIndex;
            }
        }
    
        outTrailCC[0]=prevCC;

        return destIndex;
    }

    /** 
     * make NFC & NFKC
     */ 
    private static final class NextCombiningArgs
    {
        char[] source;
        int start;
        //int limit;
        char c;
        char c2;
        int combiningIndex;
        char cc;
    }
    
    /** 
     * get the composition properties of the next character
     */ 
    private static int getNextCombining(NextCombiningArgs args, int limit, 
					int options)
    {
        long norm32; 
        int combineFlags;
        // get properties 
        args.c=args.source[args.start++];
        norm32=getNorm32(args.c);
    
        // preset output values for most characters 
        args.c2=0;
        args.combiningIndex=0;
        args.cc=0;
    
        if((norm32&(CC_MASK|COMBINES_ANY))==0) {
            return 0;
        } else {
            if(isNorm32Regular(norm32)) {
                // set cc etc. below 
            } else if(isNorm32HangulOrJamo(norm32)) {
                // a compatibility decomposition contained Jamos 
                args.combiningIndex=(int)((UNSIGNED_INT_MASK)&(0xfff0|
                                                        (norm32>>EXTRA_SHIFT)));
                return (int)(norm32&COMBINES_ANY);
            } else {
                // c is a lead surrogate, get the real norm32 
                if(args.start!=limit && Character.isLowSurrogate(args.c2=
                                                     args.source[args.start])) {
                    ++args.start;
                    norm32=getNorm32FromSurrogatePair(norm32, args.c2);
                } else {
                    args.c2=0;
                    return 0;
                }
            }
        
            if(nx_contains(options, args.c, args.c2)) {
                return 0; // excluded: norm32==0 
            }

            args.cc= (char)(byte)(norm32>>CC_SHIFT);
    
            combineFlags=(int)(norm32&COMBINES_ANY);
            if(combineFlags!=0) {
                int index = getExtraDataIndex(norm32);
                args.combiningIndex=index>0 ? extraData[(index-1)] :0;
            }

            return combineFlags;
        }
    }
    
    /**
     * given a composition-result starter (c, c2) - which means its cc==0,
     * it combines forward, it has extra data, its norm32!=0,
     * it is not a Hangul or Jamo,
     * get just its combineFwdIndex
     *
     * norm32(c) is special if and only if c2!=0
     */
    private static int getCombiningIndexFromStarter(char c,char c2)
    {
        long norm32;

        norm32=getNorm32(c);
        if(c2!=0) {
            norm32=getNorm32FromSurrogatePair(norm32, c2);
        }
        return extraData[(getExtraDataIndex(norm32)-1)];
    }

    
    /**
     * Find the recomposition result for
     * a forward-combining character
     * (specified with a pointer to its part of the combiningTable[])
     * and a backward-combining character
     * (specified with its combineBackIndex).
     *
     * If these two characters combine, then set (value, value2)
     * with the code unit(s) of the composition character.
     *
     * Return value:
     * 0    do not combine
     * 1    combine
     * >1   combine, and the composition is a forward-combining starter
     *
     * See unormimp.h for a description of the composition table format.
     */
    private static int combine(char[] table, int tableStart, 
                               int combineBackIndex, int[] outValues) 
    {
        int key;
        int value,value2;
    
        if(outValues.length<2){
            throw new IllegalArgumentException();
        }
    
        // search in the starter's composition table 
        for(;;) {
            key=table[tableStart++];
            if(key>=combineBackIndex) {
                break;
            }
            tableStart+= ((table[tableStart]&0x8000) != 0)? 2 : 1;
        }

        // mask off bit 15, the last-entry-in-the-list flag 
        if((key&0x7fff)==combineBackIndex) {
            // found! combine! 
            value=table[tableStart];

            // is the composition a starter that combines forward? 
            key=(int)((UNSIGNED_INT_MASK)&((value&0x2000)+1));

            // get the composition result code point from the variable-length 
            // result value 
         
            if((value&0x8000) != 0) {
                if((value&0x4000) != 0) {
                    // surrogate pair composition result 
                    value=(int)((UNSIGNED_INT_MASK)&((value&0x3ff)|0xd800));
                    value2=table[tableStart+1];
                } else {
                    // BMP composition result U+2000..U+ffff 
                    value=table[tableStart+1];
                    value2=0;
                }
            } else {
                // BMP composition result U+0000..U+1fff 
                value&=0x1fff;
                value2=0;
            }
            outValues[0]=value;
            outValues[1]=value2;    
            return key;
        } else {
            // not found 
            return 0;
        }
    }

    private static final class RecomposeArgs
    {
        char[] source;
        int start;
        int limit;
    }
    
    /**
     * recompose the characters in [p..limit[
     * (which is in NFD - decomposed and canonically ordered),
     * adjust limit, and return the trailing cc
     *
     * since for NFKC we may get Jamos in decompositions, we need to
     * recompose those too
     *
     * note that recomposition never lengthens the text:
     * any character consists of either one or two code units;
     * a composition may contain at most one more code unit than the original 
     * starter, while the combining mark that is removed has at least one code 
     * unit
     */
    private static char recompose(RecomposeArgs args, int options) 
    {
        int remove, q, r;
        int combineFlags;
        int combineFwdIndex, combineBackIndex;
        int result, value=0, value2=0;
        int prevCC;
        boolean starterIsSupplementary;
        int starter;
        int[] outValues = new int[2];
        starter=-1;                   // no starter 
        combineFwdIndex=0;            // will not be used until starter!=NULL 
        starterIsSupplementary=false; // will not be used until starter!=NULL 
        prevCC=0;
    
        NextCombiningArgs ncArg = new NextCombiningArgs();
        ncArg.source  = args.source;
    
        ncArg.cc      =0;
        ncArg.c2      =0;    

        for(;;) {
            ncArg.start = args.start;
            combineFlags=getNextCombining(ncArg,args.limit, options);
            combineBackIndex=ncArg.combiningIndex;
            args.start = ncArg.start;
                    
            if(((combineFlags&COMBINES_BACK)!=0) && starter!=-1) {
                if((combineBackIndex&0x8000)!=0) {
                    // c is a Jamo V/T, see if we can compose it with the 
                    // previous character 
                 
                    remove=-1; // NULL while no Hangul composition 
                    ncArg.c2=args.source[starter];
                    if(combineBackIndex==0xfff2) {
                        // Jamo V, compose with previous Jamo L and following 
                        // Jamo T 
                     
                        ncArg.c2=(char)(ncArg.c2-JAMO_L_BASE);
                        if(ncArg.c2<JAMO_L_COUNT) {
                            remove=args.start-1;
                            ncArg.c=(char)(HANGUL_BASE+(ncArg.c2*JAMO_V_COUNT+
                                           (ncArg.c-JAMO_V_BASE))*JAMO_T_COUNT);
                            if(args.start!=args.limit && 
                                        (ncArg.c2=(char)(args.source[args.start]
                                         -JAMO_T_BASE))<JAMO_T_COUNT) {
                                ++args.start;
                                ncArg.c+=ncArg.c2;
                            }
                            if(!nx_contains(options, ncArg.c)) {
                                args.source[starter]=ncArg.c;
                            } else {
                                // excluded 
                                if(!isHangulWithoutJamoT(ncArg.c)) {
                                    --args.start; // undo the ++args.start from reading the Jamo T 
                                }
                                // c is modified but not used any more -- c=*(p-1); -- re-read the Jamo V/T 
                                remove=args.start;
                            }
                        }

                    }

                    if(remove!=-1) {
                        // remove the Jamo(s) 
                        q=remove;
                        r=args.start;
                        while(r<args.limit) {
                            args.source[q++]=args.source[r++];
                        }
                        args.start=remove;
                        args.limit=q;
                    }

                    ncArg.c2=0; // c2 held *starter temporarily 

                    /*
                     * now: cc==0 and the combining index does not include 
                     * "forward" -> the rest of the loop body will reset starter
                     * to NULL; technically, a composed Hangul syllable is a 
                     * starter, but it does not combine forward now that we have
                     * consumed all eligible Jamos; for Jamo V/T, combineFlags 
                     * does not contain _NORM_COMBINES_FWD
                     */

                } else if(
                    // the starter is not a Jamo V/T and 
                    !((combineFwdIndex&0x8000)!=0) &&
                    // the combining mark is not blocked and 
                    (prevCC<ncArg.cc || prevCC==0) &&
                    // the starter and the combining mark (c, c2) do combine 
                    0!=(result=combine(combiningTable,combineFwdIndex, 
                                       combineBackIndex, outValues)) &&
                    // the composition result is not excluded 
                    !nx_contains(options, (char)value, (char)value2)
                ) {
                    value=outValues[0];
                    value2=outValues[1];
                    /* replace the starter with the composition, remove the 
                     * combining mark 
                     */
                    remove= ncArg.c2==0 ? args.start-1 : args.start-2; // index to the combining mark 

                    // replace the starter with the composition 
                    args.source[starter]=(char)value;
                    if(starterIsSupplementary) {
                        if(value2!=0) {
                            // both are supplementary 
                            args.source[starter+1]=(char)value2;
                        } else {
                            // the composition is shorter than the starter, 
                            // move the intermediate characters forward one 
                            starterIsSupplementary=false;
                            q=starter+1;
                            r=q+1;
                            while(r<remove) {
                                args.source[q++]=args.source[r++];
                            }
                            --remove;
                        }
                    } else if(value2!=0) {
                        // the composition is longer than the starter, 
                        // move the intermediate characters back one 
                        starterIsSupplementary=true;
                        // temporarily increment for the loop boundary 
                        ++starter; 
                        q=remove;
                        r=++remove;
                        while(starter<q) {
                            args.source[--r]=args.source[--q];
                        }
                        args.source[starter]=(char)value2;
                        --starter; // undo the temporary increment 
                    // } else { both are on the BMP, nothing more to do 
                    }

                    // remove the combining mark by moving the following text 
                    // over it 
                    if(remove<args.start) {
                        q=remove;
                        r=args.start;
                        while(r<args.limit) {
                            args.source[q++]=args.source[r++];
                        }
                        args.start=remove;
                        args.limit=q;
                    }

                    // keep prevCC because we removed the combining mark 

                    // done? 
                    if(args.start==args.limit) {
                        return (char)prevCC;
                    }

                    // is the composition a starter that combines forward? 
                    if(result>1) {
                       combineFwdIndex=getCombiningIndexFromStarter((char)value,
                                                                  (char)value2);
                    } else {
                       starter=-1;
                    }

                    // we combined and set prevCC, continue with looking for 
                    // compositions 
                    continue;
                }
            }

            // no combination this time 
            prevCC=ncArg.cc;
            if(args.start==args.limit) {
                return (char)prevCC;
            }

            // if (c, c2) did not combine, then check if it is a starter 
            if(ncArg.cc==0) {
                // found a new starter; combineFlags==0 if (c, c2) is excluded 
                if((combineFlags&COMBINES_FWD)!=0) {
                    // it may combine with something, prepare for it 
                    if(ncArg.c2==0) {
                        starterIsSupplementary=false;
                        starter=args.start-1;
                    } else {
                        starterIsSupplementary=false;
                        starter=args.start-2;
                    }
                    combineFwdIndex=combineBackIndex;
                } else {
                    // it will not combine with anything 
                    starter=-1;
                }
            }
        }
    }

    /**  
     * find the last true starter between src[start]....src[current] going
     * backwards and return its index
     */
    private static int findPreviousStarter(char[]src, int srcStart, int current, 
                                           int ccOrQCMask, int decompQCMask,
                                           char minNoMaybe) { 
        long norm32; 
        PrevArgs args = new PrevArgs();
        args.src = src;
        args.start = srcStart;
        args.current = current;
       
        while(args.start<args.current) { 
            norm32= getPrevNorm32(args, minNoMaybe, ccOrQCMask|decompQCMask); 
            if(isTrueStarter(norm32, ccOrQCMask, decompQCMask)) { 
                break; 
            } 
        } 
        return args.current; 
    }
    
    /** 
     * find the first true starter in [src..limit[ and return the 
     * pointer to it 
     */
    private static int findNextStarter(char[] src,int start,int limit,
                                       int qcMask, int decompQCMask, 
                                       char minNoMaybe) 
    {
        int p;
        long norm32; 
        int ccOrQCMask;
        char c, c2;

        ccOrQCMask=CC_MASK|qcMask;
    
        DecomposeArgs decompArgs = new DecomposeArgs();

        for(;;) {
            if(start==limit) {
                break; // end of string 
            }
            c=src[start];
            if(c<minNoMaybe) {
                break; // catches NUL terminater, too 
            }

            norm32=getNorm32(c);
            if((norm32&ccOrQCMask)==0) {
                break; // true starter 
            }

            if(isNorm32LeadSurrogate(norm32)) {
                // c is a lead surrogate, get the real norm32 
                if((start+1)==limit || 
                                   !Character.isLowSurrogate(c2=(src[start+1]))){
                    // unmatched first surrogate: counts as a true starter                   
                    break; 
                }
                norm32=getNorm32FromSurrogatePair(norm32, c2);

                if((norm32&ccOrQCMask)==0) {
                    break; // true starter 
                }
            } else {
                c2=0;
            }

            // (c, c2) is not a true starter but its decomposition may be 
            if((norm32&decompQCMask)!=0) {
                // (c, c2) decomposes, get everything from the variable-length
                //  extra data 
                p=decompose(norm32, decompQCMask, decompArgs);

                // get the first character's norm32 to check if it is a true 
                // starter 
                if(decompArgs.cc==0 && (getNorm32(extraData,p, qcMask)&qcMask)==0) {
                    break; // true starter 
                }
            }

            start+= c2==0 ? 1 : 2; // not a true starter, continue 
        }

        return start;
    }

    private static final class ComposePartArgs
    {
        int prevCC;
        int length;   // length of decomposed part 
    }
     
    /** 
     * decompose and recompose [prevStarter..src[
     */ 
    private static char[] composePart(ComposePartArgs args, 
                                      int prevStarter, 
                                      char[] src, int start, int limit,
                                      int qcMask,
                                      int options) 
    {
        int recomposeLimit;
        boolean compat =((qcMask&QC_NFKC)!=0);
    
        // decompose [prevStarter..src[ 
        int[] outTrailCC = new int[1];
        char[] buffer = new char[(limit-prevStarter)*MAX_BUFFER_SIZE];

        for(;;){
            args.length=decompose(src, prevStarter, (start),
                                      buffer, 0, buffer.length, 
                                      compat, outTrailCC, options);
            if(args.length<=buffer.length){
                break;
            }else{
                buffer = new char[args.length];
            }
        } 

        // recompose the decomposition 
        recomposeLimit=args.length;
    
        if(args.length>=2) {
            RecomposeArgs rcArgs = new RecomposeArgs();
            rcArgs.source   = buffer;
            rcArgs.start    = 0;
            rcArgs.limit    = recomposeLimit; 
            args.prevCC=recompose(rcArgs, options);
            recomposeLimit = rcArgs.limit;
        }
    
        // return with a pointer to the recomposition and its length 
        args.length=recomposeLimit;
        return buffer;
    }

    private static boolean composeHangul(char prev, char c,
                                         long norm32, 
                                         char[] src,int[] srcIndex, int limit,
                                         boolean compat, 
                                         char[] dest,int destIndex,
                                         int options) 
    {
        int start=srcIndex[0];
        if(isJamoVTNorm32JamoV(norm32)) {
            // c is a Jamo V, compose with previous Jamo L and 
            // following Jamo T 
            prev=(char)(prev-JAMO_L_BASE);
            if(prev<JAMO_L_COUNT) {
                c=(char)(HANGUL_BASE+(prev*JAMO_V_COUNT+
                                                 (c-JAMO_V_BASE))*JAMO_T_COUNT);

                // check if the next character is a Jamo T (normal or 
                // compatibility) 
                if(start!=limit) {
                    char next, t;

                    next=src[start];
                    if((t=(char)(next-JAMO_T_BASE))<JAMO_T_COUNT) {
                        // normal Jamo T 
                        ++start;
                        c+=t;
                    } else if(compat) {
                        // if NFKC, then check for compatibility Jamo T 
                        // (BMP only) 
                        norm32=getNorm32(next);
                        if(isNorm32Regular(norm32) && ((norm32&QC_NFKD)!=0)) {
                            int p; // index into extra data array
                            DecomposeArgs dcArgs = new DecomposeArgs();
                            p=decompose(norm32, QC_NFKD, dcArgs);
                            if(dcArgs.length==1 && 
                                   (t=(char)(extraData[p]-JAMO_T_BASE))
                                                   <JAMO_T_COUNT) {
                                // compatibility Jamo T 
                                ++start;
                                c+=t;
                            }
                        }
                    }
                }
                if(nx_contains(options, c)) {
                    if(!isHangulWithoutJamoT(c)) {
                        --start; // undo ++start from reading the Jamo T 
                    }
                    return false;
                }
                dest[destIndex]=c;
                srcIndex[0]=start;
                return true;
            }
        } else if(isHangulWithoutJamoT(prev)) {
            // c is a Jamo T, compose with previous Hangul LV that does not 
            // contain a Jamo T 
            c=(char)(prev+(c-JAMO_T_BASE));
            if(nx_contains(options, c)) {
                return false;
            }
            dest[destIndex]=c;
            srcIndex[0]=start;
            return true;
        }
        return false;
    }
    
    /// problem: public method access changed to package private
    static int compose(char[] src, int srcIndex, int srcLimit,
                       char[] dest,int destIndex,int destLimit,
                       boolean compat, int options) 
    {
        int prevSrc, prevStarter;
        long norm32; 
        int ccOrQCMask, qcMask;
        int  reorderStartIndex, length;
        char c, c2, minNoMaybe;
        int cc, prevCC;
        int[] ioIndex = new int[1];
    
        if(!compat) {
            minNoMaybe=(char)indexes[INDEX_MIN_NFC_NO_MAYBE];
            qcMask=QC_NFC;
        } else {
            minNoMaybe=(char)indexes[INDEX_MIN_NFKC_NO_MAYBE];
            qcMask=QC_NFKC;
        }

        /*
         * prevStarter points to the last character before the current one
         * that is a "true" starter with cc==0 and quick check "yes".
         *
         * prevStarter will be used instead of looking for a true starter
         * while incrementally decomposing [prevStarter..prevSrc[
         * in _composePart(). Having a good prevStarter allows to just decompose
         * the entire [prevStarter..prevSrc[.
         *
         * When _composePart() backs out from prevSrc back to prevStarter,
         * then it also backs out destIndex by the same amount.
         * Therefore, at all times, the (prevSrc-prevStarter) source units
         * must correspond 1:1 to destination units counted with destIndex,
         * except for reordering.
         * This is true for the qc "yes" characters copied in the fast loop,
         * and for pure reordering.
         * prevStarter must be set forward to src when this is not true:
         * In _composePart() and after composing a Hangul syllable.
         *
         * This mechanism relies on the assumption that the decomposition of a 
         * true starter also begins with a true starter. gennorm/store.c checks 
         * for this.
         */
     
        prevStarter=srcIndex;

        ccOrQCMask=CC_MASK|qcMask;
        // destIndex=
        reorderStartIndex=0; // ####TODO#### check this *
        prevCC=0;

        // avoid compiler warnings 
        norm32=0;
        c=0;

        for(;;) {
            // count code units below the minimum or with irrelevant data for 
            // the quick check 
            prevSrc=srcIndex;

            while(srcIndex!=srcLimit && ((c=src[srcIndex])<minNoMaybe || 
                     ((norm32=getNorm32(c))&ccOrQCMask)==0)) {
                prevCC=0;
                ++srcIndex;
            }


            // copy these code units all at once 
            if(srcIndex!=prevSrc) {
                length=(int)(srcIndex-prevSrc);
                if((destIndex+length)<=destLimit) {
                    System.arraycopy(src,prevSrc,dest,destIndex,length);
                }
                destIndex+=length;
                reorderStartIndex=destIndex;

                // set prevStarter to the last character in the quick check 
                // loop 
                prevStarter=srcIndex-1;
                if(Character.isLowSurrogate(src[prevStarter]) && 
                    prevSrc<prevStarter && 
                    Character.isHighSurrogate(src[(prevStarter-1)])) {
                    --prevStarter;
                }

                prevSrc=srcIndex;
            }

            // end of source reached? 
            if(srcIndex==srcLimit) {
                break;
            }

            // c already contains *src and norm32 is set for it, increment src
            ++srcIndex;

            /*
             * source buffer pointers:
             *
             *  all done      quick check   current char  not yet
             *                "yes" but     (c, c2)       processed
             *                may combine
             *                forward
             * [-------------[-------------[-------------[-------------[
             * |             |             |             |             |
             * start         prevStarter   prevSrc       src           limit
             *
             *
             * destination buffer pointers and indexes:
             *
             *  all done      might take    not filled yet
             *                characters for
             *                reordering
             * [-------------[-------------[-------------[
             * |             |             |             |
             * dest      reorderStartIndex destIndex     destCapacity
             */

            // check one above-minimum, relevant code unit 
            /*
             * norm32 is for c=*(src-1), and the quick check flag is "no" or 
             * "maybe", and/or cc!=0
             * check for Jamo V/T, then for surrogates and regular characters
             * c is not a Hangul syllable or Jamo L because
             * they are not marked with no/maybe for NFC & NFKC(and their cc==0)
             */
         
            if(isNorm32HangulOrJamo(norm32)) {
                /*
                 * c is a Jamo V/T:
                 * try to compose with the previous character, Jamo V also with 
                 * a following Jamo T, and set values here right now in case we 
                 * just continue with the main loop
                 */
             
                prevCC=cc=0;
                reorderStartIndex=destIndex;
                ioIndex[0]=srcIndex;
                if( 
                    destIndex>0 &&
                    composeHangul(src[(prevSrc-1)], c, norm32,src, ioIndex,
                                  srcLimit, compat, dest,
                                  destIndex<=destLimit ? destIndex-1: 0,
                                  options)
                ) {
                    srcIndex=ioIndex[0];
                    prevStarter=srcIndex;
                    continue;
                }
            
                srcIndex = ioIndex[0];

                /* the Jamo V/T did not compose into a Hangul syllable, just 
                 * append to dest 
                 */
                c2=0;
                length=1;
                prevStarter=prevSrc;
            } else {
                if(isNorm32Regular(norm32)) {
                    c2=0;
                    length=1;
                } else {
                    // c is a lead surrogate, get the real norm32 
                    if(srcIndex!=srcLimit &&
                                     Character.isLowSurrogate(c2=src[srcIndex])) {
                        ++srcIndex;
                        length=2;
                        norm32=getNorm32FromSurrogatePair(norm32, c2);
                    } else {
                        // c is an unpaired lead surrogate, nothing to do 
                        c2=0;
                        length=1;
                        norm32=0;
                    }
                }
                ComposePartArgs args =new ComposePartArgs();
            
                // we are looking at the character (c, c2) at [prevSrc..src[ 
                if(nx_contains(options, c, c2)) {
                    // excluded: norm32==0 
                    cc=0;
                } else if((norm32&qcMask)==0) {
                    cc=(int)((UNSIGNED_BYTE_MASK)&(norm32>>CC_SHIFT));
                } else {
                    char[] p;

                    /*
                     * find appropriate boundaries around this character,
                     * decompose the source text from between the boundaries,
                     * and recompose it
                     *
                     * this puts the intermediate text into the side buffer because
                     * it might be longer than the recomposition end result,
                     * or the destination buffer may be too short or missing
                     *
                     * note that destIndex may be adjusted backwards to account
                     * for source text that passed the quick check but needed to
                     * take part in the recomposition
                     */
                 
                    int decompQCMask=(qcMask<<2)&0xf; /* decomposition quick check mask 
                    /*
                     * find the last true starter in [prevStarter..src[
                     * it is either the decomposition of the current character (at prevSrc),
                     * or prevStarter
                     */
                 
                    if(isTrueStarter(norm32, CC_MASK|qcMask, decompQCMask)) {
                        prevStarter=prevSrc;
                    } else {
                        // adjust destIndex: back out what had been copied with qc "yes" 
                        destIndex-=prevSrc-prevStarter;
                    }
            
                    // find the next true starter in [src..limit[ 
                    srcIndex=findNextStarter(src, srcIndex,srcLimit, qcMask, 
                                               decompQCMask, minNoMaybe);
                    // args.prevStarter = prevStarter;
                    args.prevCC = prevCC;                    
                    // args.destIndex = destIndex;
                    args.length = length;
                    p=composePart(args,prevStarter,src,srcIndex,srcLimit,qcMask, options);
                    
                    if(p==null) {
                        // an error occurred (out of memory) 
                        break;
                    }
                
                    prevCC      = args.prevCC;
                    length      = args.length;
                
                    /* append the recomposed buffer contents to the destination 
                     * buffer 
                     */
                    if((destIndex+args.length)<=destLimit) {
                        int i=0;
                        while(i<args.length) {
                            dest[destIndex++]=p[i++];
                            --length;
                        }
                    } else {
                        // buffer overflow 
                        // keep incrementing the destIndex for preflighting 
                        destIndex+=length;
                    }

                    prevStarter=srcIndex;
                    continue;
                }
            }

            // append the single code point (c, c2) to the destination buffer 
            if((destIndex+length)<=destLimit) {
                if(cc!=0 && cc<prevCC) {
                    // (c, c2) is out of order with respect to the preceding 
                    // text 
                    int reorderSplit= destIndex;
                    destIndex+=length;
                    prevCC=insertOrdered(dest,reorderStartIndex, reorderSplit, 
                                         destIndex, c, c2, cc);
                } else {
                    // just append (c, c2) 
                    dest[destIndex++]=c;
                    if(c2!=0) {
                        dest[destIndex++]=c2;
                    }
                    prevCC=cc;
                }
            } else {
                // buffer overflow 
                // keep incrementing the destIndex for preflighting 
                destIndex+=length;
                prevCC=cc;
            }
        }

        return destIndex;
    }
    
    public static int getCombiningClass(int c) {
        long norm32;
        norm32=getNorm32(c);
        return (char)((norm32>>CC_SHIFT)&0xFF);
    }
    
    
    /**
     * Get the canonical decomposition 
     * sherman  for ComposedCharIter
     */

    static int getDecompose(int chars[], String decomps[]) {
        DecomposeArgs args = new DecomposeArgs();
        int length=0;
        long norm32 = 0;
        int ch = -1;
        int index = 0;
        int i = 0;

        while (++ch < 0x2fa1e) {   //no cannoical above 0x3ffff
	    //TBD !!!! the hack code heres save us about 50ms for startup
	    //need a better solution/lookup
            if (ch == 0x30ff) 
                ch = 0xf900;
            else if (ch == 0x10000) 
                ch = 0x1d15e;
            else if (ch == 0x1d1c1) 
		ch = 0x2f800;

            norm32 = NormalizerImpl.getNorm32(ch);
            if((norm32 & QC_NFD)!=0 && i < chars.length) {
                chars[i] = ch;
                index = decompose(norm32, args);
                decomps[i++] = new String(extraData,index, args.length);
            }
	}
        return i;
    }

    private static int strCompare(char[] s1, int s1Start, int s1Limit,
                                  char[] s2, int s2Start, int s2Limit,
                                  boolean codePointOrder) {
                        
        int start1, start2, limit1, limit2;
 
        char c1, c2;
    
        /* setup for fix-up */
        start1=s1Start;
        start2=s2Start;
        
        int length1, length2;
        
        length1 = s1Limit - s1Start;
        length2 = s2Limit - s2Start;
            
        int lengthResult;

        if(length1<length2) {
            lengthResult=-1;
            limit1=start1+length1;
        } else if(length1==length2) {
            lengthResult=0;
            limit1=start1+length1;
        } else /* length1>length2 */ {
            lengthResult=1;
            limit1=start1+length2;
        }

        if(s1==s2) {
            return lengthResult;
        }

        for(;;) {
            /* check pseudo-limit */
            if(s1Start==limit1) {
                return lengthResult;
            }

            c1=s1[s1Start];
            c2=s2[s2Start];
            if(c1!=c2) {
                break;
            }
            ++s1Start;
            ++s2Start;
        }

        /* setup for fix-up */
        limit1=start1+length1;
        limit2=start2+length2;

    
        /* if both values are in or above the surrogate range, fix them up */
        if(c1>=0xd800 && c2>=0xd800 && codePointOrder) {
            /* subtract 0x2800 from BMP code points to make them smaller than
             *  supplementary ones */
            if(
                ( c1<=0xdbff && (s1Start+1)!=limit1 && 
                  Character.isLowSurrogate(s1[(s1Start+1)])
                ) ||
                ( Character.isLowSurrogate(c1) && start1!=s1Start && 
                  Character.isHighSurrogate(s1[(s1Start-1)])
                )
            ) {
                /* part of a surrogate pair, leave >=d800 */
            } else {
                /* BMP code point - may be surrogate code point - make <d800 */
                c1-=0x2800;
            }
    
            if(
                ( c2<=0xdbff && (s2Start+1)!=limit2 && 
                  Character.isLowSurrogate(s2[(s2Start+1)])
                ) ||
                ( Character.isLowSurrogate(c2) && start2!=s2Start && 
                  Character.isHighSurrogate(s2[(s2Start-1)])
                )
            ) {
                /* part of a surrogate pair, leave >=d800 */
            } else {
                /* BMP code point - may be surrogate code point - make <d800 */
                c2-=0x2800;
            }
        }
    
        /* now c1 and c2 are in UTF-32-compatible order */
        return (int)c1-(int)c2;
    }
   
    // normalization exclusion sets ------------------------------------------
    private static final boolean nx_contains(int options, int c) 
    {
        if (options != Normalizer.IGNORE_HANGUL)
            return false;
         return  c >= 0xac00 && c <=0xd7a3;
    }

    private static final boolean nx_contains(int options, char c, char c2) 
    {
        if (options != Normalizer.IGNORE_HANGUL)
            return false;
         return  c2 == 0 && c >= 0xac00 && c <= 0xd7a3;

    }

    /* The data below is the precomposed characters that added in 4.0,
       we might use them if we need to support Unicode 3.2 Normalization
    private static final synchronized UnicodeSet internalGetNXUnicode32()
    {
        return new UnicodeSet("[[\u0221][\u0234-\u0236][\u02AE-\u02AF]"
                        + "[\u02EF-\u02FF][\u0350-\u0357][\u035D-\u035F]"
                        + "[\u03F7-\u03FB][\u0600-\u0603][\u060D-\u0615]"
                        + "[\u0656-\u0658][\u06EE-\u06EF][\u06FF]"
                        + "[\u072D-\u072F][\u074D-\u074F][\u0904]"
                        + "[\u09BD][\u0A01][\u0A03][\u0A8C][\u0AE1-\u0AE3]"
                        + "[\u0AF1][\u0B35][\u0B71][\u0BF3-\u0BFA]"
                        + "[\u0CBC-\u0CBD][\u17DD][\u17F0-\u17F9]"
                        + "[\u1900-\u191C][\u1920-\u192B][\u1930-\u193B]"
                        + "[\u1940][\u1944-\u196D][\u1970-\u1974]"
                        + "[\u19E0-\u19FF][\u1D00-\u1D6B][\u2053-\u2054]"
                        + "[\u213B][\u23CF-\u23D0][\u24FF][\u2614-\u2615]"
                        + "[\u268A-\u2691][\u26A0-\u26A1][\u2B00-\u2B0D]"
                        + "[\u321D-\u321E][\u3250][\u327C-\u327D]"
                        + "[\u32CC-\u32CF][\u3377-\u337A][\u33DE-\u33DF]"
                        + "[\u33FF][\u4DC0-\u4DFF][\uFDFD][\uFE47-\uFE48]"
                        + "[\uD800\uDC00-\uD800\uDC0B]" 
                        + "[\uD800\uDC0D-\uD800\uDC26]" 
                        + "[\uD800\uDC28-\uD800\uDC3A]"
                        + "[\uD800\uDC3C-\uD800\uDC3D]" 
                        + "[\uD800\uDC3F-\uD800\uDC4D]" 
                        + "[\uD800\uDC50-\uD800\uDC5D]"
                        + "[\uD800\uDC80-\uD800\uDCFA]" 
                        + "[\uD800\uDD00-\uD800\uDD02]" 
                        + "[\uD800\uDD07-\uD800\uDD33]"
                        + "[\uD800\uDD37-\uD800\uDD3F]" 
                        + "[\uD800\uDF80-\uD800\uDF9D][\uD800\uDF9F]"
                        + "[\uD801\uDC26-\uD801\uDC27]" 
                        + "[\uD801\uDC4E-\uD801\uDC9D]" 
                        + "[\uD801\uDCA0-\uD801\uDCA9]"
                        + "[\uD802\uDC00-\uD802\uDC05][\uD802\uDC08]" 
                        + "[\uD802\uDC0A-\uD802\uDC35]"
                        + "[\uD802\uDC37-\uD802\uDC38][\uD802\uDC3C]" 
                        + "[\uD802\uDC3F][\uD834\uDF00-\uD834\uDF56]"
                        + "[\uD835\uDCC1][\uDB40\uDD00-\uDB40\uDDEF]]");
    }
    */
}
