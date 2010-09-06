/*
 * @(#)Normalizer.java	1.14 03/12/19
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

import java.text.CharacterIterator;

/**
 * <h2>Unicode normalization API</h2>
 *
 * <p>
 * <code>Normalizer</code> transforms Unicode text into an equivalent 
 * composed or decomposed form, allowing for easier sorting and searching of 
 * text. <code>Normalizer</code> supports the standard normalization forms 
 * described in
 * <a href="http://www.unicode.org/unicode/reports/tr15/" target="unicode">
 * Unicode Standard Annex #15 &mdash; Unicode Normalization Forms</a>.
 * </p>
 * <p>
 * Characters with accents or other adornments can be encoded in
 * several different ways in Unicode.  For example, take the character A-acute.
 * In Unicode, this can be encoded as a single character (the
 * "composed" form):
 * </p>
 * <p>
 *      00C1    LATIN CAPITAL LETTER A WITH ACUTE
 * </p>
 * <p>
 * or as two separate characters (the "decomposed" form):
 * </p>
 * <p>
 *      0041    LATIN CAPITAL LETTER A
 *      0301    COMBINING ACUTE ACCENT
 * </p>
 * <p>
 * To a user of your program, however, both of these sequences should be
 * treated as the same "user-level" character "A with acute accent".  When you 
 * are searching or comparing text, you must ensure that these two sequences 
 * are treated equivalently. In addition, you must handle characters with more 
 * than one accent. Sometimes the order of a character's combining accents is
 * significant, while in other cases accent sequences in different orders are
 * really equivalent.
 * </p>
 * <p>
 * Similarly, the string "ffi" can be encoded as three separate letters:
 * </p>
 * <p>
 *      0066    LATIN SMALL LETTER F
 *      0066    LATIN SMALL LETTER F
 *      0069    LATIN SMALL LETTER I
 * <\p>
 * <p>
 * or as the single character
 * </p>
 * <p>
 *      FB03    LATIN SMALL LIGATURE FFI
 * <\p>
 * <p>
 * The ffi ligature is not a distinct semantic character, and strictly speaking
 * it shouldn't be in Unicode at all, but it was included for compatibility
 * with existing character sets that already provided it.  The Unicode standard
 * identifies such characters by giving them "compatibility" decompositions
 * into the corresponding semantic characters.  When sorting and searching, you
 * will often want to use these mappings.
 * </p>
 * <p>
 * <code>normalize</code> helps solve these problems by transforming text into 
 * the canonical composed and decomposed forms as shown in the first example 
 * above. In addition, you can have it perform compatibility decompositions so 
 * that you can treat compatibility characters the same as their equivalents.
 * Finally, <code>normalize</code> rearranges accents into the proper canonical
 * order, so that you do not have to worry about accent rearrangement on your
 * own.
 * </p>
 * <p>
 * Form FCD, "Fast C or D", is also designed for collation.
 * It allows to work on strings that are not necessarily normalized
 * with an algorithm (like in collation) that works under "canonical closure", 
 * i.e., it treats precomposed characters and their decomposed equivalents the 
 * same.
 * </p>
 * <p>
 * It is not a normalization form because it does not provide for uniqueness of 
 * representation. Multiple strings may be canonically equivalent (their NFDs 
 * are identical) and may all conform to FCD without being identical themselves.
 * </p>
 * <p>
 * The form is defined such that the "raw decomposition", the recursive 
 * canonical decomposition of each character, results in a string that is 
 * canonically ordered. This means that precomposed characters are allowed for 
 * as long as their decompositions do not need canonical reordering.
 * </p>
 * <p>
 * Its advantage for a process like collation is that all NFD and most NFC texts
 * - and many unnormalized texts - already conform to FCD and do not need to be 
 * normalized (NFD) for such a process. The FCD quick check will return YES for 
 * most strings in practice.
 * </p>
 * <p>
 * <code>normalize(FCD)</code> may be implemented with NFD.
 * </p>
 * <p>
 * For more details on FCD see the collation design document:
 * http://oss.software.ibm.com/cvs/icu/~checkout~/icuhtml/design/collation/ICU_collation_design.htm
 * </p>
 * <p>
 * ICU collation performs either NFD or FCD normalization automatically if 
 * normalization is turned on for the collator object. Beyond collation and 
 * string search, normalized strings may be useful for string equivalence 
 * comparisons, transliteration/transcription, unique representations, etc.
 * </p>
 * <p>
 * The W3C generally recommends to exchange texts in NFC.
 * Note also that most legacy character encodings use only precomposed forms and
 * often do not encode any combining marks by themselves. For conversion to such
 * character encodings the Unicode text needs to be normalized to NFC.
 * For more usage examples, see the Unicode Standard Annex.
 * @draft ICU 2.2
 
 */

public final class Normalizer implements Cloneable
{    
    //-------------------------------------------------------------------------
    // Private data
    //-------------------------------------------------------------------------  
    private char[] buffer = new char[100];
    private int bufferStart = 0;
    private int bufferPos   = 0;
    private int bufferLimit = 0;
    
    // The input text and our position in it
    private UCharacterIterator  text;
    private Mode                mode = COMPOSE;
    private int                 options = 0;
    private int                 currentIndex;
    private int                 nextIndex;
    
    /**
     * Options bit set value to select Unicode 3.2 normalization
     * (except NormalizationCorrections).
     * At most one Unicode version can be selected at a time.
     */
    //public static final int UNICODE_3_2=0x02;

    /**
     * Option to disable Hangul/Jamo composition and decomposition.
     * This option applies to Korean text,
     * which can be represented either in the Jamo alphabet or in Hangul
     * characters, which are really just two or three Jamo combined
     * into one visual glyph.  Since Jamo takes up more storage space than
     * Hangul, applications that process only Hangul text may wish to turn
     * this option on when decomposing text.
     * <p>
     * The Unicode standard treates Hangul to Jamo conversion as a
     * canonical decomposition, so this option must be turned <b>off</b> if you
     * wish to transform strings into one of the standard
     * <a href="http://www.unicode.org/unicode/reports/tr15/" target="unicode">
     * Unicode Normalization Forms</a>.
     * <p>
     * @see #setOption
     */
    public static final int IGNORE_HANGUL = 0x0001;

