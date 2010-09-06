/*
 * @(#)TrueTypeFont.java	1.7 04/01/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.GraphicsEnvironment;
import java.awt.geom.Point2D;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.FileChannel;
import java.util.HashSet;
import java.util.Locale;
import sun.java2d.Disposer;
import sun.java2d.DisposerRecord;

/**
 * TrueTypeFont is not called SFntFont because it is not expected
 * to handle all types that may be housed in a such a font file.
 * If additional types are supported later, it may make sense to
 * create an SFnt superclass. Eg to handle sfnt-housed postscript fonts.
 * OpenType fonts are handled by this class, and possibly should be
 * represented by a subclass.
 * An instance stores some information from the font file to faciliate
 * faster access. File size, the table directory and the names of the font
 * are the most important of these. It amounts to approx 400 bytes
 * for a typical font. Systems with mutiple locales sometimes have up to 400
 * font files, and an app which loads all font files would need around
 * 160Kbytes. So storing any more info than this would be expensive.
 */
public class TrueTypeFont extends FileFont {

   /* -- Tags for required TrueType tables */
    public static final int cmapTag = 0x636D6170; // 'cmap'
    public static final int glyfTag = 0x676C7966; // 'glyf'
    public static final int headTag = 0x68656164; // 'head'
    public static final int hheaTag = 0x68686561; // 'hhea'
    public static final int hmtxTag = 0x686D7478; // 'hmtx'
    public static final int locaTag = 0x6C6F6361; // 'loca'
    public static final int maxpTag = 0x6D617870; // 'maxp'
    public static final int nameTag = 0x6E616D65; // 'name'
    public static final int postTag = 0x706F7374; // 'post'
    public static final int os_2Tag = 0x4F532F32; // 'OS/2'

    /* -- Tags for opentype related tables */
    public static final int GDEFTag = 0x47444546; // 'GDEF'
    public static final int GPOSTag = 0x47504F53; // 'GPOS'
    public static final int GSUBTag = 0x47535542; // 'GSUB'
    public static final int mortTag = 0x6D6F7274; // 'mort'

    /* -- Tags for non-standard tables */
    public static final int fdscTag = 0x66647363; // 'fdsc' - gxFont descriptor
    public static final int fvarTag = 0x66766172; // 'fvar' - gxFont variations
    public static final int featTag = 0x66656174; // 'feat' - layout features
    public static final int EBLCTag = 0x45424C43; // 'EBLC' - embedded bitmaps

    /* --  Other tags */
    public static final int ttcfTag = 0x74746366; // 'ttcf' - TTC file
    public static final int v1ttTag = 0x00010000; // 'v1tt' - Version 1 TT font
    public static final int trueTag = 0x74727565; // 'true' - Version 2 TT font

    /* -- ID's used in the 'name' table */
    public static final int MS_PLATFORM_ID = 3;
    /* MS locale id for US English is the "default" */
    public static final short ENGLISH_LOCALE_ID = 0x0409; // 1033 decimal
    public static final int FAMILY_NAME_ID = 1;
    // public static final int STYLE_WEIGHT_ID = 2; // currently unused.
    public static final int FULL_NAME_ID = 4;
    public static final int POSTSCRIPT_NAME_ID = 6;


    class DirectoryEntry {
	int tag;
	int offset;
	int length;
    }

    /* There is a pool which limits the number of fd's that are in
     * use. Normally fd's are closed as they are replaced in the pool.
     * But if an instance of this class becomes unreferenced, then there
     * needs to be a way to close the fd. A finalize() method could do this,
     * but using the Disposer class will ensure its called in a more timely
     * manner. This is not something which should be relied upon to free
     * fd's - its a safeguard.
     */
    private class TTDisposerRecord extends DisposerRecord {
	
	FileChannel channel = null;

	public synchronized void dispose() {
	    try {
		if (channel != null) {
		    channel.close(); 
		}
	    } catch (IOException e) {
	    } finally {
		channel = null;
	    }
	}
    }

