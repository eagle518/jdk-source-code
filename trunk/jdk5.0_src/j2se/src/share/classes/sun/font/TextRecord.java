/*
 *  @(#)TextRecord.java	1.1 03/04/28
 *
 * (C) Copyright IBM Corp. 2003 - All Rights Reserved
 */

package sun.font;

/**
 * Represents a region of text and context
 */
public final class TextRecord {
    public char[] text;
    public int start;
    public int limit;
    public int min;
    public int max;

    public void init(char[] text, int start, int limit, int min, int max) {
        this.text = text;
        this.start = start;
        this.limit = limit;
        this.min = min;
        this.max = max;
    }
}