    /**
     * Constant indicating that the end of the iteration has been reached.
     * This is guaranteed to have the same value as {@link UCharacterIterator#DONE}.
     */
    public static final int DONE = UCharacterIterator.DONE;

    /**
     * Constants for normalization modes.
     */
    public static class Mode 
    {
        private int modeValue;
        private Mode(int value){
            modeValue = value;
        }
        /**
         * This method is used for method dispatch
         */
        protected int normalize(char[] src, int srcStart, int srcLimit,
                                char[] dest,int destStart,int destLimit, 
                                int options){
            int srcLen = (srcLimit - srcStart);
            int destLen = (destLimit - destStart);
            if( srcLen > destLen ){
                return srcLen;
            }
            System.arraycopy(src,srcStart,dest,destStart,srcLen);
            return srcLen;
        }

        /**
         * This method is used for method dispatch
         */
        protected String normalize(String src, int options){
            return src;
        }
        /**
         * This method is used for method dispatch
         */
        protected int getMinC(){
            return -1;
        }
        /**
         * This method is used for method dispatch
         */
        protected int getMask(){
            return -1;
        }
        /**
         * This method is used for method dispatch
         */
        protected IsPrevBoundary getPrevBoundary(){
            return null;
        }
        /**
         * This method is used for method dispatch
         */
        protected IsNextBoundary getNextBoundary(){
            return null;
        }
        /**
         * This method is used for method dispatch
         */
        protected QuickCheckResult quickCheck(char[] src,int start, int limit, 
                                              boolean allowMaybe, int options){
            if(allowMaybe){
                return MAYBE;
            }
            return NO;
        }
    };
    
    /** 
     * No decomposition/composition.  
     */
    /// problem: values changed from 0 to 1
    public static final Mode NO_OP = new Mode(1);

    /** 
     * Canonical decomposition.  
     */
    public static final Mode DECOMP = new NFDMode(2);
    
    private static final class NFDMode extends Mode{
        private NFDMode(int value){
            super(value);
        }
        protected int normalize(char[] src, int srcStart, int srcLimit,
                                char[] dest,int destStart,int destLimit, 
                                int options){
          int[] trailCC = new int[1];
          return NormalizerImpl.decompose(src,  srcStart,srcLimit,
                                          dest, destStart,destLimit,
                                          false, trailCC, options);
        }
    
        protected String normalize( String src, int options){
            return decompose(src, false, options);
        }
        protected int getMinC(){
            return NormalizerImpl.MIN_WITH_LEAD_CC;
        }
        protected IsPrevBoundary getPrevBoundary(){
            return new IsPrevNFDSafe();
        }
        protected IsNextBoundary getNextBoundary(){
            return new IsNextNFDSafe();
        }
        protected int getMask(){
            return (NormalizerImpl.CC_MASK|NormalizerImpl.QC_NFD);
        }
        protected QuickCheckResult quickCheck(char[] src, int start, 
                                              int limit, boolean allowMaybe,
                                              int options){
            return NormalizerImpl.quickCheck(
                                  src, start, limit,
                                  NormalizerImpl.getFromIndexesArr(
                                       NormalizerImpl.INDEX_MIN_NFD_NO_MAYBE
                                  ),
                                  NormalizerImpl.QC_NFD,
                                  allowMaybe,
                                  options);
        }
    };
    
    /** 
     * Compatibility decomposition.  
     */
    public static final Mode DECOMP_COMPAT = new NFKDMode(3);

    private static final class NFKDMode extends Mode
    {
        private NFKDMode(int value){
            super(value);
        }
        protected int normalize(char[] src, int srcStart, int srcLimit,
                                char[] dest,int destStart,int destLimit, 
                                int options){
          int[] trailCC = new int[1];
          return NormalizerImpl.decompose(src,  srcStart,srcLimit,
                           dest, destStart,destLimit,
                           true, trailCC, options);
        }
        protected String normalize( String src, int options){
            return decompose(src,true,options);
        }
        protected int getMinC(){
            return NormalizerImpl.MIN_WITH_LEAD_CC;
        }
        protected IsPrevBoundary getPrevBoundary(){
            return new IsPrevNFDSafe();
        }
        protected IsNextBoundary getNextBoundary(){
            return new IsNextNFDSafe();
        }
        protected int getMask(){
            return (NormalizerImpl.CC_MASK|NormalizerImpl.QC_NFKD);
        }
        protected QuickCheckResult quickCheck(char[] src,int start, 
                                              int limit,boolean allowMaybe,
                                              int options){
            return NormalizerImpl.quickCheck(
                                  src,start,limit,
                                  NormalizerImpl.getFromIndexesArr(
                                      NormalizerImpl.INDEX_MIN_NFKD_NO_MAYBE
                                  ),
                                  NormalizerImpl.QC_NFKD,
                                  allowMaybe,
                                  options);
        }                                     
    };

    /** 
     * Canonical decomposition followed by canonical composition.  
     */
    public static final Mode COMPOSE = new NFCMode(4);
    
    private static final class NFCMode extends Mode{
        private NFCMode(int value){
            super(value);
        }
        protected int normalize(char[] src, int srcStart, int srcLimit,
                                char[] dest,int destStart,int destLimit,
                                int options){
            return NormalizerImpl.compose( src, srcStart, srcLimit,
                                           dest,destStart,destLimit,
                                           false, options);
        }
  
        protected String normalize( String src, int options){
            return compose(src,false, options);
        }
       
        protected int getMinC(){
            return NormalizerImpl.getFromIndexesArr(
                                    NormalizerImpl.INDEX_MIN_NFC_NO_MAYBE);
        }
        protected IsPrevBoundary getPrevBoundary(){
            return new IsPrevTrueStarter();
        }
        protected IsNextBoundary getNextBoundary(){
            return new IsNextTrueStarter();
        }
        protected int getMask(){
            return (NormalizerImpl.CC_MASK|NormalizerImpl.QC_NFC);
        }
        protected QuickCheckResult quickCheck(char[] src,int start, 
                                              int limit,boolean allowMaybe,
                                              int options){
            return NormalizerImpl.quickCheck(
                                   src,start,limit,
                                   NormalizerImpl.getFromIndexesArr(
                                         NormalizerImpl.INDEX_MIN_NFC_NO_MAYBE
                                   ),
                                   NormalizerImpl.QC_NFC,
                                   allowMaybe,
                                   options);
        }       
    };
    