    TTDisposerRecord disposerRecord = new TTDisposerRecord();

    /* > 0 only if this font is a part of a collection */
    int fontIndex = 0;

    /* Number of fonts in this collection. ==1 if not a collection */
    int directoryCount = 1;

    /* offset in file of table directory for this font */
    int directoryOffset; // 12 if its not a collection.

    /* number of table entries in the directory/offsets table */
    int numTables;
    
    /* The contents of the the directory/offsets table */ 
    DirectoryEntry []tableDirectory;

//     protected byte []gposTable = null;
//     protected byte []gdefTable = null;
//     protected byte []gsubTable = null;
//     protected byte []mortTable = null;
//     protected boolean hintsTabledChecked = false;
//     protected boolean containsHintsTable = false;

    /**
     * - does basic verification of the file
     * - reads the header table for this font (within a collection)
     * - reads the names (full, family).
     * - determines the style of the font.
     * - initializes the CMAP
     * @throws FontFormatException - if the font can't be opened
     * or fails verification,  or there's no usable cmap
     */
    TrueTypeFont(String platname, Object nativeNames, int fIndex,
		 boolean javaRasterizer)
	throws FontFormatException {
	super(platname, nativeNames);
	useJavaRasterizer = javaRasterizer;
	fontRank = Font2D.TTF_RANK;
	verify();
	init(fIndex);
	Disposer.addObjectRecord(this, disposerRecord);
    }

    /* Enable natives just for fonts picked up from the platform that
     * may have external bitmaps on Solaris. Could do this just for
     * the fonts that are specified in font configuration files which
     * would lighten the burden (think about that).
     * The EBLCTag is used to skip natives for fonts that contain embedded
     * bitmaps as there's no need to use X11 for those fonts.
     * Skip all the latin fonts as they don't need this treatment.
     * Further refine this to fonts that are natively accessible (ie
     * as PCF bitmap fonts on the X11 font path).
     * This method is called when creating the first strike for this font.
     */
    protected void checkUseNatives() {
	checkedNatives = true;
	if (!FontManager.isSolaris || useJavaRasterizer ||
            FontManager.useT2K || nativeNames == null ||
	    getDirectoryEntry(EBLCTag) != null ||
            GraphicsEnvironment.isHeadless()) {
            return; /* useNatives is false */
	} else if (nativeNames instanceof String) {
            String name = (String)nativeNames;
            /* Don't do do this for Latin fonts */
            if (name.indexOf("8859") > 0) {
		return;
            } else if (NativeFont.hasExternalBitmaps(name)) {
		nativeFonts = new NativeFont[1];
		try {
		    nativeFonts[0] = new NativeFont(name, true);
		    /* If reach here we have an non-latin font that has
		     * external bitmaps and we successfully created it.
		     */
		    useNatives = true;
		} catch (FontFormatException e) {
		    nativeFonts = null;
		}
            }
	} else if (nativeNames instanceof String[]) {
	    String[] natNames = (String[])nativeNames;
	    int numNames = natNames.length;
	    boolean externalBitmaps = false;
            for (int nn = 0; nn < numNames; nn++) {
		if (natNames[nn].indexOf("8859") > 0) {
		    return;
		} else if (NativeFont.hasExternalBitmaps(natNames[nn])) {
                    externalBitmaps = true;
                }
            }
            if (!externalBitmaps) { 
               return;
            }
	    useNatives = true;
       	    nativeFonts = new NativeFont[numNames];
            for (int nn = 0; nn < numNames; nn++) {
		try {
		    nativeFonts[nn] = new NativeFont(natNames[nn], true);
	   	} catch (FontFormatException e) {
                    useNatives = false;
		    nativeFonts = null;
		}
            }
	}
	if (useNatives) {
	    glyphToCharMap = new char[getMapper().getNumGlyphs()];
	}
    }


