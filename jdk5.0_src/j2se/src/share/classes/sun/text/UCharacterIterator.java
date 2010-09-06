/*
 * @(#)UCharacterIterator.java	1.4 03/12/19
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
 * <p>
 * Iterator class that defines an API for iteration on text objects.This is an 
 * interface for forward and backward iteration and random access into a text 
 * object. Forward iteration is done with post-increment and backward iteration 
 * is done with pre-decrement semantics, while the 
 * <code>java.text.CharacterIterator</code> interface methods provided forward 
 * iteration with "pre-increment" and backward iteration with pre-decrement 
 * semantics. This API is more efficient for forward iteration over code points.
 * The other major difference is that this API can do both code unit and code point 
 * iteration, <code>java.text.CharacterIterator</code> can only iterate over 
 * code units and is limited to BMP (0 - 0xFFFF)
 * </p>
 * @author Ram
 */
public class UCharacterIterator implements Cloneable 
{
    // protected constructor ------------------------------------------------
    
    private UCharacterIterator(String str)
    {
        if (str==null) {
            throw new NullPointerException();
        }
        text  = str;
        currentIndex = 0;
    }
    
    /**
     * Protected default constructor for the subclasses
     */
    protected UCharacterIterator()
    {
    }
    
    // public methods -------------------------------------------------------
    
    /**
     * Returns a <code>UCharacterIterator</code> object given a 
     * source string.
     * @param source a string
     * @return UCharacterIterator object
     * @exception NullPointerException if the argument is null
     */
    /// problem: removed TextUCharacterIterator 
    public static final UCharacterIterator getInstance(String source)
    {
        return new UCharacterIterator(source);
    }
    
    /**
     * <p>
     * Returns a <code>UCharacterIterator</code> object given a 
     * CharacterIterator.
     * </p>
     * @param source a valid CharacterIterator object.
     * @return UCharacterIterator object
     * @exception NullPointerException if the argument is null
     */
    public static final UCharacterIterator getInstance(CharacterIterator source)
    {
        return new CharacterIteratorWrapper(source);
    }
    
    /**
     * Returns the code unit at the current index.  If index is out
     * of range, returns DONE.  Index is not changed.
     * @return current code unit
     */
    public int current()
    {
        if (currentIndex < text.length()) {
            return text.charAt(currentIndex);
        }
        return DONE;
    }
    
    /**
     * Returns the length of the text
     * @return length of the text
     */
    public int getLength()
    {
        return text.length();
    }

    /**
     * Gets the current index in text.
     * @return current index in text.
     */
    public int getIndex()
    {
        return currentIndex;
    }

    /**
     * Returns the UTF16 code unit at index, and increments to the next
     * code unit (post-increment semantics).  If index is out of
     * range, DONE is returned, and the iterator is reset to the limit
     * of the text.
     * @return the next UTF16 code unit, or DONE if the index is at the limit
     *         of the text.
     */
    public int next()
    {
        if (currentIndex < text.length()) {
            return text.charAt(currentIndex++);
        }
        return DONE;
    }
    
    /**
     * Returns the code point at index, and increments to the next code
     * point (post-increment semantics).  If index does not point to a
     * valid surrogate pair, the behavior is the same as
     * <code>next()<code>.  Otherwise the iterator is incremented past
     * the surrogate pair, and the code point represented by the pair
     * is returned.
     * @return the next codepoint in text, or DONE if the index is at
     *         the limit of the text.
     */
    public int nextCodePoint(){
        int ch1 = next();
        if(Character.isHighSurrogate((char)ch1)){
            int ch2 = next();
            if(Character.isLowSurrogate((char)ch2)){
                return Character.toCodePoint((char)ch1, (char)ch2);
            }else if (ch2 != DONE) {
                // unmatched surrogate so back out
                previous();
            }
        }
        return ch1;
    }
    
    /**
     * Decrement to the position of the previous code unit in the
     * text, and return it (pre-decrement semantics).  If the
     * resulting index is less than 0, the index is reset to 0 and
     * DONE is returned.
     * @return the previous code unit in the text, or DONE if the new
     *         index is before the start of the text.
     */
    public int previous()
    {
        if (currentIndex > 0) {
            return text.charAt(--currentIndex);
        }
        return DONE;
    }

    public int previousCodePoint(){
        int ch1 = previous();
        if(Character.isLowSurrogate((char)ch1)){
            int ch2 = previous();
            if(Character.isHighSurrogate((char)ch2)){
                return Character.toCodePoint((char)ch2, (char)ch1);
            }else if (ch2 != DONE) {
                //unmatched trail surrogate so back out
                next();
            }   
        }
        return ch1;
    }

    
    /**
     * Sets the index to the specified index in the text.
     * @param currentIndex the index within the text. 
     * @exception An IllegalArgumentException is thrown if an invalid
     *            index is supplied
     */
    public void setIndex(int currentIndex)
    {
        if (currentIndex < 0 || currentIndex > text.length()) {
            throw new IllegalArgumentException("Invalid index");
        }
        this.currentIndex = currentIndex;
    }


    /**
     * Returns the start index of the text.
     * @return the index at which the text begins.
     */
     public int getBeginIndex() {
         return 0;
     }


    /**
     * Sets the current index to the limit.
     */
    public void setToLimit() 
    {
        setIndex(getLength());
    }

       
    /**
     * Moves the current position by the number of code units
     * specified, either forward or backward depending on the sign
     * of delta (positive or negative respectively).  If the resulting
     * index would be less than zero, the index is set to zero, and if
     * the resulting index would be greater than limit, the index is
     * set to limit.
     *
     * @param delta the number of code units to move the current
     *              index.
     * @return the new index.
     * @exception An IllegalArgumentExceptio is thrown if an invalid
     *            index is supplied 
     */
    public int moveIndex(int delta) {
		int x = Math.max(0, Math.min(getIndex() + delta, getLength()));
		setIndex(x);
		return x;
    }
    
    /**
     * Creates a copy of this iterator, independent from other iterators.
     * If it is not possible to clone the iterator, returns null.
     */
    public Object clone() throws CloneNotSupportedException
    {
        return super.clone();
    }   
    
    // UForwardCharacterIterator --------------------------------------------
    
    /**
     * Indicator that we have reached the ends of the UTF16 text.
     */
    public static final int DONE = -1;
        
    
    private String text;
    /**
     * Current currentIndex
     */
    private int currentIndex;

    /**
     * <p>
     * This class is a wrapper around CharacterIterator and implements the 
     * UCharacterIterator protocol
     * @author ram
     * </p>
     */
    static class CharacterIteratorWrapper extends UCharacterIterator 
    {    
        private CharacterIterator iterator;
        
        CharacterIteratorWrapper(CharacterIterator iter)
        {
            if (iter == null) {
                throw new NullPointerException();
            }
            iterator = iter;   
        }
    
        public int current() {
            int c = iterator.current();
            if(c==CharacterIterator.DONE){
    		  return DONE;
            }
            return c;
        }

        public int getLength() {
    	    return (iterator.getEndIndex() - iterator.getBeginIndex());
        }
    
        public int getIndex() {
            return iterator.getIndex();
        }
    
        public int next() {
            int i = iterator.current();
            iterator.next();
            if(i==CharacterIterator.DONE){  
    		  return DONE;
            }
            return i;
        }
        public int previous() {
            int i = iterator.previous();
            if(i==CharacterIterator.DONE){
                return DONE;
            }
            return i;
        }
    
        public void setIndex(int index) {
            iterator.setIndex(index);
        } 


        public int getBeginIndex() {
            return iterator.getBeginIndex();
        }

    }
}
    
    