    /** 
     * Compatibility decomposition followed by canonical composition. 
     */
    public static final Mode COMPOSE_COMPAT =new NFKCMode(5);

    private static final class NFKCMode extends Mode{
        private NFKCMode(int value){
            super(value);
        }
        protected int normalize(char[] src, int srcStart, int srcLimit,
                                char[] dest,int destStart,int destLimit, 
                                int options){
          return NormalizerImpl.compose(src,  srcStart,srcLimit,
                         dest, destStart,destLimit,
                         true, options);
        }

        protected String normalize( String src, int options){
            return compose(src,true, options);
        }
        protected int getMinC(){
            return NormalizerImpl.getFromIndexesArr(
                                    NormalizerImpl.INDEX_MIN_NFKC_NO_MAYBE);
        }
        protected IsPrevBoundary getPrevBoundary(){
            return new IsPrevTrueStarter();
        }
        protected IsNextBoundary getNextBoundary(){
            return new IsNextTrueStarter();
        }
        protected int getMask(){
            return (NormalizerImpl.CC_MASK|NormalizerImpl.QC_NFKC);
        }
        protected QuickCheckResult quickCheck(char[] src,int start, 
                                              int limit,boolean allowMaybe,
                                              int options){
            return NormalizerImpl.quickCheck(
                                   src,start,limit,
                                   NormalizerImpl.getFromIndexesArr(
                                      NormalizerImpl.INDEX_MIN_NFKC_NO_MAYBE
                                   ),
                                   NormalizerImpl.QC_NFKC,
                                   allowMaybe,
                                   options);
        }
    };
                                         
    /** 
     * "Fast C or D" form. 
     */
    public static final Mode FCD = new FCDMode(6);

    private static final class FCDMode extends Mode{
        private FCDMode(int value){
            super(value);
        }
        protected QuickCheckResult quickCheck(char[] src,int start, 
                                              int limit,boolean allowMaybe,
                                              int options){
            return NormalizerImpl.checkFCD(src, start, limit, options)?YES:NO;
        }
    };

      
    /**
     * Result values for quickCheck().
     * For details see Unicode Technical Report 15.
     */
    public static final class QuickCheckResult{
        private int resultValue;
        private QuickCheckResult(int value){
            resultValue=value;
        }
    }
    
    /** 
     * Indicates that string is not in the normalized format
     */
    public static final QuickCheckResult NO = new QuickCheckResult(0);

    /** 
     * Indicates that string is in the normalized format
     */
    public static final QuickCheckResult YES = new QuickCheckResult(1);

    /** 
     * Indicates it cannot be determined if string is in the normalized 
     * format without further thorough checks.
     */
    public static final QuickCheckResult MAYBE = new QuickCheckResult(2);
    
    //-------------------------------------------------------------------------
    // Constructors
    //-------------------------------------------------------------------------

    /**
     * Creates a new <tt>Normalizer</tt> object for iterating over the
     * normalized form of a given string.
     * <p>
     * The <tt>options</tt> parameter specifies which optional
     * <tt>Normalizer</tt> features are to be enabled for this object.
     * <p> 
     * @param str  The string to be normalized.  The normalization
     *              will start at the beginning of the string.
     *
     * @param mode The normalization mode.
     * 
     * @param opt Any optional features to be enabled.
     *            Currently the only available option is {@link #UNICODE_3_2}.
     *            If you want the default behavior corresponding to one of the
     *            standard Unicode Normalization Forms, use 0 for this argument.
     */
    public Normalizer(String str, Mode mode, int opt) {
        this.text = UCharacterIterator.getInstance(str);
        this.mode = mode; 
        this.options=opt;
    }
    
    /**
     * Creates a new <tt>Normalizer</tt> object for iterating over the
     * normalized form of the given text.
     * <p>
     * @param iter  The input text to be normalized.  The normalization
     *              will start at the beginning of the string.
     *
     * @param mode  The normalization mode.
     *
     * @param opt Any optional features to be enabled.
     *            Currently the only available option is {@link #UNICODE_3_2}.
     *            If you want the default behavior corresponding to one of the
     *            standard Unicode Normalization Forms, use 0 for this argument.
     */
    /// problem: do we need opt anymore?
    public Normalizer(CharacterIterator iter, Mode mode, int opt){
        this.text = UCharacterIterator.getInstance(
                                        (CharacterIterator)iter.clone()
                                    );
        this.mode = mode;
        this.options = opt;
    } 

    /**
     * Creates a new <tt>Normalizer</tt> object for iterating over the
     * normalized form of the given text.
     * <p>
     * @param iter  The input text to be normalized.  The normalization
     *              will start at the beginning of the string.
     *
     * @param mode  The normalization mode.
     * @param options The normalization options, ORed together (0 for no options).
     */
    /// problem: do we need opt anymore?
    ///          icu4j code clones the iter for use, that depends on whether we
    ///          want to release this API as public
    private Normalizer(UCharacterIterator iter, Mode mode, int options){
        try{
            this.text     = (UCharacterIterator)iter.clone();
            this.mode     = mode;
            this.options  = options;
        }catch (CloneNotSupportedException e) {
            throw new InternalError(e.toString());
        }    
    }    
    
    /**
     * <p>
     * Creates a new <code>Normalizer</code> object for iterating over the
     * normalized form of a given string.
     * </p>
     * @param str The string to be normalized. The normalization will start 
     *            at the beginning of the string.
     * @param mode The normalization mode.
     */ 
    public Normalizer(String str, Mode mode) 
    {
          this(str, mode, 0);
    }
    
    /**
     * Creates a new <tt>Normalizer</tt> object for iterating over the
     * normalized form of the given text.
     * <p>
     * @param iter  The input text to be normalized.  The normalization
     *              will start at the beginning of the string.
     *
     * @param mode  The normalization mode.
     *
     * @param opt Any optional features to be enabled.
     *            Currently the only available option is {@link #UNICODE_3_2}.
     *            If you want the default behavior corresponding to one of the
     *            standard Unicode Normalization Forms, use 0 for this argument.
     */
    public Normalizer(CharacterIterator iter, Mode mode){
        this(iter, mode, 0);
    }
    