    /* This is intended to be called, and the returned value used,
     * from within a block synchronized on this font object. 
     * ie the channel returned may be nulled out at any time by "close()"
     * unless the caller holds a lock.
     * Deadlock warning: FontManager.addToPool(..) acquires a global lock,
     * which means nested locks may be in effect. 
     */
    private synchronized FileChannel open() throws FontFormatException {
	if (disposerRecord.channel == null) {
	    //System.out.println("open TTF platName="+platName);
	    try {
		RandomAccessFile raf = (RandomAccessFile)
		java.security.AccessController.doPrivileged(
	            new java.security.PrivilegedAction() {
			public Object run() {
			    try {
				return new RandomAccessFile(platName, "r");
			    } catch (FileNotFoundException ffne) {
			    }
			    return null;
		    }
	        });
		disposerRecord.channel = raf.getChannel();
		fileSize = (int)disposerRecord.channel.size();
		FontManager.addToPool(this);
	    } catch (NullPointerException e) {
		close();
		throw new FontFormatException(e.toString());
	    } catch (ClosedChannelException e) {
		/* NIO I/O is interruptible, recurse to retry operation.
		 * The call to channel.size() above can throw this exception.
		 * Clear interrupts before recursing in case NIO didn't.
		 * Note that close() sets disposerRecord.channel to null.
		 */
		Thread.interrupted();
		close();
		open();
	    } catch (IOException e) {
		close();
		throw new FontFormatException(e.toString());
	    }
	}
	return disposerRecord.channel;
    }

    protected synchronized void close() {
	disposerRecord.dispose();
    }
 
    
    int readBlock(ByteBuffer buffer, int offset, int length) {
	int bread = 0;
	try {
	    synchronized (this) {
		if (disposerRecord.channel == null) {
		    open();
		}
		if (offset + length > fileSize) {
		    if (offset >= fileSize) {
			return 0;
		    } else {
			length = fileSize - offset;
		    }
		}
		buffer.clear();
		while (bread != length) {
		    disposerRecord.channel.position(offset+bread);
		    int cnt = disposerRecord.channel.read(buffer);
		    if (cnt == -1) {
			buffer.flip();
			throw new IOException("unexpected EOF" + this);
		    }
		    bread += cnt;
		}
		buffer.flip();

	    }
 	} catch (FontFormatException e) {
	    e.printStackTrace();
	} catch (ClosedChannelException e) {
	    /* NIO I/O is interruptible, recurse to retry operation.
	     * Clear interrupts before recursing in case NIO didn't.
	     */
	    Thread.interrupted();
	    close();
	    return readBlock(buffer, offset, length);
	} catch (IOException e) {
	    e.printStackTrace();
	}
	return bread;
    }
   
    ByteBuffer readBlock(int offset, int length) {

	ByteBuffer buffer = ByteBuffer.allocate(length);
	try {
	    synchronized (this) {
		if (disposerRecord.channel == null) {
		    open();
		}
		if (offset + length > fileSize) {
		    if (offset > fileSize) {
			return null; // assert?
		    } else {
			buffer = ByteBuffer.allocate(fileSize-offset);
		    }
		}
		disposerRecord.channel.position(offset);
		disposerRecord.channel.read(buffer);
		buffer.flip();
	    }
	} catch (FontFormatException e) {
	    return null;
	} catch (ClosedChannelException e) {
	    /* NIO I/O is interruptible, recurse to retry operation.
	     * Clear interrupts before recursing in case NIO didn't.
	     */
	    Thread.interrupted();
	    close();
	    readBlock(buffer, offset, length);
	} catch (IOException e) {
	    return null;
	}
	return buffer;
    }

    /* This is used by native code which can't allocate a direct byte
     * buffer because of bug 4845371. It, and references to it in native
     * code in scalerMethods.c can be removed once that bug is fixed.
     * 4845371 is now fixed but we'll keep this around as it doesn't cost
     * us anything if its never used/called.
     */
    byte[] readBytes(int offset, int length) {
	ByteBuffer buffer = readBlock(offset, length);
	if (buffer.hasArray()) {
	    return buffer.array();
	} else {
	    byte[] bufferBytes = new byte[buffer.limit()];
	    buffer.get(bufferBytes);
	    return bufferBytes;
	}
    }

