/*
 * @(#)JPEGImageDecoderImpl.java	1.9 03/12/19
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

/**
 * Class JPEGImageDecoder 
 * 
 * JPEGImageDecoder decompresses an JPEG InputStreams into a Raster
 * or a BufferedImage depending upon the method invoked. The
 * JPEGImageDecoder can also act as an ImageProducer sending each
 * scanline to image consumers.  Decoding the JPEG input stream is
 * controlled by the parameters in theJPEGParam
 * object.  If no JPEGParam object is specified when JPEGImageDecoder 
 * is constructed, the constructor creates a JPEGParam object that has 
 * decompression options set to the defaults for the IJG implementation.
 * 
 * The JPEGParam object is updated with information from the file header
 * during decompression. If the input stream contains tables only
 * information (no image data), the JPEGParam object will be updated and
 * NULL returned for the output image. If the input stream contains only
 * image data, the parameters and tables in the current JPEGParam object
 * will be used to decode in decoding the JPEG stream. If no tables are 
 * set in the JPEGParam  object, an exception will be thrown.
 *
 * ColorSpace comments:  First off JPEG by specification is color blind!
 * That said, the IJG jpeg library does perform some color space conversion 
 * in the name of better compression ratios.  If the color space of the data
 * that is passed to the decoder has been converted the decoder will undo
 * the conversion if it knows how.  Some updates to the standard color space
 * designations have been made to allow this decoder to be utilized for
 * decoding FlashPix Images.  I the case where these color spaces are 
 * encountered no color space conversion is performed.  The data is simply
 * decoded by the library.  See the JPEGParam description for more details
 * on additional color space designations.
 *
 * SampleModel/DataModel comments:  First off the best performance from
 * a strictly decompression point of view can be had by using methods that
 * request the native library to return data in a ComponentSampleModel
 * that contains DataBuffer.BYTE_DATA.   Additionally, a 
 * BufferedImage can be returned from the decoder.  If possible this 
 * BufferedImage will contain an array of int's that contain packed 
 * ARGB data.  If the data cannot be coereced into this form an exception
 * will be thrown.  This particular data organization
 * ( BufferedImage.TYPE_INT_ARGB ) will facilitate easy display of the 
 * decoded images within the java2d framework.
 * 
 * Final Comments:  This decoder will process interchange, abbreviated and 
 * progressive jpeg streams.  However, progressive jpeg streams are treated
 * as interchange streams.  They return once with the entire image in the 
 * image buffer.  
 */

import java.io.InputStream;
import java.io.IOException;
import java.awt.Point;
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DirectColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.DataBufferInt;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import com.sun.image.codec.jpeg.*;

/**
 * This class describes a JPEG image decoder.  This decoder takes an
 * InputStream that contains JPEG encoded image data.  The
 * JPEGImageDecoder will decode the JPEG image data according to the
 * parameters set in a JPEGParam object.  The resulting image data is
 * returned in either a Raster or a BufferedImage.
 * @see JPEGDecoderParam
 * @see Raster
 * @see BufferedImage
 * @version 4 December 1997
 */
 
public class JPEGImageDecoderImpl implements JPEGImageDecoder {

    private static final Class InputStreamClass = InputStream.class;

    /** 
     * Private Stuff 
     *
     */
    private JPEGDecodeParam 	param   = null;
    private InputStream         input   = null;
    private WritableRaster      aRas    = null;
    private BufferedImage       aBufImg = null;
    private ColorModel		cm      = null;
    private boolean		unpack  = false;
    private boolean		flip    = false;

    // load the jpeg lib when created.
    static { 
	java.security.AccessController.doPrivileged(
		new sun.security.action.LoadLibraryAction("jpeg"));
    }
	
    /**
     * Constructs a JPEGImageDecoder that will decode the JPEG data 
     * found in the InputStream.  The default decompression parameters will 
     * be set.
     * @param in an InputStream containing JPEG encoded data
     * @see InputStream
     */
    public JPEGImageDecoderImpl(InputStream in) {
		if (in == null) 
			throw new IllegalArgumentException("InputStream is null.");

		input = in;
		initDecoder( InputStreamClass );
    }