    /**
     * Clones this <tt>Normalizer</tt> object.  All properties of this
     * object are duplicated in the new object, including the cloning of any
     * {@link CharacterIterator} that was passed in to the constructor
     * or to {@link #setText(CharacterIterator) setText}.
     * However, the text storage underlying
     * the <tt>CharacterIterator</tt> is not duplicated unless the
     * iterator's <tt>clone</tt> method does so.
     * @draft ICU 2.2
     */
    public Object clone() {
        try {
            Normalizer copy = (Normalizer) super.clone();
            copy.text = (UCharacterIterator) text.clone();
            //clone the internal buffer
            if (buffer != null) {
                copy.buffer = new char[buffer.length];
                System.arraycopy(buffer,0,copy.buffer,0,buffer.length);
            }
            return copy;
        }
        catch (CloneNotSupportedException e) {
            throw new InternalError(e.toString());
        }
    }
    
    //--------------------------------------------------------------------------
    // Static Utility methods
    //--------------------------------------------------------------------------
    
    /**
     * Compose a string.
     * The string will be composed to according the the specified mode.
     * @param str        The string to compose.
     * @param compat     If true the string will be composed accoding to 
     *                    NFKC rules and if false will be composed according to 
     *                    NFC rules.
     * @param options    The only recognized option is UNICODE_3_2
     * @return String    The composed string   
     */            
    public static String compose(String str, boolean compat, int options){
           
        char[] dest = new char[str.length()*MAX_BUF_SIZE_COMPOSE];
        int destSize=0;
        char[] src = str.toCharArray();
        for(;;){
            destSize=NormalizerImpl.compose(src, 0, src.length,
                                            dest,0, dest.length, compat,
                                            options);
            if(destSize<=dest.length){
                return new String(dest,0,destSize);  
            }else{
                dest = new char[destSize];
            }
        }                   
    }
     
    private static final int MAX_BUF_SIZE_COMPOSE = 2;
    private static final int MAX_BUF_SIZE_DECOMPOSE = 3;

    /**
     * Decompose a string.
     * The string will be decomposed to according the the specified mode.
     * @param str     The string to decompose.
     * @param compat  If true the string will be decomposed accoding to NFKD 
     *                 rules and if false will be decomposed according to NFD 
     *                 rules.
     * @param options The normalization options, ORed together (0 for no options).
     * @return String The decomposed string 
     */         
    public static String decompose(String str, boolean compat, int options){
    
        char[] dest = new char[str.length()*MAX_BUF_SIZE_DECOMPOSE];
        int[] trailCC = new int[1];
        int destSize=0;
        for(;;){
            destSize=NormalizerImpl.decompose(str.toCharArray(),0,str.length(),
                                              dest,0,dest.length,
                                              compat,trailCC, options);
            if(destSize<=dest.length){
                return new String(dest,0,destSize); 
            }else{
                dest = new char[destSize];
            }
        } 
            
    }
    
    /**
     * Decompose a string.
     * The string will be decomposed to according the the specified mode.
     * @param str     The string to decompose.
     * @param compat  If true the string will be decomposed accoding to NFKD 
     *                rules and if false will be decomposed according to NFD 
     *                rules.
     * @param options The normalization options, ORed together 
     *                (0 for no options).
     * @return String The decomposed string 
     */            
    public static String decompose(String source, boolean compat, int options, 
                                   boolean addSingleQuotation)
    {
        char[] dest = new char[source.length() * MAX_BUF_SIZE_DECOMPOSE];
        int[] trailCC = new int[1];
        for ( ; ; ) {
            int destSize = NormalizerImpl.decompose(source.toCharArray(), 0, 
                                                    source.length(), dest, 0, 
                                                    dest.length, compat,
                                                    trailCC, options);
            if (destSize <= dest.length) {
                if (!addSingleQuotation) {
                    return new String(dest, 0, destSize); 
		} else {
                    StringBuffer tmpBuf = new StringBuffer();
                    return new String(dest, 0, destSize); 
		}
            }
            else {
                dest = new char[destSize];
            }
        }
    }
    
    /**
     * Normalizes a <tt>String</tt> using the given normalization operation.
     * <p>
     * The <tt>options</tt> parameter specifies which optional
     * <tt>Normalizer</tt> features are to be enabled for this operation.
     * Currently the only available option is {@link #UNICODE_3_2}.
     * If you want the default behavior corresponding to one of the standard
     * Unicode Normalization Forms, use 0 for this argument.
     * <p>
     * @param str       the input string to be normalized.
     * @param aMode     the normalization mode
     * @param options   the optional features to be enabled.
     * @return String   the normalized string
     */
    public static String normalize(String str, Mode mode, int options){
        return mode.normalize(str,options);
    }

    /**
     * Normalize a string.
     * The string will be normalized according the the specified normalization
     * mode and options.
     * @param src       The char array to compose.
     * @param srcStart  Start index of the source
     * @param srcLimit  Limit index of the source
     * @param dest      The char buffer to fill in
     * @param destStart Start index of the destination buffer  
     * @param destLimit End index of the destination buffer
     * @param mode      The normalization mode; one of Normalizer.NONE, 
     *                   Normalizer.NFD, Normalizer.NFC, Normalizer.NFKC, 
     *                   Normalizer.NFKD, Normalizer.DEFAULT
     * @param options The normalization options, ORed together (0 for no options). 
     * @return int      The total buffer size needed;if greater than length of 
     *                   result, the output was truncated.
     * @exception       IndexOutOfBoundsException if the target capacity is 
     *                   less than the required length
     */       
    /// problem: public method access changed to private
    private static int normalize(char[] src,int srcStart, int srcLimit, 
                                char[] dest,int destStart, int destLimit,
                                Mode  mode, int options){
        int length = mode.normalize(src,srcStart,srcLimit,dest,destStart,destLimit, options);
   
        if(length<=(destLimit-destStart)){
            return length;
        }else{
            throw new IndexOutOfBoundsException(Integer.toString(length));
        } 
    }
    
    /**
     * Convenience method.
     *
     * @param source   string for determining if it is in a normalized format
     * @param mode     normalization format (Normalizer.NFC,Normalizer.NFD,  
     *                  Normalizer.NFKC,Normalizer.NFKD)  sherman: FCD???
     * @param options   Options for use with exclusion set an tailored Normalization
     *                   The only option that is currently recognized is UNICODE_3_2     
     * @return         Return code to specify if the text is normalized or not 
     *                     (Normalizer.YES, Normalizer.NO or Normalizer.MAYBE)
     */
    public static QuickCheckResult quickCheck( String source, Mode mode, int options){
        return mode.quickCheck(source.toCharArray(), 0, source.length(), true, options);
    }
    
