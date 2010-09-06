/*
 * @(#)JPEGImageEncoderImpl.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) 1997-1998 Eastman Kodak Company.                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

package sun.awt.image.codec;

import java.io.OutputStream;
import java.io.IOException;
import java.awt.Point;
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.IndexColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.DataBufferInt;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.ComponentSampleModel;
import java.awt.image.SinglePixelPackedSampleModel;
import java.awt.image.WritableRaster;
import java.awt.image.RescaleOp;
import com.sun.image.codec.jpeg.*;

/** 

 * JPEGImageEncoderImpl implements the JPEGImageEncoder interface.  It
 * will encode buffers of image data into JPEG data streams.
 * Essentially users of this class are required to provide image data
 * in a Raster object or a BufferedImage, set the necessary parameters
 * in the JPEGEncodeParams object and successfully open the
 * <code>OutputStream</code> that is the destination of the encoded
 * jpeg stream.
 *
 * The JPEGImageEncoder class will encode Rasters of image data into
 * interchange, and abbreviated JPEG data streams that are written to the
 * OutputStream.
 * 
 * @see		JPEGEncodeParam
 * @see		Raster
 * @see		BufferedImage
 * @see		OutputStream
 */

public class JPEGImageEncoderImpl implements JPEGImageEncoder {

	/** Object that contains the details of encoding data into jpeg */
	private  OutputStream		outStream = null;
	private  JPEGParam		param     = null;
	private  boolean		pack      = false;

	private static final Class OutputStreamClass = OutputStream.class;

	/** 
	 * static method to load the JPEG lib when the class is
	 * constructed.
	 */
	static { 
	    java.security.AccessController.doPrivileged(
		new sun.security.action.LoadLibraryAction("jpeg"));
	}
	
	
	/** 
	 * Constructs a JPEGImageEncoder using a destination OutputStream.
	 * @param dest - destination of the encoded data.
	 */
	public JPEGImageEncoderImpl(OutputStream dest) {
		if (dest == null) 
			throw new IllegalArgumentException("OutputStream is null.");

		outStream = dest;
		initEncoder(OutputStreamClass);
	}
	
	/** 
	 * Constructs a JPEGImageEncoder using a destination OutputStream.
	 * @param dest - destination of the encoded data.
	 * @param jep The JPEGEncodeParam object to use for encoding.
	 */
	public JPEGImageEncoderImpl(OutputStream dest,
								JPEGEncodeParam jep) {
		this(dest);
		setJPEGEncodeParam(jep);
	}
	
	/** 
	 * Returns the 'default' encoded COLOR_ID for a given ColorModel.
	 * This method is not needed in the simple case of encoding
	 * Buffered Images (the library will figure things out for you.
	 * It can be useful however for encoding Rasters.  To determine
	 * what needs to be done to the image prior to encoding.
	 
	 * @param cm The ColorModel to map to an jpeg encoded COLOR_ID.
	 * @return The default mapping of cm to a jpeg Color_ID note that
	 * in a few cases color conversion is required.
	 */
	public int getDefaultColorId(ColorModel cm) {
		boolean alpha = cm.hasAlpha();
		ColorSpace cs = cm.getColorSpace();
		ColorSpace csPYCC = null;
		switch (cs.getType()) {
		case ColorSpace.TYPE_GRAY:
			return JPEGEncodeParam.COLOR_ID_GRAY;

		case ColorSpace.TYPE_RGB:
			if (alpha)
				return JPEGEncodeParam.COLOR_ID_YCbCrA;
			else 
				return JPEGEncodeParam.COLOR_ID_YCbCr;
			
		case ColorSpace.TYPE_YCbCr:
			if (csPYCC == null) {
				try {
					csPYCC = ColorSpace.getInstance(
						    ColorSpace.CS_PYCC);
				} catch (IllegalArgumentException e) {
					// catch the case when PYCC.pf is not
					// installed (default download JRE)
				}
			}
			if (cs == csPYCC)
				return (alpha? 
						JPEGEncodeParam.COLOR_ID_PYCCA : 
						JPEGEncodeParam.COLOR_ID_PYCC);
			else
				return (alpha? 
						JPEGEncodeParam.COLOR_ID_YCbCrA : 
						JPEGEncodeParam.COLOR_ID_YCbCr);
			
		case ColorSpace.TYPE_CMYK:
			return JPEGEncodeParam.COLOR_ID_CMYK;
		default:
			return JPEGEncodeParam.COLOR_ID_UNKNOWN;
		}
	}

