/**
 * @(#)Scanner.java	1.57 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.parser;

import java.io.*;
import java.nio.*;
import java.nio.ByteBuffer;
import java.nio.charset.*;
import java.nio.channels.*;
import java.util.regex.*;

import com.sun.tools.javac.util.*;

import com.sun.tools.javac.code.Source;

import static com.sun.tools.javac.parser.Tokens.*;

/** The lexical analyzer maps an input stream consisting of
 *  ASCII characters and Unicode escapes into a token sequence.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Scanner implements /* imports */ LayoutCharacters {


    /** A factory for creating scanners. */
    public static class Factory {
	/** The context key for the scanner factory. */
	protected static final Context.Key<Scanner.Factory> scannerFactoryKey =
	    new Context.Key<Scanner.Factory>();

	/** Get the Factory instance for this context. */
	public static Factory instance(Context context) {
	    Factory instance = context.get(scannerFactoryKey);
	    if (instance == null)
		instance = new Factory(context);
	    return instance;
	}

	final Log log;
	final Name.Table names;
	final Source source;
	final Keywords keywords;
	final ByteBufferCache byteBufferCache;

	/** Create a new scanner factory. */
	protected Factory(Context context) {
	    context.put(scannerFactoryKey, this);
	    this.log = Log.instance(context);
	    this.names = Name.Table.instance(context);
	    this.source = Source.instance(context);
	    this.keywords = Keywords.instance(context);
	    this.byteBufferCache = ByteBufferCache.instance(context);
	}

	public Scanner newScanner(InputStream input, String encoding)
	    throws IOException {
	    return new Scanner(this, input, encoding);
	}

        public Scanner newScanner(CharBuffer buffer) {
            return new Scanner(this, buffer);
        }

        public Scanner newScanner(char[] input, int inputLength) {
            return new Scanner(this, input, inputLength);
        }
    }

    /* Output variables; set by nextToken():
     */

    /** The token, set by nextToken().
     */
    private Tokens token;

    /** Allow hex floating-point literals.
     */
    private boolean allowHexFloats;

    /** The token's position. pos = line << Position.LINESHIFT + col.
     *  Line and column numbers start at 1.
     */
    private int pos;

    /** The last character position of the token.
     */
    private int endPos;

    /** The last character position of the previous token.
     */
    private int prevEndPos;

    /** The position where a lexical error occurred;
     */
    private int errPos = Position.NOPOS;

    /** The name of an identifier or token:
     */
    private Name name;

    /** The radix of a numeric literal token.
     */
    private int radix;

    /** Has a @deprecated been encountered in last doc comment?
     *  this needs to be reset by client.
     */
    private boolean deprecatedFlag = false;

    /** A character buffer for literals.
     */
    private char[] sbuf = new char[128];
    private int sp;

    /** The input buffer, index of next chacter to be read,
     *  index of one past last character in buffer.
     */
    private char[] buf;
    private int bp;
    private int buflen;

    /** The current character.
     */
    private char ch;

    /** The line number position of the current character.
     */
    private int line;

    /** The column number position of the current character.
     */
    private int col;

    /** The buffer index of the last converted unicode character
     */
    private int unicodeConversionBp = 0;

    /** The log to be used for error reporting.
     */
    private final Log log;

    /** The name table. */
    private final Name.Table names;

    /** The keyword table. */
    private final Keywords keywords;

    /**
     * Documentation string of the current token.
     */
    private String docComment = null;

    /**
     * Buffer for doc comment.
     */
    private char[] docCommentBuffer;

    /**
     * Number of characters in doc comment buffer.
     */
    private int docCommentCount;

    /**
     * Source version.
     */
    private Source source;

    /**
     * The name of the platform's default encoding.
     */
    private static String defaultEncodingName =
	// System.getProperty("file.encoding");
	new OutputStreamWriter(new ByteArrayOutputStream()).getEncoding();

    /** Common code for constructors. */
    private Scanner(Factory fac) {
	this.log = fac.log;
	this.names = fac.names;
	this.keywords = fac.keywords;
	this.byteBufferCache = fac.byteBufferCache;
	this.source = fac.source;
	this.allowHexFloats = fac.source.allowHexFloats();
    }

    private static final boolean hexFloatsWork = hexFloatsWork();
    private static boolean hexFloatsWork() {
        try {
            Float.valueOf("0x1.0p1");
            return true;
        } catch (NumberFormatException ex) {
            return false;
        }
    }

    /** Create a scanner from the input buffer.  buffer must implement
     *  array() and compact(), and remaining() must be less than limit().
     */
    protected Scanner(Factory fac, CharBuffer buffer) {
	this(fac,
	     ((CharBuffer)buffer.compact().flip()).array(),
	     buffer.limit());
    }

    /** Create a scanner from the input array.  The array must have at
     *  least a single character of extra space.
     */
    protected Scanner(Factory fac, char[] input, int inputLength) {
	this(fac);
	assert inputLength < input.length;
	buf = input;
	buflen = inputLength;
	buf[buflen] = EOI;
        line = 1;
        col = 0;
        bp = -1;
	scanChar();
        nextToken();
    }

    /** Create a scanner from an input stream. */
    protected Scanner(Factory fac, InputStream in, String encodingName)
	throws IOException {
	this(fac, makeByteBuffer(fac, in), encodingName, true);
	in.close();
    }

    /** Make a byte buffer from an input stream. */
    private static ByteBuffer makeByteBuffer(Factory fac,
					     InputStream in)
	throws IOException {
	int limit = in.available();
	if (limit < 1024) limit = 1024;
	ByteBuffer result = fac.byteBufferCache.get(limit);
	int position = 0;
	while (in.available() != 0) {
	    if (position >= limit)
		// expand buffer
		result = ByteBuffer.
		    allocate(limit <<= 1).
		    put((ByteBuffer)result.flip());
	    int count = in.read(result.array(),
				position,
				limit - position);
	    if (count < 0) break;
	    result.position(position += count);
	}
	return (ByteBuffer)result.flip();
    }

    /** A single-element cache of direct byte buffers. */
    private static class ByteBufferCache {
	private ByteBuffer cached;
	ByteBuffer get(int capacity) {
	    if (capacity < 20480) capacity = 20480;
	    ByteBuffer result =
		(cached != null && cached.capacity() >= capacity)
		? (ByteBuffer)cached.clear()
		: ByteBuffer.allocate(capacity + capacity>>1);
	    cached = null;
	    return result;
	}
	void put(ByteBuffer x) {
	    cached = x;
	}
	protected static final Context.Key<ByteBufferCache> key =
	    new Context.Key<ByteBufferCache>();
	protected ByteBufferCache(Context context) {
	    context.put(key, this);
	}
	static ByteBufferCache instance(Context context) {
	    ByteBufferCache instance = context.get(key);
	    if (instance == null)
		instance = new ByteBufferCache(context);
	    return instance;
	}
    }
    private final ByteBufferCache byteBufferCache;

    /** Make a scanner from a ByteBuffer, possibly disposing the buffer. */
    private Scanner(Factory fac,
		    ByteBuffer buffer,
		    String encodingName,
		    boolean dispose) {
	this(fac, decode(fac, buffer, encodingName));
	if (dispose) byteBufferCache.put(buffer);
    }

    /** Make a scanner from a ByteBuffer. */
    public Scanner(Factory fac, ByteBuffer buffer, String encodingName) {
	this(fac, buffer, encodingName, false);
    }

    /** Decode a ByteBuffer into a CharBuffer. */
    private static CharBuffer decode(Factory fac,
				     ByteBuffer inbuf,
				     String encodingName) {
	CharsetDecoder decoder;
	try {
	    if (encodingName == null) encodingName = defaultEncodingName;
	    Charset charset = Charset.forName(encodingName);
	    decoder = charset.newDecoder();
	    decoder.
		onMalformedInput(CodingErrorAction.REPORT).
		onUnmappableCharacter(CodingErrorAction.REPORT);
	} catch (IllegalCharsetNameException e) {
            lexError(fac.log,
		     Position.NOPOS,
		     "unsupported.encoding",
		     encodingName);
            return (CharBuffer)CharBuffer.allocate(1).flip();
	} catch (UnsupportedCharsetException e) {
            lexError(fac.log,
		     Position.NOPOS,
		     "unsupported.encoding",
		     encodingName);
            return (CharBuffer)CharBuffer.allocate(1).flip();
        }
	// slightly overestimate the buffer size to avoid reallocation.
	float factor =
	    decoder.averageCharsPerByte() * 0.8f +
	    decoder.maxCharsPerByte() * 0.2f;
	CharBuffer dest = CharBuffer.
	    allocate(10 + (int)(inbuf.remaining()*factor));

        while (true) {
	    CoderResult result = decoder.decode(inbuf, dest, true);
	    dest.flip();

	    if (result.isUnderflow()) { // done reading
		// mare sure there is at least one extra character
		if (dest.limit() == dest.capacity()) {
		    dest = CharBuffer.allocate(dest.capacity()+1).put(dest);
		    dest.flip();
		}
		return dest;
	    } else if (result.isOverflow()) { // buffer too small; expand
		int newCapacity =
		    10 + dest.capacity() +
		    (int)(inbuf.remaining()*decoder.maxCharsPerByte());
		dest = CharBuffer.allocate(newCapacity).put(dest);
	    } else if (result.isMalformed() || result.isUnmappable()) {
		// bad character in input

		// find line of error
		int line = 0;
		int lastLineStart = 0;
		Pattern linePattern = Pattern.compile("^.*$",
						      Pattern.MULTILINE);
		Matcher m = linePattern.matcher(dest);
		while (m.find()) {
		    line++;
		    lastLineStart = m.start();
		}

		// find column of error
		int column = 0;
		for (int i=lastLineStart; i<dest.limit(); i++) {
		    if (dest.get(i) == '\t')
			column = (column / TabInc * TabInc) + TabInc;
		    else
			column++;
		}
		column++;

		// report coding error (warn only pre 1.5)
		if (!fac.source.allowEncodingErrors()) {
		    lexError(fac.log,
			     Position.make(line, column),
			     "illegal.char.for.encoding",
			     encodingName);
		} else {
		    lexWarning(fac.log,
			     Position.make(line, column),
			     "illegal.char.for.encoding",
			     encodingName);
		}

		// skip past the coding error
		inbuf.position(inbuf.position() + result.length());

		// undo the flip() to prepare the output buffer
		// for more translation
		dest.position(dest.limit());
		dest.limit(dest.capacity());
		dest.put((char)0xfffd); // backward compatible
	    } else {
		throw new AssertionError(result);
	    }
	}
	// unreached
    }

    /** Report a warning at the given position using the provided argument.
     */
    private static void lexWarning(Log log, int pos, String key, Object arg) {
        log.warning(pos, key, arg);
    }

    /** Report an error at the given position using the provided argument.
     */
    private static void lexError(Log log, int pos, String key, Object arg) {
        log.error(pos, key, arg);
    }

    /** Report an error at the given position.
     */
    private static void lexError(Log log, int pos, String key) {
        lexError(log, pos, key, null);
    }

    /** Report an error at the given position using the provided argument.
     */
    private void lexError(int pos, String key, Object arg) {
        log.error(pos, key, arg);
        token = ERROR;
        errPos = pos;
    }

    /** Report an error at the given position.
     */
    private void lexError(int pos, String key) {
	lexError(pos, key, null);
    }

    /** Report an error at the current token position.
     */
    private void lexError(String key) {
	lexError(pos, key, null);
    }

    /** Report an error at the current token position using the provided
     *  argument.
     */
    private void lexError(String key, Object arg) {
	lexError(pos, key, arg);
    }

    /** Report a warning at the given position.
     */
    private void lexWarning(int pos, String key) {
	log.warning(pos, key);
    }

    /** Convert an ASCII digit from its base (8, 10, or 16)
     *  to its value.
     */
    private int digit(int base) {
	char c = ch;
	int result = Character.digit(c, base);
	if (result >= 0 && c > 0x7f) {
	    lexWarning(pos+1, "illegal.nonascii.digit");
	    ch = "0123456789abcdef".charAt(result);
	}
	return result;
    }

    /** Convert unicode escape; bp points to initial '\' character
     *  (Spec 3.3).
     */
    private void convertUnicode() {
	int startcol = col;
	if (ch == '\\' && unicodeConversionBp != bp) {
	    bp++; ch = buf[bp]; col++;
	    if (ch == 'u') {
		do {
		    bp++; ch = buf[bp]; col++;
		} while (ch == 'u');
		int limit = bp + 3;
		if (limit < buflen) {
		    int d = digit(16);
		    int code = d;
		    while (bp < limit && d >= 0) {
			bp++; ch = buf[bp]; col++;
			d = digit(16);
			code = (code << 4) + d;
		    }
		    if (d >= 0) {
			ch = (char)code;
			unicodeConversionBp = bp;
			return;
		    }
		}
		lexError(Position.make(line, startcol), "illegal.unicode.esc");
	    } else {
		bp--;
		ch = '\\';
		col--;
	    }
	}
    }

    /** Read next character.
     */
    private void scanChar() {
	bp++;
	ch = buf[bp];
	switch (ch) {
	case '\r': // return
	    col = 0;
	    line++;
	    break;
	case '\n': // newline
	    if (bp == 0 || buf[bp-1] != '\r') {
		col = 0;
		line++;
	    }
	    break;
	case '\t': // tab
	    col = (col / TabInc * TabInc) + TabInc;
	    break;
	case '\\': // possible unicode
	    col++;
	    convertUnicode();
	    break;
	default:
	    col++;
	    break;
	}
    }

    /** Read next character in comment, skipping over double '\' characters.
     */
    private void scanCommentChar() {
	scanChar();
	if (ch == '\\') {
	    if (buf[bp+1] == '\\' && unicodeConversionBp != bp) {
		bp++; col++;
	    } else {
		convertUnicode();
	    }
	}
    }

    /** Unconditionally expand the comment buffer.
     */
    private void expandCommentBuffer() {
	char[] newBuffer = new char[docCommentBuffer.length * 2];
	System.arraycopy(docCommentBuffer, 0, newBuffer,
			 0, docCommentBuffer.length);
	docCommentBuffer = newBuffer;
    }

    /**
     * Read next character in doc comment, skipping over double '\' characters.
     * If a double '\' is skipped, put in the buffer and update buffer count.
     */
    private void scanDocCommentChar() {
	scanChar();
	if (ch == '\\') {
	    if (buf[bp+1] == '\\' && unicodeConversionBp != bp) {
		if (docCommentCount == docCommentBuffer.length)
		    expandCommentBuffer();
	        docCommentBuffer[docCommentCount++] = ch;
		bp++; col++;
	    } else {
		convertUnicode();
	    }
	}
    }

    /** Append a character to sbuf.
     */
    private void putChar(char ch) {
	if (sp == sbuf.length) {
	    char[] newsbuf = new char[sbuf.length * 2];
	    System.arraycopy(sbuf, 0, newsbuf, 0, sbuf.length);
	    sbuf = newsbuf;
	}
	sbuf[sp++] = ch;
    }

    /** For debugging purposes: print character.
     */
    private void dch() {
        System.err.print((char)ch); System.out.flush();
    }

    /** Read next character in character or string literal and copy into sbuf.
     */
    private void scanLitChar() {
        if (ch == '\\') {
	    if (buf[bp+1] == '\\' && unicodeConversionBp != bp) {
		bp++; col++;
		putChar('\\');
		scanChar();
	    } else {
		scanChar();
		switch (ch) {
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		    char leadch = ch;
		    int oct = digit(8);
		    scanChar();
		    if ('0' <= ch && ch <= '7') {
			oct = oct * 8 + digit(8);
			scanChar();
			if (leadch <= '3' && '0' <= ch && ch <= '7') {
			    oct = oct * 8 + digit(8);
			    scanChar();
			}
		    }
		    putChar((char)oct);
		    break;
		case 'b':
		    putChar('\b'); scanChar(); break;
		case 't':
		    putChar('\t'); scanChar(); break;
		case 'n':
		    putChar('\n'); scanChar(); break;
		case 'f':
		    putChar('\f'); scanChar(); break;
		case 'r':
		    putChar('\r'); scanChar(); break;
		case '\'':
		    putChar('\''); scanChar(); break;
		case '\"':
		    putChar('\"'); scanChar(); break;
		case '\\':
		    putChar('\\'); scanChar(); break;
		default:
 		    lexError(Position.make(line, col), "illegal.esc.char");
		}
	    }
	} else if (bp != buflen) {
            putChar(ch); scanChar();
        }
    }

    /** Read fractional part of hexadecimal floating point number.
     */
    private void scanHexExponentAndSuffix() {
        if (ch == 'p' || ch == 'P') {
	    putChar(ch);
            scanChar();
            if (ch == '+' || ch == '-') {
		putChar(ch);
                scanChar();
	    }
	    if ('0' <= ch && ch <= '9') {
		do {
		    putChar(ch);
		    scanChar();
		} while ('0' <= ch && ch <= '9');
		if (!allowHexFloats) {
		    lexError("unsupported.fp.lit");
                    allowHexFloats = true;
                }
                else if (!hexFloatsWork)
		    lexError("unsupported.cross.fp.lit");
	    } else
		lexError("malformed.fp.lit");
	} else {
	    lexError("malformed.fp.lit");
	}
	if (ch == 'f' || ch == 'F') {
	    putChar(ch);
	    scanChar();
            token = FLOATLITERAL;
	} else {
	    if (ch == 'd' || ch == 'D') {
		putChar(ch);
		scanChar();
	    }
	    token = DOUBLELITERAL;
	}
    }

    /** Read fractional part of floating point number.
     */
    private void scanFraction() {
        while (digit(10) >= 0) {
	    putChar(ch);
            scanChar();
        }
	int sp1 = sp;
        if (ch == 'e' || ch == 'E') {
	    putChar(ch);
            scanChar();
            if (ch == '+' || ch == '-') {
		putChar(ch);
                scanChar();
	    }
	    if ('0' <= ch && ch <= '9') {
		do {
		    putChar(ch);
		    scanChar();
		} while ('0' <= ch && ch <= '9');
		return;
	    }
	    lexError("malformed.fp.lit");
	    sp = sp1;
	}
    }

    /** Read fractional part and 'd' or 'f' suffix of floating point number.
     */
    private void scanFractionAndSuffix() {
	this.radix = 10;
	scanFraction();
	if (ch == 'f' || ch == 'F') {
	    putChar(ch);
	    scanChar();
            token = FLOATLITERAL;
	} else {
	    if (ch == 'd' || ch == 'D') {
		putChar(ch);
		scanChar();
	    }
	    token = DOUBLELITERAL;
	}
    }

    /** Read fractional part and 'd' or 'f' suffix of floating point number.
     */
    private void scanHexFractionAndSuffix(boolean seendigit) {
	this.radix = 16;
	assert ch == '.';
	putChar(ch);
	scanChar();
        while (digit(16) >= 0) {
	    seendigit = true;
	    putChar(ch);
            scanChar();
        }
	if (!seendigit)
	    lexError("invalid.hex.number");
	else
	    scanHexExponentAndSuffix();
    }

    /** Read a number.
     *  @param radix  The radix of the number; one of 8, 10, 16.
     */
    private void scanNumber(int radix) {
	this.radix = radix;
	// for octal, allow base-10 digit in case it's a float literal
	int digitRadix = (radix <= 10) ? 10 : 16;
	boolean seendigit = false;
	while (digit(digitRadix) >= 0) {
	    seendigit = true;
	    putChar(ch);
	    scanChar();
	}
	if (radix == 16 && ch == '.') {
	    scanHexFractionAndSuffix(seendigit);
	} else if (seendigit && radix == 16 && (ch == 'p' || ch == 'P')) {
	    scanHexExponentAndSuffix();
	} else if (radix <= 10 && ch == '.') {
	    putChar(ch);
	    scanChar();
	    scanFractionAndSuffix();
	} else if (radix <= 10 &&
		   (ch == 'e' || ch == 'E' ||
		    ch == 'f' || ch == 'F' ||
		    ch == 'd' || ch == 'D')) {
	    scanFractionAndSuffix();
	} else {
	    if (ch == 'l' || ch == 'L') {
		scanChar();
		token = LONGLITERAL;
	    } else {
		token = INTLITERAL;
	    }
	}
    }

    /** Read an identifier.
     */
    private void scanIdent() {
	do {
	    if (sp == sbuf.length) putChar(ch); else sbuf[sp++] = ch;
	    // optimization, was: putChar(ch);

	    scanChar();
	    switch (ch) {
	    case 'A': case 'B': case 'C': case 'D': case 'E':
	    case 'F': case 'G': case 'H': case 'I': case 'J':
	    case 'K': case 'L': case 'M': case 'N': case 'O':
	    case 'P': case 'Q': case 'R': case 'S': case 'T':
	    case 'U': case 'V': case 'W': case 'X': case 'Y':
	    case 'Z':
	    case 'a': case 'b': case 'c': case 'd': case 'e':
	    case 'f': case 'g': case 'h': case 'i': case 'j':
	    case 'k': case 'l': case 'm': case 'n': case 'o':
	    case 'p': case 'q': case 'r': case 's': case 't':
	    case 'u': case 'v': case 'w': case 'x': case 'y':
	    case 'z':
	    case '$': case '_':
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
            case '\u0000': case '\u0001': case '\u0002': case '\u0003':
            case '\u0004': case '\u0005': case '\u0006': case '\u0007':
            case '\u0008': case '\u000E': case '\u000F': case '\u0010':
            case '\u0011': case '\u0012': case '\u0013': case '\u0014':
            case '\u0015': case '\u0016': case '\u0017':
            case '\u0018': case '\u0019': case '\u001B':
            case '\u007F':
		break;
            case '\u001A': // EOI is also a legal identifier part
                if (bp >= buflen) {
                    name = names.fromChars(sbuf, 0, sp);
                    token = keywords.key(name);
                    return;
                }
                break;
	    default:
                boolean isJavaIdentifierPart;
                if (ch < '\u0080') {
                    // all ASCII range chars already handled, above
                    isJavaIdentifierPart = false;
                } else {
                    char high = scanSurrogates();
                    if (high != 0) {
	                if (sp == sbuf.length) {
                            putChar(high);
                        } else {
                            sbuf[sp++] = high;
                        }
                        isJavaIdentifierPart = Character.isJavaIdentifierPart(
                            Character.toCodePoint(high, ch));
                    } else {
                        isJavaIdentifierPart = Character.isJavaIdentifierPart(ch);
                    }
                }
		if (!isJavaIdentifierPart) {
		    name = names.fromChars(sbuf, 0, sp);
		    token = keywords.key(name);
		    return;
		}
	    }
	} while (true);
    }

    /** Are surrogates supported?
     */
    final static boolean surrogatesSupported = surrogatesSupported();
    private static boolean surrogatesSupported() {
        try {
            boolean b = Character.isHighSurrogate('a');
            return true;
        } catch (NoSuchMethodError ex) {
            return false;
        }
    }

    /** Scan surrogate pairs.  If 'ch' is a high surrogate and
     *  the next character is a low surrogate, then put the low
     *  surrogate in 'ch', and return the high surrogate.
     *  otherwise, just return 0.
     */
    private char scanSurrogates() {
        if (surrogatesSupported && Character.isHighSurrogate(ch)) {
            char high = ch;

            scanChar();

            if (Character.isLowSurrogate(ch)) {
                return high;
            }

            ch = high;
        }

        return 0;
    }

    /** Return true if ch can be part of an operator.
     */
    private boolean isSpecial(char ch) {
        switch (ch) {
        case '!': case '%': case '&': case '*': case '?':
        case '+': case '-': case ':': case '<': case '=':
        case '>': case '^': case '|': case '~':
	case '@':
            return true;
        default:
            return false;
        }
    }

    /** Read longest possible sequence of special characters and convert
     *  to token.
     */
    private void scanOperator() {
	while (true) {
	    putChar(ch);
	    Name newname = names.fromChars(sbuf, 0, sp);
            if (keywords.key(newname) == IDENTIFIER) {
		sp--;
		break;
	    }
            name = newname;
            token = keywords.key(newname);
	    scanChar();
	    if (!isSpecial(ch)) break;
	}
    }

    /** Scan a doccomment line after the inital '*''s for
     *  a @deprecated tag. This should be extended to support all javadoc tags.
     */
    private void scanDocCommentTag() {
	int start = bp + 1;
	do {
	    scanDocCommentChar();
	} while ('a' <= ch && ch <= 'z');
	if (names.fromChars(buf, start, bp - start) == names.deprecated) {
	    deprecatedFlag = true;
	}
    }

    /**
     * Skip a non-documentation comment. This method should be called once
     * the initial /, * and the next character have been read.
     */
    private void skipComment() {
	while (bp < buflen) {
	    switch (ch) {
	    case '*':
		scanChar();
		if (ch == '/') return;
		break;
	    default:
		scanCommentChar();
		break;
	    }
	}
    }

    /**
     * Scan a documention comment and return it as a string. This method
     * should be called once the initial /, * and * have been read. It
     * gathers the content of the comment (witout leading spaces and '*'s)
     * in the string buffer. Stops on the close '/'.
     */
    private String scanDocComment() {

	// buffer where the doc comment is stored
	if (docCommentBuffer == null) docCommentBuffer = new char[1024];
	docCommentCount = 0;
	
	boolean linestart = false;
        boolean firstLine = true;

	// consume any number of stars
	while (bp < buflen && ch == '*') {
	    scanDocCommentChar();
	}
	// is the comment in the form /**/, /***/, /****/, etc. ?
	if (bp < buflen && ch == '/') {
	    return "";
	}

	// skip a newline on the first line of the comment.
	if (bp < buflen) {
	    if (ch == LF) {
		scanDocCommentChar();
                firstLine = false;
	    } else if (ch == CR) {
		scanDocCommentChar();
		if (ch == LF) {
		    scanDocCommentChar();
                    firstLine = false;
		}
	    }
	}

    outerLoop:

	// The outerLoop processes the doc comment, looping once
	// for each line.  For each line, it first strips off
	// whitespace, then it consumes any stars, then it
	// puts the rest of the line into our buffer.
	while (bp < buflen) {

	    // The wsLoop consumes whitespace from the beginning
	    // of each line.
	wsLoop:

	    while (bp < buflen) {
		switch(ch) {
		case ' ':
		    scanDocCommentChar();
		    break;
		case '\t':
		    col = ((col - 1) / TabInc * TabInc) + TabInc;
		    scanDocCommentChar();
		    break;
 		case FF:
 		    col = 0;
 		    scanDocCommentChar();
 		    break;
// Treat newline at beginning of line (blank line, no star)
// as comment text.  Old javadoc compatibility requires this.
/*---------------------------------*
 		case CR: // (Spec 3.4)
 		    scanDocCommentChar();
 		    if (ch == LF) {
 			col = 0;
 			scanDocCommentChar();
 		    }
 		    break;
 		case LF: // (Spec 3.4)
 		    scanDocCommentChar();
 		    break;
*---------------------------------*/
		default:
		    // we've seen something that isn't whitespace;
		    // jump out.
		    break wsLoop;
		}
	    }

	    // Are there stars here?  If so, consume them all
	    // and check for the end of comment.
	    if (ch == '*') {
		// skip all of the stars
		do {
		    scanDocCommentChar();
		} while (ch == '*');
		
		// check for the closing slash.
		if (ch == '/') {
		    // We're done with the doc comment
		    // scanChar() and breakout.
		    break outerLoop;
		}
	    } else if (! firstLine) {
                //The current line does not begin with a '*' so we will indent it.
                for (int i = 1; i < col; i++) {
                    if (docCommentCount == docCommentBuffer.length)
			expandCommentBuffer();
                    docCommentBuffer[docCommentCount++] = ' ';
                }
            }
	
	    linestart = true;
	
	    // The textLoop processes the rest of the characters
	    // on the line, adding them to our buffer.
	textLoop:
	    while (bp < buflen) {
		switch (ch) {
		case '*':
		    // Is this just a star?  Or is this the
		    // end of a comment?
		    linestart = false;
		    scanDocCommentChar();
		    if (ch == '/') {
			// This is the end of the comment,
			// set ch and return our buffer.
			break outerLoop;
		    }
		    // This is just an ordinary star.  Add it to
		    // the buffer.
		    if (docCommentCount == docCommentBuffer.length)
			expandCommentBuffer();
		    docCommentBuffer[docCommentCount++] = '*';
		    break;
		case ' ':
		case '\t':
		    if (docCommentCount == docCommentBuffer.length)
			expandCommentBuffer();
		    docCommentBuffer[docCommentCount++] = (char)ch;
		    scanDocCommentChar();
		    break;
 		case FF:
 		    scanDocCommentChar();
 		    break textLoop; // treat as end of line
 		case CR: // (Spec 3.4)
 		    scanDocCommentChar();
 		    if (ch != LF) {
		        // Canonicalize CR-only line terminator to LF
		        if (docCommentCount == docCommentBuffer.length)
			    expandCommentBuffer();
			docCommentBuffer[docCommentCount++] = (char)LF;
			break textLoop;
		    }
		    /* fall through to LF case */
		case LF: // (Spec 3.4)
		    // We've seen a newline.  Add it to our
		    // buffer and break out of this loop,
		    // starting fresh on a new line.
		    if (docCommentCount == docCommentBuffer.length)
			expandCommentBuffer();
		    docCommentBuffer[docCommentCount++] = (char)ch;
  		    scanDocCommentChar();
  		    break textLoop;
		default:
		    if (ch == '@' && linestart) {
			// scan possible @deprecated tag. since scanDocCommentTag() scans
			// forward, also remember to put the chars into doc comment buffer.
			int start = bp + 1;
			do {
			    if (docCommentCount == docCommentBuffer.length)
				expandCommentBuffer();
			    docCommentBuffer[docCommentCount++] = (char)ch;
			    scanDocCommentChar();
			} while ('a' <= ch && ch <= 'z');
			if (names.fromChars(buf, start, bp - start) ==
			    names.deprecated) {
			    deprecatedFlag = true;
			}
		    } else {
			// Add the character to our buffer.
			if (docCommentCount == docCommentBuffer.length)
			    expandCommentBuffer();
			docCommentBuffer[docCommentCount++] = (char)ch;
			scanDocCommentChar();
		    }
		    linestart = false;
		}
	    } // end textLoop
            firstLine = false;
	} // end outerLoop

	if (docCommentCount > 0) {
	    int i = docCommentCount - 1;
	trailLoop:
	    while (i > -1) {
		switch (docCommentBuffer[i]) {
		case '*':
		    i--;
		    break;
		default:
		    break trailLoop;
		}
	    }
	    docCommentCount = i + 1;
	
	    // Return the text of the doc comment.
	    return new String(docCommentBuffer, 0, docCommentCount);
	} else {
	    return "";
	}
    }


    /** The value of a literal token, recorded as a string.
     *  For integers, leading 0x and 'l' suffixes are suppressed.
     */
    public String stringVal() {
	return new String(sbuf, 0, sp);
    }

    /** Read token.
     */
    public void nextToken() {

	try {
	    prevEndPos = endPos;
	    sp = 0;
	    docComment = null; // reset docComment to null for every token read
	
	    while (true) {
		pos = (line << Position.LINESHIFT) + col;
		int start = bp;
		switch (ch) {
		case ' ': // (Spec 3.6)
		case '\t': // (Spec 3.6)
		case FF: // (Spec 3.6)
		case CR: // (Spec 3.4)
		case LF: // (Spec 3.4)
		    scanChar();
		    break;
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
		case '$': case '_':
		    scanIdent();
		    return;
		case '0':
		    scanChar();
		    if (ch == 'x' || ch == 'X') {
			scanChar();
			if (ch == '.') {
			    scanHexFractionAndSuffix(false);
			} else if (digit(16) < 0) {
			    lexError("invalid.hex.number");
			} else {
			    scanNumber(16);
			}
		    } else {
			putChar('0');
			scanNumber(8);
		    }
		    return;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		    scanNumber(10);
		    return;
		case '.':
		    scanChar();
		    if ('0' <= ch && ch <= '9') {
			putChar('.');
			scanFractionAndSuffix();
		    } else if (ch == '.') {
			putChar('.'); putChar('.');
			scanChar();
			if (ch == '.') {
			    scanChar();
			    putChar('.');
			    token = ELLIPSIS;
			} else {
			    lexError("malformed.fp.lit");
			}
		    } else {
			token = DOT;
		    }
		    return;
		case ',':
		    scanChar(); token = COMMA; return;
		case ';':
		    scanChar(); token = SEMI; return;
		case '(':
		    scanChar(); token = LPAREN; return;
		case ')':
		    scanChar(); token = RPAREN; return;
		case '[':
		    scanChar(); token = LBRACKET; return;
		case ']':
		    scanChar(); token = RBRACKET; return;
		case '{':
		    scanChar(); token = LBRACE; return;
		case '}':
		    scanChar(); token = RBRACE; return;
		case '/':
		    scanChar();
		    if (ch == '/') {
			do {
			    scanCommentChar();
			} while (ch != CR && ch != LF && bp < buflen);
			break;
		    } else if (ch == '*') {
			scanChar();
			if (ch == '*') { // if we're scanning a docComment
			    docComment = scanDocComment();
			} else {
			    skipComment();
			}
			if (ch == '/') {
			    scanChar();
			    break;
			} else {
			    lexError("unclosed.comment");
			    return;
			}
		    } else if (ch == '=') {
			name = names.slashequals;
			token = SLASHEQ;
			scanChar();
		    } else {
			name = names.slash;
			token = SLASH;
		    }
		    return;
		case '\'':
		    scanChar();
		    if (ch == '\'') {
			lexError("empty.char.lit");
		    } else {
			if (ch == CR || ch == LF)
			    lexError(pos, "illegal.line.end.in.char.lit");
			scanLitChar();
			if (ch == '\'') {
			    scanChar();
			    token = CHARLITERAL;
			} else {
			    lexError(pos, "unclosed.char.lit");
			}
		    }
		    return;
		case '\"':
		    scanChar();
		    while (ch != '\"' && ch != CR && ch != LF && bp < buflen)
			scanLitChar();
		    if (ch == '\"') {
			token = STRINGLITERAL;
			scanChar();
		    } else {
			lexError(pos, "unclosed.str.lit");
		    }
		    return;
		default:
		    if (isSpecial(ch)) {
			scanOperator();
		    } else {
                        boolean isJavaIdentifierStart;
                        if (ch < '\u0080') {
                            // all ASCII range chars already handled, above
                            isJavaIdentifierStart = false;
                        } else {
                            char high = scanSurrogates();
                            if (high != 0) {
	                        if (sp == sbuf.length) {
                                    putChar(high);
                                } else {
                                    sbuf[sp++] = high;
                                }

                                isJavaIdentifierStart = Character.isJavaIdentifierStart(
                                    Character.toCodePoint(high, ch));
                            } else {
                                isJavaIdentifierStart = Character.isJavaIdentifierStart(ch);
                            }
                        }
                        if (isJavaIdentifierStart) {
			    scanIdent();
		        } else if (bp == buflen || ch == EOI && bp+1 == buflen) { // JLS 3.5
			    token = EOF;
		        } else {
                            lexError("illegal.char", String.valueOf((int)ch));
			    scanChar();
		        }
		    }
		    return;
		}
	    }
	} finally {
	    endPos = (line << Position.LINESHIFT) + col - 1;
	}
    }

    /** Return the current token, set by nextToken().
     */
    public Tokens token() {
        return token;
    }

    /** Sets the current token.
     */
    public void token(Tokens token) {
        this.token = token;
    }

    /** Return the current token's position.
     *  pos = line << Position.LINESHIFT + col.
     *  Line and column numbers start at 1.
     */
    public int pos() {
        return pos;
    }

    /** Return the last character position of the current token.
     */
    public int endPos() {
        return endPos;
    }

    /** Return the last character position of the previous token.
     */
    public int prevEndPos() {
        return prevEndPos;
    }

    /** Return the position where a lexical error occurred;
     */
    public int errPos() {
        return errPos;
    }

    /** Set the position where a lexical error occurred;
     */
    public void errPos(int pos) {
        errPos = pos;
    }

    /** Return the name of an identifier or token for the current token.
     */
    public Name name() {
        return name;
    }

    /** Return the radix of a numeric literal token.
     */
    public int radix() {
        return radix;
    }

    /** Has a @deprecated been encountered in last doc comment?
     *  This needs to be reset by client with resetDeprecatedFlag.
     */
    public boolean deprecatedFlag() {
        return deprecatedFlag;
    }

    public void resetDeprecatedFlag() {
        deprecatedFlag = false;
    }

    /**
     * Returns the documentation string of the current token.
     */
    public String docComment() {
        return docComment;
    }
}