    /**
     * Retrieving the combining class of argument ch
     * @param ch character to retrieve combining class of
     * @return combining class of ch
     */
    public static final int getClass(int ch) 
    {
        return NormalizerImpl.getCombiningClass(ch);
    }
    
    //-------------------------------------------------------------------------
    // Iteration API
    //-------------------------------------------------------------------------

    /**
     * Return the current character in the normalized text->
     * @return The codepoint as an int
     */
    public int current() {
        if(bufferPos<bufferLimit || nextNormalize()) {
            return getCodePointAt(bufferPos);
        } else {
            return DONE;
        }
    }

    /**
     * Return the next character in the normalized text and advance
     * the iteration position by one.  If the end
     * of the text has already been reached, {@link #DONE} is returned.
     * @return The codepoint as an int
     */
    public int next() {
        if(bufferPos<bufferLimit ||  nextNormalize()) {
            int c=getCodePointAt(bufferPos);
            bufferPos+=(c>0xFFFF) ? 2 : 1;
            return c;
        } else {
            return DONE;
        }
    }

    
    /**
     * Return the previous character in the normalized text and decrement
     * the iteration position by one.  If the beginning
     * of the text has already been reached, {@link #DONE} is returned.
     * @return The codepoint as an int
     */
    public int previous() {
        if(bufferPos>0 || previousNormalize()) {
            int c=getCodePointAt(bufferPos-1);
            bufferPos-=(c>0xFFFF) ? 2 : 1;
            return c;
        } else {
            return DONE;
        }
    }

   /**
    * Reset the index to the beginning of the text.
    * This is equivalent to setIndexOnly(startIndex)).
    */
    public void reset() {
        currentIndex = nextIndex = text.getBeginIndex();
        text.setIndex(currentIndex);
        clearBuffer();
    }
    
    /**
    * Set the iteration position in the input text that is being normalized,
    * without any immediate normalization.
    * After setIndexOnly(), getIndex() will return the same index that is
    * specified here.
    *
    * @param index the desired index in the input text.
    */
    public void setIndexOnly(int index) {
        text.setIndex(index);
        currentIndex=nextIndex=index; // validates index
        clearBuffer();
    }

    /**
     * Set the iteration position in the input text that is being normalized
     * and return the first normalized character at that position.
     * <p>
     * <b>Note:</b> This method sets the position in the <em>input</em> text,
     * while {@link #next} and {@link #previous} iterate through characters
     * in the normalized <em>output</em>.  This means that there is not
     * necessarily a one-to-one correspondence between characters returned
     * by <tt>next</tt> and <tt>previous</tt> and the indices passed to and
     * returned from <tt>setIndex</tt> and {@link #getIndex}.
     * <p>
     * @param index the desired index in the input text->
     *
     * @return   the first normalized character that is the result of iterating
     *            forward starting at the given index.
     *
     * @throws IllegalArgumentException if the given index is less than
     *          {@link #getBeginIndex} or greater than {@link #getEndIndex}.
     * @return The codepoint as an int
     */
    public int setIndex(int index) {
        setIndexOnly(index);
        return current();
    }

    /**
     * Retrieve the index of the start of the input text. This is the begin 
     * index of the <tt>CharacterIterator</tt> or the start (i.e. 0) of the 
     * <tt>String</tt> over which this <tt>Normalizer</tt> is iterating
     * @return The codepoint as an int
     * @see #getEndIndex
     */
    public int getBeginIndex() {
        return 0;
    }

    /**
     * Retrieve the index of the end of the input text.  This is the end index
     * of the <tt>CharacterIterator</tt> or the length of the <tt>String</tt>
     * over which this <tt>Normalizer</tt> is iterating
     * @return The codepoint as an int
     * @see #getBeginIndex
     */
    public int getEndIndex() {
        return text.getLength();
    }
    /**
     * Return the first character in the normalized text->  This resets
     * the <tt>Normalizer's</tt> position to the beginning of the text->
     * @return The codepoint as an int
     */
    public int first() {
        reset();
        return next();
    }
    
    /**
     * Return the last character in the normalized text->  This resets
     * the <tt>Normalizer's</tt> position to be just before the
     * the input text corresponding to that normalized character.
     * @return The codepoint as an int
     */
    public int last() {
        text.setToLimit();
        currentIndex=nextIndex=text.getIndex();
        clearBuffer();
        return previous();
    }
    
    /**
     * Retrieve the current iteration position in the input text that is
     * being normalized.  This method is useful in applications such as
     * searching, where you need to be able to determine the position in
     * the input text that corresponds to a given normalized output character.
     * <p>
     * <b>Note:</b> This method sets the position in the <em>input</em>, while
     * {@link #next} and {@link #previous} iterate through characters in the
     * <em>output</em>.  This means that there is not necessarily a one-to-one
     * correspondence between characters returned by <tt>next</tt> and
     * <tt>previous</tt> and the indices passed to and returned from
     * <tt>setIndex</tt> and {@link #getIndex}.
     * @return The current iteration position
     */
    public int getIndex(){
        if(bufferPos<bufferLimit) {
            return currentIndex;
        } else {
            return nextIndex;
        }
    }
    
    //-------------------------------------------------------------------------
    // Property access methods
    //-------------------------------------------------------------------------
    
    /**
     * Set the normalization mode for this object.
     * <p>
     * <b>Note:</b>If the normalization mode is changed while iterating
     * over a string, calls to {@link #next} and {@link #previous} may
     * return previously buffers characters in the old normalization mode
     * until the iteration is able to re-sync at the next base character.
     * It is safest to call {@link #setText setText()}, {@link #first},
     * {@link #last}, etc. after calling <tt>setMode</tt>.
     * <p>
     * @param newMode the new mode for this <tt>Normalizer</tt>.
     * The supported modes are:
     * <ul>
     *  <li>{@link #COMPOSE}        - Unicode canonical decompositiion
     *                                  followed by canonical composition.
     *  <li>{@link #COMPOSE_COMPAT} - Unicode compatibility decompositiion
     *                                  follwed by canonical composition.
     *  <li>{@link #DECOMP}         - Unicode canonical decomposition
     *  <li>{@link #DECOMP_COMPAT}  - Unicode compatibility decomposition.
     *  <li>{@link #NO_OP}          - Do nothing but return characters
     *                                  from the underlying input text.
     * </ul>
     *
     * @see #getMode
     */
    public void setMode(Mode newMode){
        mode = newMode;
    }
    /**
     * Return the basic operation performed by this <tt>Normalizer</tt>
     *
     * @see #setMode
     */
    public Mode getMode() {
        return mode;
    }