    /**
     * Constructs a JPEGImageDecoder that will decode the JPEG data 
     * found in the InputStream.  The default decompression parameters will 
     * be set.
     * @param in an InputStream containing JPEG encoded data
     * @param jdp The JPEGDecodeParam object to use when decoding.
     * @see InputStream
     */
    public JPEGImageDecoderImpl(InputStream in, JPEGDecodeParam jdp) {
		this(in);
		setJPEGDecodeParam(jdp);
    }

    /**
     * Returns the JPEGDecodeParam object containing the JPEG tables and 
     * parameters
     */
    public JPEGDecodeParam getJPEGDecodeParam() {
		if (param != null)
			return (JPEGDecodeParam)param.clone();
		else
			return null;
    }

    /**
     * Sets the JPEGDecodeParam object used to determine the features of the
     * decompression performed on the JPEG encoded data.
     * @param jdp JPEGDecodeParam object
     */
    public void setJPEGDecodeParam(JPEGDecodeParam jdp) {
        param = (JPEGDecodeParam)jdp.clone();
    }
	
	/**
	 * Get the input stream that decoding will occur from.
	 * @return The stream that the decoder is currently assciated with.
	 */
	public synchronized InputStream getInputStream() {
		return input;
	}

    /**
     * decodes the JPEG stream that was passed as part of construction.  The 
     * JPEG decompression will be performed according to the current settings 
     * of the JPEGParam object.
     * @return Raster the resulting image will be returned in a byte component
     *         Raster of data.  Colorspace and other pertinent information can 
     *         be obtained from the JPEGParam object.
     * @exception ImageFormatException if irregularities in the JPEG stream or 
     *         an unknown condition was encountered an ImageFormatException is
     *         thrown.
     */
    public synchronized Raster decodeAsRaster() throws ImageFormatException { 
		try {
			param = readJPEGStream(input, param, false);
        } catch(IOException e) {
			System.out.println("Can't open input Stream" + e);
			e.printStackTrace();
		}

		return aRas;
    }

    /**
     * Decodes the current JPEG data stream.  The result of decoding this
     * InputStream is a BufferedImage the ColorModel associated with
     * this BufferedImage is determined based on the encoded COLOR_ID
     * of the JPEGDecodeParam object.
     * @return BufferedImage - contains the image data.
     * @exception ImageFormatException -  if irregularities in the JPEG stream
     *     or an unknown condition was encountered an ImageFormatException is
     *     thrown.
     */
    public synchronized  BufferedImage decodeAsBufferedImage()
		throws ImageFormatException 
	{
		try {
			param = readJPEGStream(input, param, true);
		} catch(IOException e){
			System.out.println("Can't open input Stream" + e);
			e.printStackTrace();
		}
		return aBufImg;
    }

    private native void initDecoder( Class inputStrClass);

	/**
	 * This is the method that does most of the work.  It calls back
	 * into Java, allocateDataBuffer to allocate the Raster and (if
	 * appropriate the BufferedImage).
	 * @param is     The stream to read the JPEG data from.
	 * @param params The parameters that control decoding.  If
	 *               non-null The decoder will assume we are decoding
	 *               an abbreviated JPEG data Stream and use the
	 *               Tables out of param.
	 * @param colorCvt This indicates weather default color
	 *                 conversions should take place.  Most noteably
	 *                 YCrCb->RGB.
	 */
    private synchronized native JPEGDecodeParam readJPEGStream
		(InputStream is, JPEGDecodeParam params, boolean colorCvt)
		throws IOException, ImageFormatException; 
		

    /** 

	 * Creates a JPEGDecodeParam object with properties read from the
	 * JPEG header. This does not actually decompress the image
	 * data. Throws an exception if the input stream contains only
	 * image data.
     */
    private void readTables() throws IOException {
		try{
            param = readJPEGStream(input, null, false);
		}
		// Shouldn't really ever happen since we aren't reading image data...
		catch( ImageFormatException ife ){ ife.printStackTrace();}
    }

