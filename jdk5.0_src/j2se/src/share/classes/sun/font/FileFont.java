/*
 * @(#)FileFont.java	1.3 12/19/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.FontFormatException;
import java.awt.geom.GeneralPath;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.File;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import sun.java2d.Disposer;
import sun.java2d.DisposerRecord;

public abstract class FileFont extends PhysicalFont {

    protected boolean useJavaRasterizer = true;
   
    /* I/O and file operations are always synchronized on the font
     * object. Two threads can be accessing the font and retrieving
     * information, and synchronized only to the extent that filesystem
     * operations require.
     * A limited number of files can be open at a time, to limit the
     * absorption of file descriptors. If a file needs to be opened
     * when there are none free, then the synchronization of all I/O
     * ensures that any in progress operation will complete before some
     * other thread closes the descriptor in order to allocate another one.
     */
    // NB consider using a RAF. FIS has finalize method so may take a
    // little longer to be GC'd. We don't use this stream at all anyway.
    // In fact why increase the size of a FileFont object if the stream
    // isn't needed ..
    //protected FileInputStream stream;
    //protected FileChannel channel;
    protected int fileSize;

    protected FileFontDisposer disposer;

    protected long pScaler;

    /* The following variables are used, (and in the case of the arrays, 
     * only initialised) for select fonts where a native scaler may be
     * used to get glyph images and metrics.
     * glyphToCharMap is filled in on the fly and used to do a reverse
     * lookup when a FileFont needs to get the charcode back from a glyph
     * code so it can re-map via a NativeGlyphMapper to get a native glyph.
     * This isn't a big hit in time, since a boolean test is sufficient
     * to choose the usual default path, nor in memory for fonts which take
     * the native path, since fonts have contiguous zero-based glyph indexes,
     * and these obviously do all exist in the font.
     */
    protected boolean checkedNatives;
    protected boolean useNatives;
    protected NativeFont[] nativeFonts;
    protected char[] glyphToCharMap;
    /*
     * @throws FontFormatException - if the font can't be opened
     */
    FileFont(String platname, Object nativeNames)
	throws FontFormatException {

	super(platname, nativeNames);
    }
    
    FontStrike createStrike(FontStrikeDesc desc) {
        if (!checkedNatives) {
           checkUseNatives();
        }
        return new FileFontStrike(this, desc);
    }

    protected void checkUseNatives() {
        checkedNatives = true;
    }

    /* This method needs to be accessible to FontManager if there is
     * file pool management. It may be a no-op.
     */
    protected abstract void close();

    /* 
     * This is the public interface. The subclasses need to implement
     * this. The returned block may be longer than the requested length.
     */
    abstract ByteBuffer readBlock(int offset, int length);

    public boolean canDoStyle(int style) {
	return true;
    }

    void setFileToRemove(File file) {
	Disposer.addObjectRecord(this,
				 new CreatedFontFileDisposerRecord(file));
    }

    /* These methods defined in scalerMethods.c */

    /* freeScaler is called by a disposer on a reference queue */
    static native void freeScaler(long pScaler);

    /* Retrieves a singleton "null" scaler instance which must
     * not be freed.
     */
    static synchronized native long getNullScaler();

    native synchronized	StrikeMetrics getFontMetrics(long pScalerContext);

    native synchronized float getGlyphAdvance(long pScalerContext,
					      int glyphCode);

    native synchronized void getGlyphMetrics(long pScalerContext,
					     int glyphCode,
					     Point2D.Float metrics);

    native synchronized long getGlyphImage(long pScalerContext,
					   int glyphCode);

    /* These methods defined in t2kscalerMethods.cpp */
    native synchronized Rectangle2D.Float getGlyphOutlineBounds(long pContext,
								int glyphCode);

    native synchronized GeneralPath getGlyphOutline(long pScalerContext,
						    int glyphCode,
						    float x, float y);

    native synchronized	GeneralPath getGlyphVectorOutline(long pScalerContext,
							  int[] glyphs,
							  int numGlyphs,
							  float x, float y);

    /* T1 & TT implementation differ so this method is abstract */
    protected abstract long getScaler();

//     protected synchronized void freeScaler() {
// 	if (pScaler != 0L) {
// 	    freeScaler(pScaler);
// 	    pScaler = 0L;
// 	}
//     }

    protected class FileFontDisposer extends DisposerRecord {

	long pScaler = 0L;
	boolean disposed = false;

	public FileFontDisposer(long pScaler) {
	    this.pScaler = pScaler;
	}

	public synchronized void dispose() {
	    if (!disposed) {
		FileFont.freeScaler(pScaler);
		pScaler = 0L;
		disposed = true;
	    }
	}
    }

    private class CreatedFontFileDisposerRecord extends DisposerRecord {
	
	File fontFile = null;

	private CreatedFontFileDisposerRecord(File file) {
	    fontFile = file;
	}

	public void dispose() {
	    java.security.AccessController.doPrivileged(
	         new java.security.PrivilegedAction() {
	              public Object run() {
			  if (fontFile != null) {
			      try {
				  /* REMIND: is it possible that the file is
				   * still open? It will be closed when the
				   * font2D is disposed but could this code
				   * execute first? If so the file would not
				   * be deleted on MS-windows.
				   */
				  fontFile.delete();
			      } catch (Exception e) {
			      }
			  }
			  return null;
		      }
	    }); 
	}
    }
}


   


    

    