    /**
     * Set options that affect this <tt>Normalizer</tt>'s operation.
     * Options do not change the basic composition or decomposition operation
     * that is being performed , but they control whether
     * certain optional portions of the operation are done.
     * Currently the only available option is:
     * <p>
     * <ul>
     *   <li>{@link #UNICODE_3_2} - Use Normalization conforming to Unicode version 3.2.
     * </ul>
     * <p>
     * @param   option  the option whose value is to be set.
     * @param   value   the new setting for the option.  Use <tt>true</tt> to
     *                  turn the option on and <tt>false</tt> to turn it off.
     *
     * @see #getOption
     */
    public void setOption(int option,boolean value) {
        if (value) {
            options |= option;
        } else {
            options &= (~option);
        }
    }
    
    /**
     * Determine whether an option is turned on or off.
     * <p>
     * @see #setOption
     */
    public int getOption(int option){
        if((options & option)!=0){
            return 1 ;
        }else{
            return 0;
        }
    }

    /**
     * Set the input text over which this <tt>Normalizer</tt> will iterate.
     * The iteration position is set to the beginning of the input text->
     * @param newText   The new string to be normalized.
     */
    public void setText(String newText){
        
        UCharacterIterator newIter = UCharacterIterator.getInstance(newText);
        if (newIter == null) {
                throw new InternalError("Could not create a new UCharacterIterator");
        }  
        text = newIter;
        reset();
    }
    
    /**
     * Set the input text over which this <tt>Normalizer</tt> will iterate.
     * The iteration position is set to the beginning of the input text->
     * @param newText   The new string to be normalized.
     */
    public void setText(CharacterIterator newText){
        
        UCharacterIterator newIter = UCharacterIterator.getInstance(newText);
        if (newIter == null) {
            throw new InternalError("Could not create a new UCharacterIterator");
        }  
        text = newIter;
        reset();
    }
        
    //-------------------------------------------------------------------------
    // Private utility methods
    //-------------------------------------------------------------------------

    /* backward iteration --------------------------------------------------- */
               
    /*
     * read backwards and get norm32
     * return 0 if the character is <minC
     * if c2!=0 then (c2, c) is a surrogate pair (reversed - c2 is first 
     * surrogate but read second!)
     */
    private static  long getPrevNorm32(UCharacterIterator src, 
                                                  int minC, 
                                                  int mask, 
                                                  char[] chars) {
        long norm32;
        int ch=0;
        /* need src.hasPrevious() */
        if((ch=src.previous()) == UCharacterIterator.DONE){
            return 0;
        }
        chars[0]=(char)ch;
        chars[1]=0;
    
        /* check for a surrogate before getting norm32 to see if we need to 
         * predecrement further */
        if(chars[0]<minC) {
            return 0;
	//sherman/Note  need a isSurrogate() method
        //} else if(!UTF16.isSurrogate(chars[0])) {
        } else if(chars[0] < Character.MIN_HIGH_SURROGATE ||
                  chars[0] > Character.MAX_LOW_SURROGATE) {
            return NormalizerImpl.getNorm32(chars[0]);
        } else if(Character.isHighSurrogate(chars[0]) || (src.getIndex()==0)) {
            /* unpaired surrogate */
            chars[1]=(char)src.current();
            return 0;
        } else if(Character.isHighSurrogate(chars[1]=(char)src.previous())) {
            norm32=NormalizerImpl.getNorm32(chars[1]);
            if((norm32&mask)==0) {
                /* all surrogate pairs with this lead surrogate have irrelevant 
                 * data */
                return 0;
            } else {
                /* norm32 must be a surrogate special */
                return NormalizerImpl.getNorm32FromSurrogatePair(norm32,chars[0]);
            }
        } else {
            /* unpaired second surrogate, undo the c2=src.previous() movement */
            src.moveIndex( 1);
            return 0;
        }
    }
    
    private interface IsPrevBoundary{
        public boolean isPrevBoundary(UCharacterIterator src,
                       int/*unsigned*/ minC, 
                       int/*unsigned*/ mask, 
                       char[] chars);
    }
    private static final class IsPrevNFDSafe implements IsPrevBoundary{
        /*
         * for NF*D:
         * read backwards and check if the lead combining class is 0
         * if c2!=0 then (c2, c) is a surrogate pair (reversed - c2 is first 
         * surrogate but read second!)
         */
        public boolean isPrevBoundary(UCharacterIterator src,
                                      int/*unsigned*/ minC, 
                                      int/*unsigned*/ ccOrQCMask, 
                                      char[] chars) {
    
            return NormalizerImpl.isNFDSafe(getPrevNorm32(src, minC, 
                                                          ccOrQCMask, chars), 
                                            ccOrQCMask, 
                                            ccOrQCMask& NormalizerImpl.QC_MASK);
        }
    }
    
    private static final class IsPrevTrueStarter implements IsPrevBoundary{
        /*
         * read backwards and check if the character is (or its decomposition 
         * begins with) a "true starter" (cc==0 and NF*C_YES)
         * if c2!=0 then (c2, c) is a surrogate pair (reversed - c2 is first 
         * surrogate but read second!)
         */
        public boolean isPrevBoundary(UCharacterIterator src, 
                                         int/*unsigned*/ minC,
                                         int/*unsigned*/ ccOrQCMask,
                                         char[] chars) {
            long norm32; 
            int/*unsigned*/ decompQCMask;
            
            decompQCMask=(ccOrQCMask<<2)&0xf; /*decomposition quick check mask*/
            norm32=getPrevNorm32(src, minC, ccOrQCMask|decompQCMask, chars);
            return NormalizerImpl.isTrueStarter(norm32,ccOrQCMask,decompQCMask);
        }
    }
    