	/**
	 * 
	 */
	private int getDecodedColorModel(int colorID, boolean convert) 
		throws ImageFormatException 
	{
		int ret;
		int []bits1 = {8};
		int []bits3 = {8, 8, 8};
		int []bits4 = {8, 8, 8, 8};
		
		cm     = null;
		unpack = false;
		flip   = false;

		if (!convert) return colorID;

		switch (colorID) {
		case JPEGDecodeParam.COLOR_ID_GRAY:
			cm = new ComponentColorModel
			    (ColorSpace.getInstance(ColorSpace.CS_GRAY), bits1, 
			    false, false, Transparency.OPAQUE,
			    DataBuffer.TYPE_BYTE);
			return colorID;
		case JPEGDecodeParam.COLOR_ID_PYCC:
			cm = new ComponentColorModel
			    (ColorSpace.getInstance(ColorSpace.CS_PYCC), bits3, 
			    false, false, Transparency.OPAQUE,
			    DataBuffer.TYPE_BYTE);
			return colorID;
		case JPEGDecodeParam.COLOR_ID_PYCCA:
			cm = new ComponentColorModel
			    (ColorSpace.getInstance(ColorSpace.CS_PYCC), bits4, 
			    true, false, Transparency.TRANSLUCENT,
			    DataBuffer.TYPE_BYTE);
			return colorID;

		case JPEGDecodeParam.COLOR_ID_RGB:
		case JPEGDecodeParam.COLOR_ID_YCbCr:
			unpack = true;
			cm = new DirectColorModel(24, 0xFF0000, 0xFF00, 0xFF);
			return JPEGDecodeParam.COLOR_ID_RGB;
			
		case JPEGDecodeParam.COLOR_ID_RGBA_INVERTED:
		case JPEGDecodeParam.COLOR_ID_YCbCrA_INVERTED:
			flip = true;
		case JPEGDecodeParam.COLOR_ID_RGBA:
		case JPEGDecodeParam.COLOR_ID_YCbCrA:
			unpack = true;
			cm = new DirectColorModel
			    (ColorSpace.getInstance(ColorSpace.CS_sRGB), 32, 
			    0xFF0000, 0xFF00, 0xFF, 0xFF000000, false,
			    DataBuffer.TYPE_INT);
			return JPEGDecodeParam.COLOR_ID_RGBA;

		case JPEGDecodeParam.COLOR_ID_UNKNOWN:
		case JPEGDecodeParam.COLOR_ID_CMYK:
		case JPEGDecodeParam.COLOR_ID_YCCK:
		default:
			throw new ImageFormatException
			("Can't construct a BufferedImage for given COLOR_ID");
		}
	}


    /**
     * allocateDataBuffer - This is a private method that is generally
     * intended to be called from native code after the JPEG header
     * has been read.  This provides the information necessary to
     * create the DataBufferByte object that will actually contain the
     * decoded image information.
     * @param imgWidth
     * @param imgHeight
     * @param imgComponents
     * @return byte[] - an array in which the decoded image data will be stored.     */
	private Object allocateDataBuffer(int imgWidth, int imgHeight,
					  int imgComponents) {
		Object data;
		if (unpack) {
			if (imgComponents == 3) {
			    int [] masks = { 0xFF0000, 0xFF00, 0xFF };
			    aRas=Raster.createPackedRaster(DataBuffer.TYPE_INT,
			        imgWidth, imgHeight, masks, new Point(0,0));
			} else if (imgComponents == 4) {
			    int [] masks = {0xFF0000, 0xFF00, 0xFF, 0xFF000000};
			    aRas=Raster.createPackedRaster(DataBuffer.TYPE_INT,
				imgWidth, imgHeight, masks, new Point(0,0));
			} else {
			    throw new ImageFormatException
			    ("Can't unpack with anything other than 3 or 4 components");
			}
			data = ((DataBufferInt)aRas.getDataBuffer()).getData();
		} else {
			aRas = Raster.createInterleavedRaster(
			    DataBuffer.TYPE_BYTE, imgWidth, imgHeight,
			    imgComponents, new Point(0,0));
			data = ((DataBufferByte)aRas.getDataBuffer()).getData();
		}

		if (cm != null)
			aBufImg = new BufferedImage(cm, aRas, true, null);

		return data;
	}
} // end class JPEGImageDecoder