	public synchronized OutputStream getOutputStream() {
		return outStream;
	}

	public synchronized void setJPEGEncodeParam(JPEGEncodeParam jep)
	{ param = new JPEGParam(jep); }

	public synchronized JPEGEncodeParam getJPEGEncodeParam()
	{ return (JPEGEncodeParam)param.clone(); }

	public JPEGEncodeParam getDefaultJPEGEncodeParam(Raster ras, int colorID)
	{
		JPEGParam ret = new JPEGParam(colorID, ras.getNumBands());
		ret.setWidth(ras.getWidth());
		ret.setHeight(ras.getHeight());

		return ret;
	}

	public JPEGEncodeParam getDefaultJPEGEncodeParam(BufferedImage bi)
	{
                ColorModel cm      = bi.getColorModel();
                int        colorID = getDefaultColorId(cm);

                if (!(cm instanceof IndexColorModel))
                    return getDefaultJPEGEncodeParam(bi.getRaster(), colorID);

                // Special case we know about ... in encode we will expand
                // data to 3 or 4 bands...
                JPEGParam ret;
                if (cm.hasAlpha()) ret = new JPEGParam(colorID, 4);
                else               ret = new JPEGParam(colorID, 3);

                ret.setWidth (bi.getWidth());
                ret.setHeight(bi.getHeight());
                return ret;
	}

        public JPEGEncodeParam getDefaultJPEGEncodeParam(int numBands, int colorID)
        {
        return new JPEGParam(colorID, numBands);
        }

        public JPEGEncodeParam getDefaultJPEGEncodeParam(JPEGDecodeParam jdp)
            throws ImageFormatException {
            return new JPEGParam(jdp);
        }

	public synchronized void encode(BufferedImage bi)
		throws IOException, ImageFormatException 
	{ 
		if (param == null)
			setJPEGEncodeParam(getDefaultJPEGEncodeParam(bi));

		if ((bi.getWidth() != param.getWidth()) ||
			(bi.getHeight() != param.getHeight()))
			throw new ImageFormatException
				("Param block's width/height doesn't match the BufferedImage");

		if (param.getEncodedColorID() != 
			getDefaultColorId(bi.getColorModel()))
			throw new ImageFormatException
				("The encoded COLOR_ID doesn't match the BufferedImage");
		
		Raster     dataSrc = bi.getRaster();
		ColorModel cm      = bi.getColorModel();

                // For index colormodel we need to expand the pallete.
                if (cm instanceof IndexColorModel) {
                        IndexColorModel icm = (IndexColorModel)cm;
                        bi = icm.convertToIntDiscrete(dataSrc, false);
                        dataSrc = bi.getRaster();
                        cm      = bi.getColorModel();
                }
		
		encode(dataSrc, cm);
	}

	public synchronized void encode(BufferedImage bi, JPEGEncodeParam jep) 
		throws IOException, ImageFormatException 
	{
		setJPEGEncodeParam(jep);
		encode(bi);
	}
	
	public void encode(Raster ras) 
		throws IOException, ImageFormatException 
	{
		if (param == null)
			setJPEGEncodeParam(getDefaultJPEGEncodeParam
							   (ras, JPEGEncodeParam.COLOR_ID_UNKNOWN));

		if ((ras.getWidth() != param.getWidth()) ||
			(ras.getHeight() != param.getHeight()))
			throw new ImageFormatException
				("Param block's width/height doesn't match the Raster");
		
		if ((param.getEncodedColorID() != JPEGEncodeParam.COLOR_ID_UNKNOWN) &&
			(param.getNumComponents()  != ras.getNumBands()))
			throw new ImageFormatException
				("Param block's COLOR_ID doesn't match the Raster.");
		
		encode(ras, (ColorModel)null);
	}

	public void encode(Raster ras, JPEGEncodeParam jep) 
		throws IOException, ImageFormatException 
	{
		setJPEGEncodeParam(jep);
		encode(ras);
	}

