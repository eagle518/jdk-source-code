/*
 * @(#)X-Buffer-bin.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#warn This file is preprocessed before being compiled

class XXX {

#begin

    /**
     * Relative <i>get</i> method for reading $a$ $type$ value.
     *
     * <p> Reads the next $nbytes$ bytes at this buffer's current position,
     * composing them into $a$ $type$ value according to the current byte order,
     * and then increments the position by $nbytes$.  </p>
     *
     * @return  The $type$ value at the buffer's current position
     *
     * @throws  BufferUnderflowException
     *          If there are fewer than $nbytes$ bytes
     *          remaining in this buffer
     */
    public abstract $type$ get$Type$();

    /**
     * Relative <i>put</i> method for writing $a$ $type$
     * value&nbsp;&nbsp;<i>(optional operation)</i>.
     *
     * <p> Writes $nbytes$ bytes containing the given $type$ value, in the
     * current byte order, into this buffer at the current position, and then
     * increments the position by $nbytes$.  </p>
     *
     * @param  value
     *         The $type$ value to be written
     *
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If there are fewer than $nbytes$ bytes
     *          remaining in this buffer
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public abstract ByteBuffer put$Type$($type$ value);

    /**
     * Absolute <i>get</i> method for reading $a$ $type$ value.
     *
     * <p> Reads $nbytes$ bytes at the given index, composing them into a
     * $type$ value according to the current byte order.  </p>
     *
     * @param  index
     *         The index from which the bytes will be read
     *
     * @return  The $type$ value at the given index
     *
     * @throws  IndexOutOfBoundsException
     *          If <tt>index</tt> is negative
     *          or not smaller than the buffer's limit,
     *          minus $nbytesButOne$
     */
    public abstract $type$ get$Type$(int index);

    /**
     * Absolute <i>put</i> method for writing $a$ $type$
     * value&nbsp;&nbsp;<i>(optional operation)</i>.
     *
     * <p> Writes $nbytes$ bytes containing the given $type$ value, in the
     * current byte order, into this buffer at the given index.  </p>
     *
     * @param  index
     *         The index at which the bytes will be written
     *
     * @param  value
     *         The $type$ value to be written
     *
     * @return  This buffer
     *
     * @throws  IndexOutOfBoundsException
     *          If <tt>index</tt> is negative
     *          or not smaller than the buffer's limit,
     *          minus $nbytesButOne$
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public abstract ByteBuffer put$Type$(int index, $type$ value);

    /**
     * Creates a view of this byte buffer as $a$ $type$ buffer.
     *
     * <p> The content of the new buffer will start at this buffer's current
     * position.  Changes to this buffer's content will be visible in the new
     * buffer, and vice versa; the two buffers' position, limit, and mark
     * values will be independent.
     *
     * <p> The new buffer's position will be zero, its capacity and its limit
     * will be the number of bytes remaining in this buffer divided by
     * $nbytes$, and its mark will be undefined.  The new buffer will be direct
     * if, and only if, this buffer is direct, and it will be read-only if, and
     * only if, this buffer is read-only.  </p>
     *
     * @return  A new $type$ buffer
     */
    public abstract $Type$Buffer as$Type$Buffer();

#end

}