    private void verify() throws FontFormatException {
	open();	
    }

    /* sizes, in bytes, of TT/TTC header records */
    private static final int TTCHEADERSIZE = 12;
    private static final int DIRECTORYHEADERSIZE = 12;
    private static final int DIRECTORYENTRYSIZE = 16;

    protected void init(int fIndex) throws FontFormatException  {
	int headerOffset = 0;
	ByteBuffer buffer = readBlock(0, TTCHEADERSIZE);
	try {
	    switch (buffer.getInt()) {

	    case ttcfTag:
		buffer.getInt(); // skip TTC version ID
		directoryCount = buffer.getInt();
		if (fIndex >= directoryCount) {
		    throw new FontFormatException("Bad collection index");
		}
		fontIndex = fIndex;
		buffer = readBlock(TTCHEADERSIZE+4*fIndex, 4);
		headerOffset = buffer.getInt();
		break;
		
	    case v1ttTag:
	    case trueTag:
		break;

	    default:
		throw new FontFormatException("Unsupported sfnt " + platName);
	    }

	    /* Now have the offset of this TT font (possibly within a TTC)
	     * After the TT version/scaler type field, is the short
	     * representing the number of tables in the table directory.
	     * The table directory begins at 12 bytes after the header.
	     * Each table entry is 16 bytes long (4 32-bit ints)
	     */
	    buffer = readBlock(headerOffset+4, 2);
	    numTables = buffer.getShort();
	    directoryOffset = headerOffset+DIRECTORYHEADERSIZE;
	    ByteBuffer bbuffer = readBlock(directoryOffset,
					   numTables*DIRECTORYENTRYSIZE);
	    IntBuffer ibuffer = bbuffer.asIntBuffer();
	    DirectoryEntry table;
	    tableDirectory = new DirectoryEntry[numTables];
	    for (int i=0; i<numTables;i++) {
		tableDirectory[i] = table = new DirectoryEntry();
		table.tag   =  ibuffer.get();
		/* checksum */ ibuffer.get();
		table.offset = ibuffer.get();
		table.length = ibuffer.get();
		if (table.offset + table.length > fileSize) {
		    throw new FontFormatException("bad table, tag="+table.tag);
		}
	    }
	    initNames();
	} catch (Exception e) {
	    if (FontManager.logging) {
		FontManager.logger.severe(e.toString());
	    }
	    if (e instanceof FontFormatException) {
		throw (FontFormatException)e;
	    } else {
		throw new FontFormatException(e.toString());
	    }
	}
	if (familyName == null || fullName == null) {
	    throw new FontFormatException("Font name not found");
	}
	setStyle();
    }

    /* The array index corresponds to a bit offset in the TrueType
     * font's OS/2 compatibility table's code page ranges fields.
     * These are two 32 bit unsigned int fields at offsets 78 and 82.
     * We are only interested in determining if the font supports
     * the windows encodings we expect as the default encoding in
     * supported locales, so we only map the first of these fields.
     */
    static final String encoding_mapping[] = {
	"cp1252",    /*  0:Latin 1  */
	"cp1250",    /*  1:Latin 2  */
	"cp1251",    /*  2:Cyrillic */
	"cp1253",    /*  3:Greek    */
	"cp1254",    /*  4:Turkish/Latin 5  */
	"cp1255",    /*  5:Hebrew   */
	"cp1256",    /*  6:Arabic   */
	"cp1257",    /*  7:Windows Baltic   */
	"",          /*  8:reserved for alternate ANSI */
	"",          /*  9:reserved for alternate ANSI */
	"",          /* 10:reserved for alternate ANSI */
	"",          /* 11:reserved for alternate ANSI */
	"",          /* 12:reserved for alternate ANSI */
	"",          /* 13:reserved for alternate ANSI */
	"",          /* 14:reserved for alternate ANSI */
	"",          /* 15:reserved for alternate ANSI */
	"ms874",     /* 16:Thai     */
	"ms932",     /* 17:JIS/Japanese */
	"gbk",       /* 18:PRC GBK Cp950  */
	"ms949",     /* 19:Korean Extended Wansung */
	"ms950",     /* 20:Chinese (Taiwan, Hongkong, Macau) */
	"ms1361",    /* 21:Korean Johab */
	"",          /* 22 */
	"",          /* 23 */
	"",          /* 24 */
	"",          /* 25 */
	"",          /* 26 */
	"",          /* 27 */
	"",          /* 28 */
	"",          /* 29 */
	"",          /* 30 */
	"",          /* 31 */
    };