	private boolean useGiven(Raster ras) {
		SampleModel sm = ras.getSampleModel();
		if (sm.getDataType() != DataBuffer.TYPE_BYTE)
			return false;

		if (!(sm instanceof ComponentSampleModel))
			return false;

		ComponentSampleModel csm = (ComponentSampleModel)sm;
		if (csm.getPixelStride() != sm.getNumBands())
			return false;
		int []offsets = csm.getBandOffsets();
		for (int i=0; i<offsets.length; i++)
			if (offsets[i] != i) return false;

		return true;
	}

	private boolean canPack(Raster ras) {
		// if (true) return false;

		SampleModel sm = ras.getSampleModel();
		if (sm.getDataType() != DataBuffer.TYPE_INT)
			return false;

		if (!(sm instanceof SinglePixelPackedSampleModel))
			return false;

		SinglePixelPackedSampleModel sppsm = 
			(SinglePixelPackedSampleModel)sm;

		int [] need  = { 0xFF0000, 0xFF00, 0xFF, 0xFF000000 };
		int [] masks = sppsm.getBitMasks();
		if ((masks.length != 3) && (masks.length != 4)) 
			return false;
			
		for (int i=0; i<masks.length; i++)
			if (masks[i] != need[i]) return false;

		return true;
	}

	private void encode(Raster ras, ColorModel cm)
		throws IOException, ImageFormatException 
	{
          SampleModel sm = ras.getSampleModel();
          int i;
          int numBands = sm.getNumBands();
          if (cm == null) {
            /* If cm is null, this is a Raster (not a BufferedImage) being
               encoded.  In this case, we don't rescale the samples, so they
               must fit in 8 bits.
             */
            for (i = 0; i < numBands; i++) {
              if (sm.getSampleSize(i) > 8) {
                throw new ImageFormatException
                  ("JPEG encoder can only accept 8 bit data.");
              }
            }
          }

          int colorID = param.getEncodedColorID();
          switch (param.getNumComponents())
            {
              case 1:
		if ((colorID != JPEGEncodeParam.COLOR_ID_GRAY) &&
		    (colorID != JPEGEncodeParam.COLOR_ID_UNKNOWN) &&
		    (param.findAPP0() != null))
                  throw new ImageFormatException(
                    "1 band JFIF files imply Y or unknown encoding.\n" +
                    "Param block indicates alternate encoding.");
                break;
              case 3:
                if ((colorID != JPEGEncodeParam.COLOR_ID_YCbCr) &&
                    (param.findAPP0() != null))
                  throw new ImageFormatException(
                    "3 band JFIF files imply YCbCr encoding.\n" +
                    "Param block indicates alternate encoding.");
                break;
              case 4:
                if ((colorID != JPEGEncodeParam.COLOR_ID_CMYK) &&
                    (param.findAPP0() != null))
                  throw new ImageFormatException(
                    "4 band JFIF files imply CMYK encoding.\n" +
                    "Param block indicates alternate encoding.");
                break;

              default:
                break;
            }

		if (!param.isImageInfoValid()) {
			// Only writing tables no reason to reformat Ras...
			writeJPEGStream(param, cm, outStream, null, 0, 0);
			return;
		}

		DataBuffer  db = ras.getDataBuffer();

		Object      data;
		int         line, start;
                boolean premult = false;
                boolean cmok = true;
                int[] cmbits = null;

                if (cm != null) {
                    if (cm.hasAlpha() && cm.isAlphaPremultiplied()) {
                        premult = true;
                        cmok = false;
                    }
                    cmbits = cm.getComponentSize();
                    for (i = 0; i < numBands; i++) {
                        if (cmbits[i] != 8) {
                            cmok = false;
                        }
                    }
                }

		pack  = false;
		if (cmok && useGiven(ras)) {
			ComponentSampleModel csm =(ComponentSampleModel)sm;

			start = (int)(db.getOffset()+
				csm.getOffset(
                                ras.getMinX() - ras.getSampleModelTranslateX(),
                                ras.getMinY() - ras.getSampleModelTranslateY()
                                ));
			line  = (int)csm.getScanlineStride();
			data  = ((DataBufferByte)db).getData();

		} else if (cmok && canPack(ras)) {
			SinglePixelPackedSampleModel sppsm;
			sppsm = (SinglePixelPackedSampleModel)sm;

			start = (int)(db.getOffset()+
				sppsm.getOffset(
                                ras.getMinX() - ras.getSampleModelTranslateX(),
                                ras.getMinY() - ras.getSampleModelTranslateY()
                                ));
			line  = (int)sppsm.getScanlineStride();
			data  = ((DataBufferInt)db).getData();
			pack  = true;
		} else {
			ComponentSampleModel csm;
			int[] offsets = new int[numBands];
			float[] rsfactors = new float[numBands];
			for (i=0; i<numBands; i++) {
				offsets[i] = i;
				if (cmok) {
                                    continue;
                                }
				if (cmbits[i] != 8) {
                                        rsfactors[i] = 255.0f /
                                            ((float) ((1 << cmbits[i]) - 1));
				} else {
					rsfactors[i] = 1.0f;
				}
			}
			csm = new ComponentSampleModel(DataBuffer.TYPE_BYTE,
			    ras.getWidth(), ras.getHeight(), numBands,
			    numBands*ras.getWidth(), offsets);
			WritableRaster wr;
			wr = Raster.createWritableRaster(csm,
			    new Point(ras.getMinX(), ras.getMinY()));

                        if (cmok) {
                            wr.setRect(ras); /* REMIND: This assumes that 8-bit
                                                samples are in LSB's of src.
                                                If a non-std ColorModel is used
                                                or we relax the rules on our
                                                std ColorModels, then more
                                                checking must be done to set
                                                cmok true.
                                              */
                        } else {
			    float[] rsoffsets =
                                new float[numBands]; /* all zeroes */
			    RescaleOp rsop = new RescaleOp(rsfactors, rsoffsets,
						           null);
			    rsop.filter(ras, wr);
                            if (premult) {
                                int[] bits = new int[numBands];
                                for (i = 0; i < numBands; i++) {
                                    bits[i] = 8;
                                }
                                ComponentColorModel ccm =
                                    new ComponentColorModel(
                                        cm.getColorSpace(), bits, true, true,
                                        Transparency.TRANSLUCENT,
                                        DataBuffer.TYPE_BYTE);
                                ccm.coerceData(wr, false);
                            }
                        }
			
			db    = wr.getDataBuffer();
			data  = ((DataBufferByte)db).getData();
                            /* Note that:
                            wr.getMinX() - wr.getSampleModelTranslateX() = 0
                            wr.getMinY() - wr.getSampleModelTranslateY() = 0
                            since we created a new SampleModel just the right
                            size for this raster.
                            */
			start = (int)(db.getOffset()+ csm.getOffset(0, 0));
			line  = (int)csm.getScanlineStride();
		}
		
		
		writeJPEGStream(param, cm, outStream, data, start, line);
	}

