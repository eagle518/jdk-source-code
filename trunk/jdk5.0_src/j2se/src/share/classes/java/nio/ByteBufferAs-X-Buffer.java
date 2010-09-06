/*
 * @(#)ByteBufferAs-X-Buffer.java	1.17 04/05/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#warn This file is preprocessed before being compiled

package java.nio;


class ByteBufferAs$Type$Buffer$RW$$BO$			// package-private
    extends {#if[ro]?ByteBufferAs}$Type$Buffer{#if[ro]?$BO$}
{

#if[rw]

    protected final ByteBuffer bb;
    protected final int offset;

#end[rw]

    ByteBufferAs$Type$Buffer$RW$$BO$(ByteBuffer bb) {	// package-private
#if[rw]
	super(-1, 0,
	      bb.remaining() >> $LG_BYTES_PER_VALUE$,
	      bb.remaining() >> $LG_BYTES_PER_VALUE$);
	this.bb = bb;
	// enforce limit == capacity
	int cap = this.capacity();
	this.limit(cap);
	int pos = this.position();
	assert (pos <= cap);
	offset = pos;
#else[rw]
	super(bb);
#end[rw]
    }

    ByteBufferAs$Type$Buffer$RW$$BO$(ByteBuffer bb,
				     int mark, int pos, int lim, int cap,
				     int off)
    {
#if[rw]
	super(mark, pos, lim, cap);
	this.bb = bb;
	offset = off;
#else[rw]
	super(bb, mark, pos, lim, cap, off);
#end[rw]
    }

    public $Type$Buffer slice() {
	int pos = this.position();
	int lim = this.limit();
	assert (pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);
	int off = (pos << $LG_BYTES_PER_VALUE$) + offset;
        assert (off >= 0);
	return new ByteBufferAs$Type$Buffer$RW$$BO$(bb, -1, 0, rem, rem, off);
    }

    public $Type$Buffer duplicate() {
	return new ByteBufferAs$Type$Buffer$RW$$BO$(bb,
						    this.markValue(),
						    this.position(),
						    this.limit(),
						    this.capacity(),
						    offset);
    }

    public $Type$Buffer asReadOnlyBuffer() {
#if[rw]
	return new ByteBufferAs$Type$BufferR$BO$(bb,
						 this.markValue(),
						 this.position(),
						 this.limit(),
						 this.capacity(),
						 offset);
#else[rw]
	return duplicate();
#end[rw]
    }

#if[rw]

    protected int ix(int i) {
	return (i << $LG_BYTES_PER_VALUE$) + offset;
    }

    public $type$ get() {
	return Bits.get$Type$$BO$(bb, ix(nextGetIndex()));
    }

    public $type$ get(int i) {
	return Bits.get$Type$$BO$(bb, ix(checkIndex(i)));
    }

#end[rw]

    public $Type$Buffer put($type$ x) {
#if[rw]
	Bits.put$Type$$BO$(bb, ix(nextPutIndex()), x);
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public $Type$Buffer put(int i, $type$ x) {
#if[rw]
	Bits.put$Type$$BO$(bb, ix(checkIndex(i)), x);
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public $Type$Buffer compact() {
#if[rw]
	int pos = position();
	int lim = limit();
	assert (pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);

	ByteBuffer db = bb.duplicate();
 	db.limit(ix(lim));
	db.position(ix(0));
	ByteBuffer sb = db.slice();
	sb.position(pos << $LG_BYTES_PER_VALUE$);
	sb.compact();
 	position(rem);
	limit(capacity());
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public boolean isDirect() {
	return bb.isDirect();
    }

    public boolean isReadOnly() {
	return {#if[rw]?false:true};
    }

#if[char]

    public String toString(int start, int end) {
	if ((end > limit()) || (start > end))
	    throw new IndexOutOfBoundsException();
	try {
	    int len = end - start;
	    char[] ca = new char[len];
	    CharBuffer cb = CharBuffer.wrap(ca);
	    CharBuffer db = this.duplicate();
	    db.position(start);
	    db.limit(end);
	    cb.put(db);
	    return new String(ca);
	} catch (StringIndexOutOfBoundsException x) {
	    throw new IndexOutOfBoundsException();
	}
    }


    // --- Methods to support CharSequence ---

    public CharSequence subSequence(int start, int end) {
	int pos = position();
	int lim = limit();
	assert (pos <= lim);
	pos = (pos <= lim ? pos : lim);
	int len = lim - pos;

	if ((start < 0) || (end > len) || (start > end))
	    throw new IndexOutOfBoundsException();
	int sublen = end - start;
 	int off = offset + ((pos + start) << $LG_BYTES_PER_VALUE$);
        assert (off >= 0);
	return new ByteBufferAsCharBuffer$RW$$BO$(bb, -1, 0, sublen, sublen, off);
    }

#end[char]


    public ByteOrder order() {
#if[boB]
	return ByteOrder.BIG_ENDIAN;
#end[boB]
#if[boL]
	return ByteOrder.LITTLE_ENDIAN;
#end[boL]
    }

}
