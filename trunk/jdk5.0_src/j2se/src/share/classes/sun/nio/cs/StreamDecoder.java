/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)StreamDecoder.java	1.16 04/02/12
 */

package sun.nio.cs;

import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.nio.charset.*;
import sun.io.ByteToCharConverter;
import sun.io.Converters;
import sun.io.ConversionBufferFullException;


public abstract class StreamDecoder
    extends Reader
{

    private static final int MIN_BYTE_BUFFER_SIZE = 32;
    private static final int DEFAULT_BYTE_BUFFER_SIZE = 8192;

    private volatile boolean isOpen = true;

    private StreamDecoder() { }

    private StreamDecoder(Object lock) {
	super(lock);
    }

    private void ensureOpen() throws IOException {
	if (!isOpen)
	    throw new IOException("Stream closed");
    }


    // -- Abstract methods implemented below by inner classes --

    abstract int implRead(char[] cb, int off, int end) throws IOException;

    abstract String encodingName();

    abstract boolean implReady();

    abstract void implClose() throws IOException;


    // In order to handle surrogates properly we must never try to produce
    // fewer than two characters at a time.  If we're only asked to return one
    // character then the other is saved here to be returned later.
    //
    private boolean haveLeftoverChar = false;
    private char leftoverChar;


    // Factories for java.io.InputStreamReader

    public static StreamDecoder forInputStreamReader(InputStream in,
						     Object lock,
						     String charsetName)
	throws UnsupportedEncodingException
    {
	String csn = charsetName;
	if (csn == null)
	    csn = Converters.getDefaultEncodingName();
	if (!Converters.isCached(Converters.BYTE_TO_CHAR, csn)) {
	    try {
		if (Charset.isSupported(csn))
		    return new CharsetSD(in, lock, Charset.forName(csn));
	    } catch (IllegalCharsetNameException x) { }
	}
	return new ConverterSD(in, lock, csn);
    }

    public static StreamDecoder forInputStreamReader(InputStream in,
						     Object lock,
						     Charset cs)
    {
	return new CharsetSD(in, lock, cs);
    }

    public static StreamDecoder forInputStreamReader(InputStream in,
						     Object lock,
						     CharsetDecoder dec)
    {
	return new CharsetSD(in, lock, dec);
    }


    // Factory for java.nio.channels.Channels.newReader

    public static StreamDecoder forDecoder(ReadableByteChannel ch,
					   CharsetDecoder dec,
					   int minBufferCap)
    {
	return new CharsetSD(ch, dec, minBufferCap);
    }


    // -- Public methods corresponding to those in InputStreamReader --

    // All synchronization and state/argument checking is done in these public
    // methods; the concrete stream-decoder subclasses defined below need not
    // do any such checking.

    public String getEncoding() {
	if (isOpen())
	    return encodingName();
	return null;
    }

    public int read() throws IOException {
        return read0();
    }

    private int read0() throws IOException {
	synchronized (lock) {

	    // Return the leftover char, if there is one
	    if (haveLeftoverChar) {
		haveLeftoverChar = false;
		return leftoverChar;
	    }

	    // Convert more bytes
	    char cb[] = new char[2];
	    int n = read(cb, 0, 2);
	    switch (n) {
	    case -1:
		return -1;
	    case 2:
		leftoverChar = cb[1];
		haveLeftoverChar = true;
		// FALL THROUGH
	    case 1:
		return cb[0];
	    default:
 		assert false : n;
		return -1;
	    }
	}
    }

    public int read(char cbuf[], int offset, int length) throws IOException {
	int off = offset;
	int len = length;
        synchronized (lock) {
            ensureOpen();
            if ((off < 0) || (off > cbuf.length) || (len < 0) ||
                ((off + len) > cbuf.length) || ((off + len) < 0)) {
                throw new IndexOutOfBoundsException();
            }
	    if (len == 0)
                return 0;

	    int n = 0;

	    if (haveLeftoverChar) {
		// Copy the leftover char into the buffer
		cbuf[off] = leftoverChar;
		off++; len--;
		haveLeftoverChar = false;
		n = 1;
		if ((len == 0) || !implReady())
		    // Return now if this is all we can produce w/o blocking
		    return n;
	    }

	    if (len == 1) {
		// Treat single-character array reads just like read()
		int c = read0();
		if (c == -1)
		    return (n == 0) ? -1 : n;
		cbuf[off] = (char)c;
 		return n + 1;
	    }

	    return n + implRead(cbuf, off, off + len);
        }
    }

    public boolean ready() throws IOException {
        synchronized (lock) {
            ensureOpen();
	    return haveLeftoverChar || implReady();
        }
    }

    public void close() throws IOException {
        synchronized (lock) {
            if (!isOpen)
                return;
	    implClose();
	    isOpen = false;
	}
    }

    private boolean isOpen() {
	return isOpen;
    }


    // -- Converter-based stream decoder --

    private static class ConverterSD
	extends StreamDecoder
    {
	private final InputStream in;
	private ByteToCharConverter btc;
	private byte[] bb;		/* Input buffer */
	private int nBytes = 0;		/* -1 implies EOF has been reached */
	private int nextByte = 0;

	private ConverterSD(InputStream in, Object lock, String charsetName)
	    throws UnsupportedEncodingException
	{
	    super(lock);
	    this.in = in;
	    this.btc = sun.io.ByteToCharConverter.getConverter(charsetName);
	    bb = new byte[DEFAULT_BYTE_BUFFER_SIZE];
	}

	private ConverterSD(ReadableByteChannel ch, String csn)
	    throws UnsupportedEncodingException
	{
	    this(Channels.newInputStream(ch), null, csn);
	    lock = this;
	}

	private void malfunction() {
	    throw new InternalError("Converter malfunction (" +
				    btc.getCharacterEncoding() +
				    ") -- please submit a bug report via " +
				    System.getProperty("java.vendor.url.bug"));
	}

	private int convertInto(char cbuf[], int off, int end)
	    throws IOException
	{
	    int nc = 0;
	    if (nextByte < nBytes) {
		try {
		    nc = btc.convert(bb, nextByte, nBytes,
				     cbuf, off, end);
		    nextByte = nBytes;
		    if (btc.nextByteIndex() != nextByte)
			malfunction();
		}
		catch (ConversionBufferFullException x) {
		    nextByte = btc.nextByteIndex();
		    nc = btc.nextCharIndex() - off;
		}
	    }
	    return nc;
	}

	private int flushInto(char cbuf[], int off, int end)
	    throws IOException
	{
	    int nc = 0;
	    try {
		nc = btc.flush(cbuf, off, end);
	    }
	    catch (ConversionBufferFullException x) {
		nc = btc.nextCharIndex() - off;
	    }
	    return nc;
	}

	int implRead(char cbuf[], int off, int end) throws IOException {
	    int nc = 0;

	    if (nextByte < nBytes)
		nc = convertInto(cbuf, off, end);

	    while (off + nc < end) {

		if (nBytes != -1) {
		    if ((nc > 0) && !inReady())
			break;      /* Block at most once */
		    nBytes = in.read(bb);
		}

		if (nBytes == -1) {
		    nBytes = 0; /* Allow file to grow */
		    nc += flushInto(cbuf, off + nc, end);
		    if (nc == 0)
			return -1;
		    else
			break;
		} else {
		    nextByte = 0;
		    nc += convertInto(cbuf, off + nc, end);
		}

	    }
	    return nc;
	}

	String encodingName() {
	    return btc.getCharacterEncoding();
	}

	private boolean inReady() {
	    try {
		return in.available() > 0;
	    } catch (IOException x) {
		return false;
	    }
	}

	boolean implReady() {
            return (nextByte < nBytes) || inReady();
	}

	void implClose() throws IOException {
	    in.close();
	}

    }


    // -- Charset-based stream decoder --

    // In the early stages of the build we haven't yet built the NIO native
    // code, so guard against that by catching the first UnsatisfiedLinkError
    // and setting this flag so that later attempts fail quickly.
    //
    private static volatile boolean channelsAvailable = true;

    private static FileChannel getChannel(FileInputStream in) {
	if (!channelsAvailable)
	    return null;
	try {
	    return in.getChannel();
	} catch (UnsatisfiedLinkError x) {
	    channelsAvailable = false;
	    return null;
	}
    }

    private static class CharsetSD extends StreamDecoder {

	private Charset cs;
	private CharsetDecoder decoder;
	private ByteBuffer bb;

	// Exactly one of these is non-null
	private InputStream in;
	private ReadableByteChannel ch;

	CharsetSD(InputStream in, Object lock, Charset cs) {
	    this(in, lock,
		 cs.newDecoder()
		 .onMalformedInput(CodingErrorAction.REPLACE)
		 .onUnmappableCharacter(CodingErrorAction.REPLACE));
	}

	CharsetSD(InputStream in, Object lock, CharsetDecoder dec) {
	    super(lock);
	    this.cs = dec.charset();
	    this.decoder = dec;

	    // This path disabled until direct buffers are faster
	    if (false && in instanceof FileInputStream) {
		ch = getChannel((FileInputStream)in);
		if (ch != null)
		    bb = ByteBuffer.allocateDirect(DEFAULT_BYTE_BUFFER_SIZE);
	    }
	    if (ch == null) {
		this.in = in;
		this.ch = null;
		bb = ByteBuffer.allocate(DEFAULT_BYTE_BUFFER_SIZE);
	    }
	    bb.flip();			// So that bb is initially empty
	}

	CharsetSD(ReadableByteChannel ch, CharsetDecoder dec, int mbc) {
	    this.in = null;
	    this.ch = ch;
	    this.decoder = dec;
	    this.cs = dec.charset();
	    this.bb = ByteBuffer.allocate(mbc < 0
					  ? DEFAULT_BYTE_BUFFER_SIZE
					  : (mbc < MIN_BYTE_BUFFER_SIZE
					     ? MIN_BYTE_BUFFER_SIZE
					     : mbc));
	    bb.flip();
	}

	private int readBytes() throws IOException {
	    bb.compact();
	    try {
		if (ch != null) {
		    // Read from the channel
		    int n = ch.read(bb);
		    if (n < 0)
			return n;
		} else {
		    // Read from the input stream, and then update the buffer
		    int lim = bb.limit();
		    int pos = bb.position();
		    assert (pos <= lim);
		    int rem = (pos <= lim ? lim - pos : 0);
		    assert rem > 0;
		    int n = in.read(bb.array(), bb.arrayOffset() + pos, rem);
		    if (n < 0)
			return n;
		    if (n == 0)
			throw new IOException("Underlying input stream returned zero bytes");
		    assert (n <= rem) : "n = " + n + ", rem = " + rem;
		    bb.position(pos + n);
		}
	    } finally {
		// Flip even when an IOException is thrown,
		// otherwise the stream will stutter
		bb.flip();
	    }

	    int rem = bb.remaining();
 	    assert (rem != 0) : rem;
 	    return rem;
	}

	int implRead(char[] cbuf, int off, int end) throws IOException {

	    // In order to handle surrogate pairs, this method requires that
	    // the invoker attempt to read at least two characters.  Saving the
	    // extra character, if any, at a higher level is easier than trying
	    // to deal with it here.
	    assert (end - off > 1);

	    CharBuffer cb = CharBuffer.wrap(cbuf, off, end - off);
	    if (cb.position() != 0)
		// Ensure that cb[0] == cbuf[off]
		cb = cb.slice();

	    boolean eof = false;
	    for (;;) {
		CoderResult cr = decoder.decode(bb, cb, eof);
		if (cr.isUnderflow()) {
		    if (eof)
			break;
		    if (!cb.hasRemaining())
			break;
		    if ((cb.position() > 0) && !inReady())
			break;		// Block at most once
		    int n = readBytes();
		    if (n < 0) {
			eof = true;
			if ((cb.position() == 0) && (!bb.hasRemaining()))
			    break;
			decoder.reset();
		    }
		    continue;
		}
		if (cr.isOverflow()) {
		    assert cb.position() > 0;
		    break;
		}
		cr.throwException();
	    }

	    if (eof) {
		// ## Need to flush decoder
		decoder.reset();
	    }

	    if (cb.position() == 0) {
		if (eof)
		    return -1;
		assert false;
	    }
	    return cb.position();
	}

	String encodingName() {
	    return ((cs instanceof HistoricallyNamedCharset)
		    ? ((HistoricallyNamedCharset)cs).historicalName()
		    : cs.name());
	}

	private boolean inReady() {
	    try {
		return (((in != null) && (in.available() > 0))
			|| (ch instanceof FileChannel)); // ## RBC.available()?
	    } catch (IOException x) {
		return false;
	    }
	}

	boolean implReady() {
            return bb.hasRemaining() || inReady();
	}

	void implClose() throws IOException {
	    if (ch != null)
		ch.close();
	    else
		in.close();
	}

    }

}