	/** 
	 * Returns the 'default' encoded COLOR_ID for a given ColorModel.
	 * This method is not needed in the simple case of encoding
	 * Buffered Images (the library will figure things out for you.
	 * It can be useful however for encoding Rasters.  To determine
	 * what needs to be done to the image prior to encoding.
	 
	 * @param cm The ColorModel to map to an jpeg encoded COLOR_ID.
	 * @return The default mapping of cm to a jpeg Color_ID note that
	 * in a few cases color conversion is required.
	 */
	private int getNearestColorId(ColorModel cm) {
		ColorSpace cs = cm.getColorSpace();
		switch (cs.getType()) {
		case ColorSpace.TYPE_RGB: {
			if (cm.hasAlpha()) return JPEGEncodeParam.COLOR_ID_RGBA;
			else               return JPEGEncodeParam.COLOR_ID_RGB;
		}
		default:
			return getDefaultColorId(cm);
		}
	}

	// native method that will initialize the method id's for all
	// calls back to java.
	private native void initEncoder( Class OutputStreamClass);

	/** 
	 * Native method that is the only interface to the JPEG library
	 * that is loaded when the class is invoked.
	 */
	private synchronized native void writeJPEGStream
		(JPEGEncodeParam p, ColorModel cm, OutputStream dest,
		 Object data, int start, int line) 
		throws IOException, ImageFormatException;
}