    /* reserved bits must not be set, include symbol bits */
    public static final int reserved_bits1 = 0x80000000;
    public static final int reserved_bits2 = 0x0000ffff;

    boolean supportsEncoding(String encoding) {

	ByteBuffer buffer = getTableBuffer(os_2Tag);
	/* required info is at offsets 78 and 82 */
	if (buffer == null || buffer.capacity() < 86) {
	    return false;
	}

	int range1 = buffer.getInt(78); /* ulCodePageRange1 */
	int range2 = buffer.getInt(82); /* ulCodePageRange2 */

	if (((range1 & reserved_bits1) | (range2 & reserved_bits2)) != 0) {
	    return false;
	}

	for (int em=0; em<encoding_mapping.length; em++) {
	    if (encoding_mapping[em].equals(encoding)) {
		if (((1 << em) & range1) != 0) {
		    return true;
		}
	    }
	}
	return false;
    }

    /* This should be generalised, but for now just need to know if
     * Hiragana or Katakana ranges are supported by the font.
     * In the 4 longs representing unicode ranges supported
     * bits 49 & 50 indicate hiragana and katakana
     * This is bits 17 & 18 in the 2nd ulong. If either is supported
     * we presume this is a JA font.
     */
     boolean supportsJA() {

	ByteBuffer buffer = getTableBuffer(os_2Tag);
	/* required info is in ulong at offset 46 */
	if (buffer == null || buffer.capacity() < 50) {
	    return false;
	}
	int range2 = buffer.getInt(46); /* ulUnicodeRange2 */
	return ((range2 & 0x60000) != 0);
    }

//     TableInfo getTableInfo(int tag) {
// 	if (tag == headTag) {
// 	    return headerTableInfo;
// 	} else {
// 	    return null;
// 	}
//     }

     ByteBuffer getTableBuffer(int tag) {
        DirectoryEntry entry = null;

	for (int i=0;i<numTables;i++) {
	    if (tableDirectory[i].tag == tag) {
		entry = tableDirectory[i];
		break;
	    }
	}
	if (entry == null || entry.length == 0 ||
	    entry.offset+entry.length > fileSize) {
	    return null;
	}

	int bread = 0;
	ByteBuffer buffer = ByteBuffer.allocate(entry.length);
	synchronized (this) {
	    try {
		if (disposerRecord.channel == null) {
		    open();
		}
		disposerRecord.channel.position(entry.offset);
		bread = disposerRecord.channel.read(buffer);
		buffer.flip();
	    } catch (ClosedChannelException e) {
		/* NIO I/O is interruptible, recurse to retry operation.
		 * Clear interrupts before recursing in case NIO didn't.
		 */
		Thread.interrupted();
		return getTableBuffer(tag);
	    } catch (IOException e) {
		return null;
	    } catch (FontFormatException e) {
		return null;
	    }

	    if (bread < entry.length) {
		return null;
	    } else {
		return buffer;
	    }
	}
    }

    byte[] getTableBytes(int tag) {
	ByteBuffer buffer = getTableBuffer(tag);
	if (buffer == null) {
	    return null;
	} else if (buffer.hasArray()) {
	    try {
		return buffer.array();
	    } catch (Exception re) {
	    }
	}
	byte []data = new byte[getTableSize(tag)];
	buffer.get(data);
	return data;
    }

