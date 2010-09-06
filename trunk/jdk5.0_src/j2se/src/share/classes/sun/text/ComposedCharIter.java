/*
 * @(#)ComposedCharIter.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.text;

public final class ComposedCharIter {
    /**
     * Constant that indicates the iteration has completed.
     * {@link #next} returns this value when there are no more composed characters
     * over which to iterate.
     */
    public static final int DONE = Normalizer.DONE;

    //cache the decomps mapping, so the seconde composedcharIter does
    //not need to get the data again.
    private static int chars[];
    private static String decomps[];
    private static int decompNum;

    static {
        int maxNum = 2000;     //TBD: Unicode 4.0 only has 1926 canoDecomp...
        chars = new int[maxNum];
        decomps = new String[maxNum];
        decompNum = NormalizerImpl.getDecompose(chars, decomps);
    }

    /**
     * Construct a new <tt>ComposedCharIter</tt>.  The iterator will return
     * all Unicode characters with canonical decompositions, excluding Korean
     * Hangul characters.
     */
    public ComposedCharIter() { }

    /**
     * Returns the next precomposed Unicode character.
     * Repeated calls to <tt>next</tt> return all of the precomposed characters defined
     * by Unicode, in ascending order.  After all precomposed characters have
     * been returned, {@link #hasNext} will return <tt>false</tt> and further calls
     * to <tt>next</tt> will return {@link #DONE}.
     */
    public int next() {
        if (curChar == decompNum - 1) {
            return DONE;
        }
        return chars[++curChar];
    }

    /**
     * Returns the Unicode decomposition of the current character.
     * This method returns the decomposition of the precomposed character most
     * recently returned by {@link #next}.  The resulting decomposition is
     * affected by the settings of the options passed to the constructor.
     */
    public String decomposition() {
        return decomps[curChar];
    }
    private int curChar = -1;
}