    private static int findPreviousIterationBoundary(UCharacterIterator src,
                                                     IsPrevBoundary obj, 
                                                     int/*unsigned*/ minC,
                                                     int/*mask*/ mask,
                                                     char[] buffer, 
                                                     int[] startIndex) {
        char[] chars=new char[2];
        boolean isBoundary;
    
         /* fill the buffer from the end backwards */
        startIndex[0] = buffer.length;
        chars[0]=0;
        while(src.getIndex()>0 && chars[0]!=UCharacterIterator.DONE) {
            isBoundary=obj.isPrevBoundary(src, minC, mask, chars);
    
            /* always write this character to the front of the buffer */
            /* make sure there is enough space in the buffer */
            if(startIndex[0] < (chars[1]==0 ? 1 : 2)) {

                // grow the buffer
                char[] newBuf = new char[buffer.length*2];
                /* move the current buffer contents up */
                System.arraycopy(buffer,startIndex[0],newBuf,
                                 newBuf.length-(buffer.length-startIndex[0]),
                                 buffer.length-startIndex[0]);
                //adjust the startIndex
                startIndex[0]+=newBuf.length-buffer.length;
                
                buffer=newBuf;
                newBuf=null;                
                
            }
    
            buffer[--startIndex[0]]=chars[0];
            if(chars[1]!=0) {
                buffer[--startIndex[0]]=chars[1];
            }
    
            /* stop if this just-copied character is a boundary */
            if(isBoundary) {
                break;
            }
        }
    
        /* return the length of the buffer contents */
        return buffer.length-startIndex[0];
    }
    
    private static int previous(UCharacterIterator src,
                   char[] dest, int destStart, int destLimit, 
                   Mode mode, 
                   boolean doNormalize, 
                   boolean[] pNeededToNormalize,
                   int options) {

        IsPrevBoundary isPreviousBoundary;
        int destLength, bufferLength;
        int/*unsigned*/ mask;
        int[] startIndex= new int[1];
        int c,c2;
        
        char minC;
        int destCapacity = destLimit-destStart;
        destLength=0;
        char[] buffer = new char[100];
        
        if(pNeededToNormalize!=null) {
            pNeededToNormalize[0]=false;
        }
        minC = (char)mode.getMinC();
        mask = mode.getMask();
        isPreviousBoundary = mode.getPrevBoundary();

        if(isPreviousBoundary==null){
            destLength=0;
            if((c=src.previous())>=0) {
                destLength=1;
                if(Character.isLowSurrogate((char)c)){
                    c2= src.previous();
                    if(c2!= UCharacterIterator.DONE){
                        if(Character.isHighSurrogate((char)c2)) {
                            if(destCapacity>=2) {
                                dest[1]=(char)c; // trail surrogate 
                                destLength=2;
                            }
                            // lead surrogate to be written below 
                            c=c2; 
                        } else {
                            src.moveIndex(1);
                        }
                    }
                }
    
                if(destCapacity>0) {
                    dest[0]=(char)c;
                }
            }
            return destLength;
         }
    
        bufferLength=findPreviousIterationBoundary(src,
                                                   isPreviousBoundary, 
                                                   minC, mask,buffer, 
                                                   startIndex);
        if(bufferLength>0) {
            if(doNormalize) {
                destLength=Normalizer.normalize(buffer,startIndex[0],
                                     startIndex[0]+bufferLength,
                                     dest, destStart,destLimit,
                                     mode, options);
                
                if(pNeededToNormalize!=null) {
                    pNeededToNormalize[0]=(boolean)(destLength!=bufferLength ||
                                                    Utility.arrayRegionMatches(
                                                            buffer,0,dest,
                                                            destStart,destLimit
                                                            ));
                }
            } else {
                /* just copy the source characters */
                if(destCapacity>0) {
                    System.arraycopy(buffer,startIndex[0],dest,0,
                                        (bufferLength<destCapacity) ? 
                                                    bufferLength : destCapacity
                                    );
                }
            }
        } 

    
        return destLength;
    }

    /* forward iteration ---------------------------------------------------- */
    /*
     * read forward and check if the character is a next-iteration boundary
     * if c2!=0 then (c, c2) is a surrogate pair
     */
    private interface IsNextBoundary{
        boolean isNextBoundary(UCharacterIterator src, 
                               int/*unsigned*/ minC, 
                               int/*unsigned*/ mask, 
                               int[] chars);
    }   
    /*
     * read forward and get norm32
     * return 0 if the character is <minC
     * if c2!=0 then (c2, c) is a surrogate pair
     * always reads complete characters
     */
    private static long getNextNorm32(UCharacterIterator src, 
				      int minC, 
				      int mask, 
				      int[] chars) {
        long norm32;
    
        /* need src.hasNext() to be true */
        chars[0]=src.next();
        chars[1]=0;
    
        if(chars[0]<minC) {
            return 0;
        }
    
        norm32=NormalizerImpl.getNorm32((char)chars[0]);
        if(Character.isHighSurrogate((char)chars[0])) {
            if(src.current()!=UCharacterIterator.DONE &&
                        Character.isLowSurrogate((char)(chars[1]=src.current()))){
                src.moveIndex(1); /* skip the c2 surrogate */
                if((norm32&mask)==0) {
                    /* irrelevant data */
                    return 0;
                } else {
                    /* norm32 must be a surrogate special */
                    return NormalizerImpl.getNorm32FromSurrogatePair(norm32,(char)chars[1]);
                }
            } else {
                /* unmatched surrogate */
                return 0;
            }
        }
        return norm32;
    }


    /*
     * for NF*D:
     * read forward and check if the lead combining class is 0
     * if c2!=0 then (c, c2) is a surrogate pair
     */
    private static final class IsNextNFDSafe implements IsNextBoundary{
        public boolean isNextBoundary(UCharacterIterator src, 
                               int minC, 
                               int ccOrQCMask, 
                               int[] chars) {
            return NormalizerImpl.isNFDSafe(getNextNorm32(src,minC,ccOrQCMask,chars), 
                             ccOrQCMask, ccOrQCMask&NormalizerImpl.QC_MASK);
       }
    }
    