    int getTableSize(int tag) {
	for (int i=0;i<numTables;i++) {
	    if (tableDirectory[i].tag == tag) {
		return tableDirectory[i].length;
	    }
	}
	return 0;
    }

    int getTableOffset(int tag) {
	for (int i=0;i<numTables;i++) {
	    if (tableDirectory[i].tag == tag) {
		return tableDirectory[i].offset;
	    }
	}
	return 0;
    }

    DirectoryEntry getDirectoryEntry(int tag) {
	for (int i=0;i<numTables;i++) {
	    if (tableDirectory[i].tag == tag) {
		return tableDirectory[i];
	    }
	}
	return null;
    }

    public String getFullName() {
	return fullName;
    }

    /* TrueTypeFont can use the fsSelection fields of OS/2 table
     * to determine the style. In the unlikely case that doesn't exist,
     * can use macStyle in the 'head' table but simpler to
     * fall back to super class algorithm of looking for well known string.
     * A very few fonts don't specify this information, but I only
     * came across one: Lucida Sans Thai Typewriter Oblique in
     * /usr/openwin/lib/locale/th_TH/X11/fonts/TrueType/lucidai.ttf
     * that explicitly specified the wrong value. It says its regular.
     * I didn't find any fonts that were inconsistent (ie regular plus some
     * other value).
     */
    private static final int fsSelectionItalicBit  = 0x00001;
    private static final int fsSelectionBoldBit    = 0x00020;
    private static final int fsSelectionRegularBit = 0x00040;
    protected void setStyle() {
	ByteBuffer os_2Table = getTableBuffer(os_2Tag);
	/* fsSelection is unsigned short at buffer offset 62 */
	if (os_2Table == null || os_2Table.capacity() < 64) {
	    super.setStyle();
	    return;
	}
	int fsSelection = os_2Table.getChar(62) & 0xffff;
	int italic  = fsSelection & fsSelectionItalicBit;
	int bold    = fsSelection & fsSelectionBoldBit;
	int regular = fsSelection & fsSelectionRegularBit;
// 	System.out.println("platname="+platName+" font="+fullName+
// 			   " family="+familyName+
// 			   " R="+regular+" I="+italic+" B="+bold);
	if (regular!=0 && ((italic|bold)!=0)) {
	    /* This is inconsistent. Try using the font name algorithm */
	    super.setStyle();
	    return;
	} else if ((regular|italic|bold) == 0) {
	    /* No style specified. Try using the font name algorithm */
	    super.setStyle();
	    return;
	}
	switch (bold|italic) {
	case fsSelectionItalicBit:
	    style = Font.ITALIC;
	    break;
	case fsSelectionBoldBit:
	    if (FontManager.isSolaris && platName.endsWith("HG-GothicB.ttf")) {
		/* Workaround for Solaris's use of a JA font that's marked as
		 * being designed bold, but is used as a PLAIN font.
		 */
		style = Font.PLAIN;
	    } else {
		style = Font.BOLD;
	    }
	    break;
	case fsSelectionBoldBit|fsSelectionItalicBit:
	    style = Font.BOLD|Font.ITALIC;
	}
    }
    
    private String makeString(byte[] bytes, short len, short encoding) {

	/* Check for fonts using encodings 2->6 is just for
	 * some old DBCS fonts, apparently mostly on Solaris.
         * Some of these fonts encode ascii names as double-byte characters.
         * ie with a leading zero byte for what properly should be a
         * single byte-char.
         */
        if (encoding >=2 && encoding <= 6) {
             byte[] oldbytes = bytes;
             int oldlen = len;
             bytes = new byte[oldlen];
             len = 0;
             for (int i=0; i<oldlen; i++) {
                 if (oldbytes[i] != 0) { 
                     bytes[len++] = oldbytes[i];
                 }
             }
         }

	String charset;
	switch (encoding) {
	    case 1:  charset = "UTF-16";  break; // most common case first.
	    case 0:  charset = "UTF-16";  break; // symbol uses this
	    case 2:  charset = "SJIS";    break;
	    case 3:  charset = "GBK";     break;
	    case 4:  charset = "MS950";   break;
	    case 5:  charset = "EUC_KR";  break;
	    case 6:  charset = "Johab";   break;
	    default: charset = "UTF-16";  break;
        }

	try {
	    return new String(bytes, 0, len, charset);
	} catch (UnsupportedEncodingException e) {
	    if (FontManager.logging) {
		FontManager.logger.warning(e + " EncodingID=" + encoding);
	    }
	    return new String(bytes, 0, len);
	} catch (Throwable t) {
	    return null;
	}
    }

    protected void initNames() {

        byte[] name = new byte[256];
	ByteBuffer buffer = getTableBuffer(nameTag);

	if (buffer != null) {
	    ShortBuffer sbuffer = buffer.asShortBuffer();
	    sbuffer.get(); // format - not needed.
	    short numRecords = sbuffer.get();
	    short stringPtr = sbuffer.get();
	    for (int i=0; i<numRecords; i++) {
		short platformID = sbuffer.get();
		if (platformID != MS_PLATFORM_ID) {
		    sbuffer.position(sbuffer.position()+5);
		    continue; // skip over this record.
		}		    
		short encodingID = sbuffer.get();
		short langID     = sbuffer.get();
		short nameID     = sbuffer.get();
		short nameLen    = sbuffer.get();
		short namePtr    = (short)(sbuffer.get() + stringPtr);

		switch (nameID) {

		case FAMILY_NAME_ID:

		    if (familyName == null || langID == ENGLISH_LOCALE_ID) {
			buffer.position(namePtr);
			buffer.get(name, 0, nameLen);
			familyName = makeString(name, nameLen, encodingID);
		    }
/*
		    for (int ii=0;ii<nameLen;ii++) {
			int val = (int)name[ii]&0xff;
			System.err.print(Integer.toHexString(val)+ " ");
		    }
		    System.err.println();
		    System.err.println("familyName="+familyName +
				       " nameLen="+nameLen+
				       " langID="+langID+ " eid="+encodingID +
				       " str len="+familyName.length());

*/
		    break;
	        
		case FULL_NAME_ID:

		    if (fullName == null || langID == ENGLISH_LOCALE_ID) {
			buffer.position(namePtr);
			buffer.get(name, 0, nameLen);
			fullName = makeString(name, nameLen, encodingID);
		    }
		    break;

		}
	    }
	}
    }

    /* Return the requested name in the requested locale, for the
     * MS platform ID. If the requested locale isn't found, return US
     * English, if that isn't found, return null and let the caller
     * figure out how to handle that.
     */
    protected String lookupName(short findLocaleID, int findNameID) {
	String foundName = null;
	byte[] name = new byte[1024];

	ByteBuffer buffer = getTableBuffer(nameTag);
	if (buffer != null) {
	    ShortBuffer sbuffer = buffer.asShortBuffer();
	    sbuffer.get(); // format - not needed.
	    short numRecords = sbuffer.get();
	    short stringPtr = sbuffer.get();
	    
	    for (int i=0; i<numRecords; i++) {
		short platformID = sbuffer.get();
		if (platformID != MS_PLATFORM_ID) {
		    sbuffer.position(sbuffer.position()+5);
		    continue; // skip over this record.
		}
		short encodingID = sbuffer.get();
		short langID     = sbuffer.get();
		short nameID     = sbuffer.get();
		short nameLen    = sbuffer.get();
		short namePtr    = (short)(sbuffer.get() + stringPtr);
		if (nameID == findNameID &&
		    ((foundName == null && langID == ENGLISH_LOCALE_ID)
		     || langID == findLocaleID)) {
		    buffer.position(namePtr);
		    buffer.get(name, 0, nameLen);
		    foundName = makeString(name, nameLen, encodingID);
		    if (langID == findLocaleID) {
			return foundName;
		    }
		}
	    }
	}
	return foundName;
    }