    /*
     * for NF*C:
     * read forward and check if the character is (or its decomposition begins 
     * with) a "true starter" (cc==0 and NF*C_YES)
     * if c2!=0 then (c, c2) is a surrogate pair
     */
    private static final class IsNextTrueStarter implements IsNextBoundary{
        public boolean isNextBoundary(UCharacterIterator src, 
                               int minC, 
                               int ccOrQCMask, 
                               int[] chars) {
            long norm32;
            int decompQCMask;
            
            decompQCMask=(ccOrQCMask<<2)&0xf; /*decomposition quick check mask*/
            norm32=getNextNorm32(src, minC, ccOrQCMask|decompQCMask, chars);
            return NormalizerImpl.isTrueStarter(norm32, ccOrQCMask, decompQCMask);
        }
    }
    
    private static int findNextIterationBoundary(UCharacterIterator src,
                                                 IsNextBoundary obj, 
                                                 int/*unsigned*/ minC, 
                                                 int/*unsigned*/ mask,
                                                 char[] buffer) {
        int[] chars = new int[2];
        int bufferIndex =0;
        
        if(src.current()==UCharacterIterator.DONE){
            return 0;
        }
        /* get one character and ignore its properties */
        chars[0]=src.next();
        buffer[0]=(char)chars[0];
        bufferIndex=1;
        
        if(Character.isHighSurrogate((char)chars[0])&& 
                                        src.current()!=UCharacterIterator.DONE){
            if(Character.isLowSurrogate((char)(chars[1]=src.next()))){
                buffer[bufferIndex++]=(char)chars[1];
            } else {
                src.moveIndex(-1); /* back out the non-trail-surrogate */
            }
        }
    
        /* get all following characters until we see a boundary */
        /* checking hasNext() instead of c!=DONE on the off-chance that U+ffff 
         * is part of the string */
        while( src.current()!=UCharacterIterator.DONE) {
            if(obj.isNextBoundary(src, minC, mask, chars)) {
                /* back out the latest movement to stop at the boundary */
                src.moveIndex(chars[1]==0 ? -1 : -2);
                break;
            } else {
                if(bufferIndex+(chars[1]==0 ? 1 : 2)<=buffer.length) {
                    buffer[bufferIndex++]=(char)chars[0];
                    if(chars[1]!=0) {
                        buffer[bufferIndex++]=(char)chars[1];
                    }
                }else{
                    char[] newBuf = new char[buffer.length    *2];
                    System.arraycopy(buffer,0,newBuf,0,bufferIndex);
                    buffer = newBuf;
                    buffer[bufferIndex++]=(char)chars[0];
                    if(chars[1]!=0) {
                        buffer[bufferIndex++]=(char)chars[1];
                    }
                }
            }
        }
    
        /* return the length of the buffer contents */
        return bufferIndex;
    }
    
    private static int next(UCharacterIterator src,
                           char[] dest, int destStart, int destLimit,
                           Normalizer.Mode mode,
                           boolean doNormalize, 
                           boolean[] pNeededToNormalize,
                           int options){
                            
        char[] buffer=new char[100];
        IsNextBoundary isNextBoundary;
        int mask;
        int bufferLength;
        int c,c2;
        char minC;
        int destCapacity = destLimit - destStart;
        int destLength = 0;
        int[] startIndex = new int[1];
        if(pNeededToNormalize!=null) {
            pNeededToNormalize[0]=false;
        }

        minC = (char)mode.getMinC();
        mask = mode.getMask();
        isNextBoundary = mode.getNextBoundary();
        
        if(isNextBoundary==null){
            destLength=0;
            c=src.next();
            if(c!=UCharacterIterator.DONE) {
                destLength=1;
                if(Character.isHighSurrogate((char)c)){
                    c2= src.next();
                    if(c2!= UCharacterIterator.DONE) {
                        if(Character.isLowSurrogate((char)c2)) {
                            if(destCapacity>=2) {
                                dest[1]=(char)c2; // trail surrogate 
                                destLength=2;
                            }
                            // lead surrogate to be written below 
                        } else {
                            src.moveIndex(-1);
                        }
                    }
                }
    
                if(destCapacity>0) {
                    dest[0]=(char)c;
                }
            }
            return destLength;
        }
        
        bufferLength=findNextIterationBoundary(src,isNextBoundary, minC, mask,
                                               buffer);
        if(bufferLength>0) {
            if(doNormalize) {
                destLength=mode.normalize(buffer,startIndex[0],bufferLength,
                                          dest,destStart,destLimit, options);
                
                if(pNeededToNormalize!=null) {
                    pNeededToNormalize[0]=(boolean)(destLength!=bufferLength ||
                                Utility.arrayRegionMatches(buffer,startIndex[0],
                                                           dest,destStart,
                                                           destLength));
                }
            } else {
                /* just copy the source characters */
                if(destCapacity>0) {
                    System.arraycopy(buffer,0,dest,destStart,
                                        Math.min(bufferLength,destCapacity)
                                     );
                }
                                      
               
            }
        }
        return destLength;
    } 
  
    private void clearBuffer() {
        bufferLimit=bufferStart=bufferPos=0;
    }
    
    private boolean nextNormalize() {
        
        clearBuffer();
        currentIndex=nextIndex;
        text.setIndex(nextIndex);
            
        bufferLimit=next(text,buffer,bufferStart,buffer.length,mode,true,null,options);
                    
        nextIndex=text.getIndex();
        return (bufferLimit>0);
    }
    
    private boolean previousNormalize() {

        clearBuffer();
        nextIndex=currentIndex;
        text.setIndex(currentIndex);
        bufferLimit=previous(text,buffer,bufferStart,buffer.length,mode,true,null,options);
        
        currentIndex=text.getIndex();
        bufferPos = bufferLimit;
        return bufferLimit>0;
    }
       
    private int getCodePointAt(int index){
        if(Character.isHighSurrogate(buffer[index])){
            if((index+1)<bufferLimit &&
                                Character.isLowSurrogate(buffer[index+1])){
                   return Character.toCodePoint(buffer[index], 
                                                    buffer[index+1]);
            }
        }else if(Character.isLowSurrogate(buffer[index])){
            if(index>0 && Character.isHighSurrogate(buffer[index-1])){
                return Character.toCodePoint(buffer[index-1],
                                                 buffer[index]);
            }
        }   
        return buffer[index];
    }
}