    /**
     * @return number of logical fonts. Is "1" for all but TTC files
     */
    public int getFontCount() {
	return directoryCount;
    }

    private native long createScaler(int fileSize, int fontIndex);

    protected synchronized long getScaler() {
	if (pScaler == 0L) {
	    pScaler = createScaler(fileSize, fontIndex);
	    if (pScaler != 0L) {
		Disposer.addObjectRecord(this, new FileFontDisposer(pScaler));
	    } else {
		pScaler = getNullScaler();
		FontManager.deRegisterBadFont(this);
	    }
	}
	return pScaler;
    }


    /* Postscript name is rarely requested. Don't waste cycles locating it
     * as part of font creation, nor storage to hold it. Get it only on demand.
     */
    public String getPostscriptName() {
	String name = lookupName(ENGLISH_LOCALE_ID, POSTSCRIPT_NAME_ID);
	if (name == null) {
	    return fullName;
	} else {
	    return name;
	}
    }

    public String getFontName(Locale locale) {
	if (locale == null) {
	    return fullName;
	} else {
	    short localeID = FontManager.getLCIDFromLocale(locale);
	    String name = lookupName(localeID, FULL_NAME_ID);
	    if (name == null) {
		return fullName;
	    } else {
		return name;
	    }
	}
    }

    public String getFamilyName(Locale locale) {
	if (locale == null) {
	    return familyName;
	} else {
	    short localeID = FontManager.getLCIDFromLocale(locale);
	    String name = lookupName(localeID, FAMILY_NAME_ID);
	    if (name == null) {
		return familyName;
	    } else {
		return name;
	    }
	}
    }

    public CharToGlyphMapper getMapper() {
	if (mapper == null) {
	    mapper = new TrueTypeGlyphMapper(this);
	}
	return mapper;
    }

    /* This duplicates initNames() but that has to run fast as its used
     * during typical start-up and the information here is likely never
     * needed.
     */
    protected void initAllNames(int requestedID, HashSet names) {

        byte[] name = new byte[256];
	ByteBuffer buffer = getTableBuffer(nameTag);

	if (buffer != null) {
	    ShortBuffer sbuffer = buffer.asShortBuffer();
	    sbuffer.get(); // format - not needed.
	    short numRecords = sbuffer.get();
	    short stringPtr = sbuffer.get();
	    for (int i=0; i<numRecords; i++) {
		short platformID = sbuffer.get();
		if (platformID != MS_PLATFORM_ID) {
		    sbuffer.position(sbuffer.position()+5);
		    continue; // skip over this record.
		}		    
		short encodingID = sbuffer.get();
		short langID     = sbuffer.get();
		short nameID     = sbuffer.get();
		short nameLen    = sbuffer.get();
		short namePtr    = (short)(sbuffer.get() + stringPtr);

		if (nameID == requestedID) {
		    buffer.position(namePtr);
		    buffer.get(name, 0, nameLen);
		    names.add(makeString(name, nameLen, encodingID));
		}
	    }
	}
    }

    String[] getAllFamilyNames() {
	HashSet aSet = new HashSet();
	try {
	    initAllNames(FAMILY_NAME_ID, aSet);
	} catch (Exception e) {
	    /* In case of malformed font */
	}
	return (String[])aSet.toArray(new String[0]);
    }

    String[] getAllFullNames() {
	HashSet aSet = new HashSet();
	try {
	    initAllNames(FULL_NAME_ID, aSet);
	} catch (Exception e) {
	    /* In case of malformed font */
	}
	return (String[])aSet.toArray(new String[0]);
    }

    /*  Used by the OpenType engine for mark positioning.
     */
    native synchronized Point2D.Float getGlyphPoint(long pScalerContext,
						    int glyphCode,
						    int ptNumber);

    public String toString() {
	return "** TrueType Font: Family="+familyName+ " Name="+fullName+
	    " style="+style+" fileName="+platName;
    }
}
