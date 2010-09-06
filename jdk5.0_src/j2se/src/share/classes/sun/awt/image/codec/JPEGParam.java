/*
 * @(#)JPEGParam.java	1.8 03/12/19
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
import com.sun.image.codec.jpeg.*;
import java.awt.image.ColorModel;
import java.awt.color.ColorSpace;
import java.util.Vector;
import java.util.Enumeration;

public class JPEGParam implements JPEGEncodeParam, Cloneable {
    /** Number of components for each Color id */
    private static int[] defComponents = { -1, /* COLOR_ID_UNKNOWN */
											1,	/* COLOR_ID_GRAYSCALE */
											3, 	/* COLOR_ID_RGB */
											3, 	/* COLOR_ID_YCbCr */
											4,  /* COLOR_ID_CMYK */
											3, 	/* COLOR_ID_PYCC */
											4,  /* COLOR_ID_RGBA */
											4,  /* COLOR_ID_YCbCrA */
											4,  /* COLOR_ID_RGBA_INVERTED */
											4,  /* COLOR_ID_YCbCrA_INVERTED */
											4,  /* COLOR_ID_PYCCA */
											4}; /* COLOR_ID_YCCK */
	
    private static int[][] stdCompMapping = {
		{ 0, 0, 0, 0 }, /* COLOR_ID_UNKNOWN */
		{ 0 }, 			/* COLOR_ID_GRAYSCALE */
		{ 0, 0, 0 }, 	/* COLOR_ID_RGB */
		{ 0, 1, 1 }, 	/* COLOR_ID_YCbCr */
		{ 0, 0, 0, 0 }, /* COLOR_ID_CMYK */
		{ 0, 1, 1 }, 	/* COLOR_ID_PYCC */
		{ 0, 0, 0, 0 }, /* COLOR_ID_RGBA */
		{ 0, 1, 1, 0 }, /* COLOR_ID_YCbCrA */
		{ 0, 0, 0, 0 }, /* COLOR_ID_RGBA_INVERTED */
		{ 0, 1, 1, 0 }, /* COLOR_ID_YCbCrA_INVERTED */
		{ 0, 1, 1, 0 }, /* COLOR_ID_PYCCA */
		{ 0, 1, 1, 0 }, /* COLOR_ID_YCCK */
	};

    private static int[][] stdSubsample   = {
		{ 1, 1, 1, 1 }, /* COLOR_ID_UNKNOWN */
		{ 1 }, 			/* COLOR_ID_GRAYSCALE */
		{ 1, 1, 1 }, 	/* COLOR_ID_RGB */
		{ 1, 2, 2 }, 	/* COLOR_ID_YCbCr */
		{ 1, 1, 1, 1 }, /* COLOR_ID_CMYK */
		{ 1, 2, 2 }, 	/* COLOR_ID_PYCC */
		{ 1, 1, 1, 1 }, /* COLOR_ID_RGBA */
		{ 1, 2, 2, 1 }, /* COLOR_ID_YCbCrA */
		{ 1, 1, 1, 1 }, /* COLOR_ID_RGBA_INVERTED */
		{ 1, 2, 2, 1 }, /* COLOR_ID_YCbCrA_INVERTED */
		{ 1, 2, 2, 1 }, /* COLOR_ID_PYCCA */
		{ 1, 2, 2, 1 }, /* COLOR_ID_YCCK */
	};
	
	/** Width of encoded image as read from JFIF header */
	private int width;
	/** Height of encoded image as read from JFIF header */
	private int height;
	private int encodedColorID;
	private int numComponents;

	private byte[][][] appMarkers;
	private byte[][]   comMarker;

	private boolean imageInfoValid;
	private boolean tableInfoValid;

	/** Subsampling values by component */
	private int horizontalSubsampling[];
	private int verticalSubsampling[];

	/** Quantization tables 1..4 */
	private JPEGQTable qTables[];
	/** The mapping between components (index) and tables (value). */
	private int qTableMapping[];

	/** DC Entropy encoding tables 1..4 */
	private JPEGHuffmanTable dcHuffTables[];
	/** The mapping between components (index) and tables (value). */
	private int dcHuffMapping[];

	/** AC Entropy encoding tables 1..4 */
	private JPEGHuffmanTable acHuffTables[];
	/** The mapping between components (index) and tables (value). */
	private int acHuffMapping[];

	/** MCUs per restart, or 0 for no restart */
	private int restartInterval;			
	
	/** 
	 * Constructs a JPEGParam block with options that are suitable
	 * for the specified colorID.  The compression tables are set to
	 * the standard encoding tables for 8 bit data.  The quantization
	 * tables are the standard ones divided by 2 (very high quality).
	 * @param colorID The colorID of the encoded data.  This is
	 * required up front as it influences the default values for many
	 * of the other entries.  For this reason it is immutable.  If you
	 * wish to change the encoded COLOR_ID, you must construct a new
	 * JPEGParam and copy over any relevant information.  This will
	 * not work for COLOR_ID_UNKOWN you must construct JPEGParam with
	 * the number of components.
	 */
	public JPEGParam(int colorID) {
		this(colorID, defComponents[colorID]);
	}

	/** 
	 * Constructs a JPEGEncodingParam object that copies most of it's
	 * state from src.
	 */
	public JPEGParam(JPEGDecodeParam src) {
		this(src.getEncodedColorID(), src.getNumComponents());
		copy(src);
	}

	/** 
	 * Constructs a JPEGEncodingParam object that copies all of it's
	 * state from src.
	 */
	public JPEGParam(JPEGEncodeParam src) {
		this(src.getEncodedColorID(), src.getNumComponents());
		copy(src);
	}

	/** 
	 * Constructs a JPEGParam block with options that are suitable for
	 * the specified colorID.  The compression tables are set to the
	 * standard encoding tables for 8 bit data.  The quantization
	 * tables are the standard ones divided by 2 (very high quality).
	 * @param colorID The colorID of the encoded data.  This is
	 * required up front as it influences the default values for many
	 * of the other entries.  For this reason it is immutable.  If you
	 * wish to change the encoded COLOR_ID, you must construct a new
	 * JPEGParam and copy over any relevant information.
	 * @param numComponents The number of components
	 */
	public JPEGParam(int colorID, int numComponents) {
		if ((colorID != COLOR_ID_UNKNOWN)  &&
			(numComponents != defComponents[colorID]))
			throw new IllegalArgumentException
				("NumComponents not in sync with COLOR_ID");

		qTables      = new JPEGQTable      [NUM_TABLES];
		acHuffTables = new JPEGHuffmanTable[NUM_TABLES];
		dcHuffTables = new JPEGHuffmanTable[NUM_TABLES];
		for( int i = 0; i < NUM_TABLES ; i++ ) {
			qTables[i] = null ;
			dcHuffTables[i] = null ;
			acHuffTables[i] =  null ;
		}
		comMarker    = null;
		appMarkers   = new byte[16][][];

		this.numComponents = numComponents;
		setDefaults(colorID);
	}

	private void copy(JPEGDecodeParam src) {
		if (getEncodedColorID() != src.getEncodedColorID())
			throw new IllegalArgumentException
				("Argument to copy must match current COLOR_ID");
		if (getNumComponents() != src.getNumComponents())
			throw new IllegalArgumentException
				("Argument to copy must match in number of components");
			
		setWidth (src.getWidth());
		setHeight(src.getHeight());

		for (int i=APP0_MARKER; i<APPF_MARKER; i++)
			setMarkerData(i, copyArrays(src.getMarkerData(i)));

		setMarkerData(COMMENT_MARKER, 
					  copyArrays(src.getMarkerData(COMMENT_MARKER)));

		setTableInfoValid(src.isTableInfoValid());
		setImageInfoValid(src.isImageInfoValid());

		setRestartInterval(src.getRestartInterval());

		for(int i=0; i<NUM_TABLES; i++) {
			setDCHuffmanTable(i, src.getDCHuffmanTable(i));
			setACHuffmanTable(i, src.getACHuffmanTable(i));
			setQTable(i, src.getQTable(i));
		}
		
		for (int i=0; i<src.getNumComponents(); i++) {
			setDCHuffmanComponentMapping
				(i, src.getDCHuffmanComponentMapping(i));
			setACHuffmanComponentMapping
				(i, src.getACHuffmanComponentMapping(i));
			setQTableComponentMapping
				(i, src.getQTableComponentMapping(i));
			setHorizontalSubsampling
				(i, src.getHorizontalSubsampling(i));
			setVerticalSubsampling
				(i, src.getVerticalSubsampling(i));
		}
	}

	private void copy(JPEGEncodeParam src) {
		copy ((JPEGDecodeParam)src);
	}


	/** 
	 * Sets all parameters to reasonable defaults:
	 * <pre>
	 *  markers 			= APP0_MARKER;
	 *  tableInfoValid		= true;
	 *  imageInfoValid		= true;
	 *	densityUnit			= DENSITY_UNIT_ASPECT_RATIO;
	 *	xDensity			= 1;
	 *	yDensity			= 1;
	 *	restartInterval		= 0;
	 *	restartInRows		= 0;
	 *	encodedColorID = COLOR_ID_YCbCr ;
	 * </pre>
	 *  The Huffman tables are set to the Standard Huffman Tables.
	 *  The quantization tables are set to the Standard QTables
	 *  divided by two (very high quality - according to the speC).
	 */
    protected  void	setDefaults(int colorID) {
		encodedColorID 		= colorID ;

		restartInterval		= 0;

		boolean isJFIFcompatible = false;
		switch(numComponents) {
		case 1:
		    if ((encodedColorID == JPEGEncodeParam.COLOR_ID_GRAY) ||
		        (encodedColorID == JPEGEncodeParam.COLOR_ID_UNKNOWN)) {
			isJFIFcompatible = true;
		    }
		    break;
		case 3:
		    if (encodedColorID == JPEGEncodeParam.COLOR_ID_YCbCr) {
			isJFIFcompatible = true;
		    }
		    break;
		case 4:
		    if (encodedColorID == JPEGEncodeParam.COLOR_ID_CMYK) {
			isJFIFcompatible = true;
		    }
		    break;
		default:
		    break;
		}
		if (isJFIFcompatible) {
			addMarkerData(APP0_MARKER, createDefaultAPP0Marker());
		}
		setTableInfoValid(true);
		setImageInfoValid(true);

		dcHuffTables[0] = JPEGHuffmanTable.StdDCLuminance;
		dcHuffTables[1] = JPEGHuffmanTable.StdDCChrominance;
		dcHuffMapping   = new int[getNumComponents()];
		System.arraycopy(stdCompMapping[encodedColorID], 0, 
						 dcHuffMapping, 0, getNumComponents());

		acHuffTables[0] = JPEGHuffmanTable.StdACLuminance;
		acHuffTables[1] = JPEGHuffmanTable.StdACChrominance;
		acHuffMapping   = new int[getNumComponents()];
		System.arraycopy(stdCompMapping[encodedColorID], 0, 
						 acHuffMapping, 0, getNumComponents());

		qTables[0] = JPEGQTable.StdLuminance.getScaledInstance(0.5f, true);
		qTables[1] = JPEGQTable.StdChrominance.getScaledInstance(0.5f, true);
		qTableMapping = new int[getNumComponents()];
		System.arraycopy(stdCompMapping[encodedColorID], 0, 
						 qTableMapping, 0, getNumComponents());

		horizontalSubsampling = new int[getNumComponents()];
		System.arraycopy(stdSubsample[encodedColorID], 0, 
						 horizontalSubsampling, 0, getNumComponents());

		verticalSubsampling   = new int[getNumComponents()];
		System.arraycopy(stdSubsample[encodedColorID], 0, 
						 verticalSubsampling, 0, getNumComponents());
	}

	public Object clone() {
		JPEGParam ret = new JPEGParam(getEncodedColorID(), 
									  getNumComponents());
		ret.copy(this);
		return ret;
	}

	/** 
	 * Get the image width
	 * @return int the width of the image data in pixels.
	 */
	public int  getWidth() { return width; }
	/** Get the image height
	 * @return The height of the image data in pixels.
	 */
	public int  getHeight() { return height ; }  

	/** 
	 * Set the image width. This is only used when decoding
	 * abbreviated JPEG data streams.  In all other cases this value
	 * is ignored.
	 * @param Width of the image data that is to be decoded.
	 */
	public void setWidth( int width ) { this.width = width; }

	/** 
	 * Set the image height.  This is only used when decoding
	 * abbreviated JPEG data streams.  In all other cases this value
	 * is ignored.
	 * @param height of the image data being encoded in pixels.
	 */
	public void setHeight( int height) { this.height = height; }  

	/** 
	 * Return the Horizontal subsampling factor for requested
	 * Component.  The Subsample factor is the number of input pixels
	 * that contribute to each output pixel.  This is distinct from
	 * the way the JPEG to each output pixel.  This is distinct from
	 * the way the JPEG standard defines this quantity, because
	 * fractional subsampling factors are not allowed, and it was felt
	 * @param component The component of the encoded image to return
	 * the subsampling factor for.
	 * @return The subsample factor.
	 */
	public int getHorizontalSubsampling(int component) {
		if ((component < 0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				( "Component must be between 0 and number of components");
		return horizontalSubsampling[component];
	}

	/** 
	 * Return the Vertical subsampling factor for requested
	 * Component.  The Subsample factor is the number of input pixels
	 * that contribute to each output pixel.  This is distinct from
	 * the way the JPEG to each output pixel.  This is distinct from
	 * the way the JPEG standard defines this quantity, because
	 * fractional subsampling factors are not allowed, and it was felt
	 * @param component The component of the encoded image to return
	 * the subsampling factor for.
	 * @return The subsample factor.
	 */
	public int getVerticalSubsampling(int component) {
		if ((component < 0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				( "Component must be between 0 and number of components");
		return verticalSubsampling[component];
	}


	/**
	 * Set the horizontal subsample factor for the given component.
	 * Note that the subsample factor is the number of input pixels
	 * that contribute to each output pixel (ussually 2 for YCC).
	 * @param component The component being specified.
	 * @param subsample The subsampling factor being specified.
	 */
	public void setHorizontalSubsampling(int component, 
										 int subsample) {
		if ((component < 0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Component must be between 0 and number of components: " +
					component );
		if (subsample<=0)
			throw new IllegalArgumentException
				( "SubSample factor must be positive: " + subsample);
			
		horizontalSubsampling[component] = subsample;
	}

	/**
	 * Set the vertical subsample factor for the given component.  Note that
	 * the subsample factor is the number of input pixels that
	 * contribute to each output pixel (ussually 2 for YCC).
	 * @param component The component being specified.
	 * @param subsample The subsampling factor being specified.
	 */
	public void setVerticalSubsampling(int component, 
									   int subsample) {
		if ((component < 0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				( "Component must be between 0 and number of components");
		if (subsample<=0)
			throw new IllegalArgumentException
				( "SubSample factor must be positive.");
			
		verticalSubsampling[component] = subsample;
	}

	/** 
	 * Returns the coefficient quantization tables or NULL if not
	 * defined. tableNum must range in value from 0 - 3.
	 * @param tableNum the index of the table to be returned.
	 * @return Quantization table stored at index tableNum.
	 */
	public JPEGQTable  getQTable(int tableNum ) {
		if (tableNum < 0 || tableNum >= NUM_TABLES )  
			throw new IllegalArgumentException
				( "tableNum must be between 0 and 3.");
		return qTables[tableNum];
	}
	/**
	 * Returns the Quantization table for the requested component.
	 * @param component the image component of interest.
	 * @return Quantization table associated with component
	 */	
	public JPEGQTable getQTableForComponent(int component) {
		if ((component < 0) || (component >= qTableMapping.length))
			throw new IllegalArgumentException
				( "Component must be between 0 and number of components");
		return getQTable(qTableMapping[component]);
	}

	/** 
	 * Returns the DC Huffman coding table requested or null if
	 * not defined
	 * @param tableNum the index of the table to be returned.
	 * @return Huffman table stored at index tableNum.  
	 */	
	public JPEGHuffmanTable getDCHuffmanTable( int tableNum ){
  		if (tableNum < 0 || tableNum >= NUM_TABLES )  
			throw new IllegalArgumentException( "tableNum must be 0-3.");

		return dcHuffTables[tableNum];
	}
	/**
	 * Returns the DC Huffman coding table for the requested component.
	 * @param component the image component of interest.
	 * @return Huffman table associated with component
	 */	
	public JPEGHuffmanTable getDCHuffmanTableForComponent(int component) {
		if ((component < 0) || (component >= dcHuffMapping.length))
			throw new IllegalArgumentException
				( "Component must be between 0 and number of components");
		return getDCHuffmanTable(dcHuffMapping[component]);
	}

		
	/** 
	 * Returns the AC Huffman coding table requested or null if
	 * not defined
	 * @param int tableNum - the index of the table to be returned.
	 * @return Huffman table stored at index tableNum.  
	 */	
	public JPEGHuffmanTable getACHuffmanTable( int tableNum ){
  		if (tableNum < 0 || tableNum >= NUM_TABLES )  
			throw new IllegalArgumentException( "tableNum must be 0-3.");

		return acHuffTables[tableNum];
	}
	/**	 
	 * Returns the AC Huffman coding table for the requested component.
	 * @param component the image component of interest.
	 * @return Huffman table associated with component
	 */	
	public JPEGHuffmanTable getACHuffmanTableForComponent(int component) {
		if ((component < 0) || (component >= acHuffMapping.length))
			throw new IllegalArgumentException
				( "Component must be between 0 and number of components");
		return getACHuffmanTable(acHuffMapping[component]);
	}

	/** 
	 * Sets the coefficient quantization tables at index
	 * passed. tableNum must range in value from 0 - 3.
	 * @param qtable that will be used.
	 * @param tableNum the index of the table to be set.
	 */	
	public void	setQTable( int tableNum, JPEGQTable qTable ) {
		if (tableNum < 0 || tableNum >= NUM_TABLES )  
			throw new IllegalArgumentException
				( "tableNum must be between 0 and 3.");
		qTables[tableNum] = qTable;
	}

	/** Sets the DC Huffman coding table at index to the table provided. 
	 * @param huffTable JPEGHuffmanTable that will be assigned
	 * to index tableNum.
	 * @param tableNum - the index of the table to be set.
	 * @exception IllegalArgumentException - thrown if the tableNum
	 * is out of range.  Index must range in value from 0 - 3.
	 */		
	public void	setDCHuffmanTable( int tableNum, 
								   JPEGHuffmanTable huffTable) {
		if (tableNum < 0 || tableNum >= NUM_TABLES )  
			throw new IllegalArgumentException
				("tableNum must be 0, 1, 2, or 3.");
		dcHuffTables[tableNum] = huffTable;
	}

	/** Sets the AC Huffman coding table at index to the table provided. 
	 * @param huffTable JPEGHuffmanTable that will be assigned
	 * to index tableNum.
	 * @param tableNum - the index of the table to be set.
	 * @exception IllegalArgumentException - thrown if the tableNum
	 * is out of range.  Index must range in value from 0 - 3.
	 */		
	public void	setACHuffmanTable( int tableNum,
								   JPEGHuffmanTable huffTable) {
		if (tableNum < 0 || tableNum >= NUM_TABLES )  
			throw new IllegalArgumentException
				("tableNum must be 0, 1, 2, or 3.");
		acHuffTables[tableNum] = huffTable;
	}



	/**
	 * Get the number of the DC Huffman table that will be used for
	 * a particular component.
	 * @param component The Component of interest.
	 * @return The table number of the DC Huffman table for component.
	 */
	public int getDCHuffmanComponentMapping(int component) {
		if ((component <0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Requested Component doesn't exist.");
		return dcHuffMapping[component];
	}

	/**
	 * Get the number of the AC Huffman table that will be used for
	 * a particular component.
	 * @param component The Component of interest.
	 * @return The table number of the AC Huffman table for component.
	 */
	public int getACHuffmanComponentMapping(int component) {
		if ((component <0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Requested Component doesn't exist.");
		return acHuffMapping[component];
	}
	/**
	 * Get the number of the quantization table that will be used for
	 * a particular component.
	 * @param component The Component of interest.
	 * @return The table number of the Quantization table for component.
	 */
	public int getQTableComponentMapping(int component) {
		if ((component <0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Requested Component doesn't exist.");
		return qTableMapping[component];
	}

	/**
	 * Sets the mapping between a component and it's DC Huffman Table.
	 * @param component The component to set the mapping for
	 * @param table The DC Huffman table to use for component
	 */
	public void setDCHuffmanComponentMapping( int component, int table ) {
		if ((component <0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Given Component doesn't exist.");

		if ((table <0) || (table >= NUM_TABLES))
			throw new IllegalArgumentException
				("Tables must be 0, 1, 2, or 3.");
		
		dcHuffMapping[component] = table;
	}

	/**
	 * Sets the mapping between a component and it's AC Huffman Table.
	 * @param component The component to set the mapping for
	 * @param table The AC Huffman table to use for component
	 */
	public void setACHuffmanComponentMapping( int component, int table ) {
		if ((component <0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Given Component doesn't exist.");

		if ((table <0) || (table >= NUM_TABLES))
			throw new IllegalArgumentException
				("Tables must be 0, 1, 2, or 3.");
		
		acHuffMapping[component] = table;
	}

	/**
	 * Sets the mapping between a component and it's Quantization Table.
	 * @param component The component to set the mapping for
	 * @param table The Quantization Table to use for component
	 */
	public void setQTableComponentMapping( int component, int table ) {
		if ((component <0) || (component >= getNumComponents()))
			throw new IllegalArgumentException
				("Given Component doesn't exist.");

		if ((table <0) || (table >= NUM_TABLES))
			throw new IllegalArgumentException
				("Tables must be 0, 1, 2, or 3.");
		
		qTableMapping[component] = table;
	}

	public boolean isImageInfoValid() {
		return imageInfoValid;
	}
	public void setImageInfoValid(boolean flag) {
		imageInfoValid = flag;
	}
	
	
	public boolean isTableInfoValid() {
		return tableInfoValid;
	}
	public void setTableInfoValid(boolean flag) {
		tableInfoValid = flag;
	}

	public boolean getMarker(int marker) 
	{ 
		byte[][] data=null;
		if (marker == COMMENT_MARKER)
			data = comMarker;
		else if ((marker >= APP0_MARKER) &&
				 (marker <= APPF_MARKER))
			data = appMarkers[marker-APP0_MARKER];
		else
			throw new IllegalArgumentException
				("Invalid Marker ID:" + marker);
		
		if (data == null) return false;
		if (data.length == 0) return false;
		return true;
	}

	public byte[][] getMarkerData(int marker) {
		if (marker == COMMENT_MARKER)
			return comMarker;
		else if ((marker >= APP0_MARKER) &&
				 (marker <= APPF_MARKER))
			return appMarkers[marker-APP0_MARKER];
		else
			throw new IllegalArgumentException
				("Invalid Marker ID:" + marker);
	}

	public void setMarkerData(int marker, byte[][]data) 
	{ 
		// System.out.println("Setting " + marker + " To: " + vec);
		if (marker == COMMENT_MARKER)
			comMarker = data;
		else if ((marker >= APP0_MARKER) &&
				 (marker <= APPF_MARKER))
			appMarkers[marker-APP0_MARKER] = data;
		else
			throw new IllegalArgumentException
				("Invalid Marker ID:" + marker);
	}

	public void addMarkerData(int marker, byte[] data) 
	{ 
		if (data == null) {
			return;
		}
		if (marker == COMMENT_MARKER) {
			comMarker = appendArray(comMarker,data);
		} else if ((marker >= APP0_MARKER) &&
				   (marker <= APPF_MARKER)) {
			appMarkers[marker-APP0_MARKER] = 
				appendArray(appMarkers[marker-APP0_MARKER], data);
		} else
			throw new IllegalArgumentException
				("Invalid Marker ID:" + marker);
	}

	/** 
	 * Returns the JPEG Encoded color id. This is generally
	 * speaking only used if you are decoding into Rasters.  Note
	 * that when decoding into a Raster no color conversion is
	 * performed.
	 * @return The value of the JPEG encoded data's color id.
	 */
    public int getEncodedColorID() { return encodedColorID;}

	/**
	 * Returns the number of components for the current encoding
	 * COLOR_ID.
	 * @return the number of Components
	 */
	public int getNumComponents()	{ return numComponents; }

	/** 
	 * Returns the number of components for the color id passed as
	 * a parameter.
	 * @param id A COLOR_ID.
	 * @return the number of components
	 * @exception IllegalArgumentException thrown when the value
	 * of the of the color id that is passed is not a valid value
	 * as described in the publicly defined color id values.
	 */
	public static int getNumComponents(int id) {
        if ( id < 0 || id >= NUM_COLOR_ID )
			throw new IllegalArgumentException("Invalid JPEGColorID.");
		else
			return defComponents[id];	
	}

	/** 
	 * Get the MCUs per restart marker.
	 * @return The number of MCUs between restart markers.
	 */
	public int getRestartInterval() { return restartInterval; }

	/**
	 * Set the MCUs per restart, or 0 for no restart markers.
	 * @param restartInterval number MCUs per restart marker.
	 */
	public void setRestartInterval( int restartInterval ) { 
		this.restartInterval = restartInterval; }

	/** 
	 * Get the JFIF code for pixel size units This value is copied
	 * from the JFIF APP0 marker. It isn't used by the JPEG code.
	 * @return Value indicating the density unit one of the
	 * DENSITY_UNIT_* constants.
	 * @thrown IllegalArgumentException If APP0 is not available.
	 */
	public int getDensityUnit() {
		if (!getMarker(APP0_MARKER))
			throw new IllegalArgumentException("No APP0 marker present");

                byte []data = findAPP0();
                if (data==null)
                  throw new IllegalArgumentException
                    ("Can't understand APP0 marker that is present");
		return data[7]; 
	}

	/** 
	 * Get the horizontal pixel density This value is copied from the
	 * JFIF APP0 marker. It isn't used by the JPEG code.
	 * @return The horizontal pixel density, in units described by
	 * @see JPEGDecodeParam.getDensityUnit.
	 */
	public int getXDensity() { 
		if (!getMarker(APP0_MARKER))
			throw new IllegalArgumentException("No APP0 marker present");

                byte []data = findAPP0();
                if (data==null)
                  throw new IllegalArgumentException
                    ("Can't understand APP0 marker that is present");

		int ret = data[8]<<8 | (data[9]&0xFF);
		return ret; 
	}

	/** 
	 * Get the vertical pixel density This value is copied into
	 * the JFIF APP0 marker. It isn't used by the JPEG code.
	 * @return The verticle pixel density, in units described by
	 * @see JPEGDecodeParam.getDensityUnit.
	 */
	public int getYDensity() {
		if (!getMarker(APP0_MARKER))
			throw new IllegalArgumentException("No APP0 marker present");

                byte []data = findAPP0();
                if (data==null)
                  throw new IllegalArgumentException
                    ("Can't understand APP0 marker that is present");

		int ret = data[10]<<8 | (data[11]&0xFF);
		return ret; 
	}

	/** Set the JFIF code for pixel size units This value is copied
	 * into the JFIF APP0 marker. It isn't used by the JPEG code.
	 * @param units One of the DENSITY_UNIT_* values.
	 */
	public void setDensityUnit( int unit) { 
		byte [] data=null;

		if (!getMarker(APP0_MARKER)) {
			data = createDefaultAPP0Marker();
			addMarkerData(APP0_MARKER, data);
		} else {
                  data = findAPP0();
                  if (data==null)
                    throw new IllegalArgumentException
                      ("Can't understand APP0 marker that is present");
		}
		data[7] = (byte)unit;
	}
	/** 
	 * Set the horizontal pixel density This value is written into
	 * the JFIF APP0 marker. It isn't used by the JPEG code.
	 * @param desnity the horizontal pixel density, in units
	 * described by @see JPEGParam.getDensityUnit.
	 */
	public void setXDensity( int density ) {
		byte [] data=null;

		if (!getMarker(APP0_MARKER)) {
			data = createDefaultAPP0Marker();
			addMarkerData(APP0_MARKER, data);
		} else {
                  data = findAPP0();
                  if (data==null)
                    throw new IllegalArgumentException
                      ("Can't understand APP0 marker that is present");
		}
		data[8] = (byte)((density>>>8) & 0xFF);
		data[9] = (byte)( density      & 0xFF);
	}
	/** 
	 * Set the vertical pixel density.  This value is copied into
	 * the JFIF APP0 marker. It isn't used by the JPEG code.
	 * @param density The verticle pixel density, in units
	 * described by @see JPEGParam.getDensityUnit.
	 */
	public void setYDensity( int density ) {
		byte [] data=null;

		if (!getMarker(APP0_MARKER)) {
			data = createDefaultAPP0Marker();
			addMarkerData(APP0_MARKER, data);
		} else {
                  data = findAPP0();
                  if (data==null)
                    throw new IllegalArgumentException
                      ("Can't understand APP0 marker that is present");
		}
		data[10] = (byte)((density>>>8) & 0xFF);
		data[11] = (byte)( density      & 0xFF);
	}
  
	/** 
	 * This creates new Quantization tables that replace the currently
	 * installed Quantization tables.  It also updates the Component
	 * QTable mapping to the default for the current encoded COLOR_ID.

	 * The Created Quantization table varies from very high
	 * compression, very low quality, (0.0) to low compression, very
	 * high quality (1.0) based on the quality parameter.<P>
	 
	 * At a quality level of 1.0 the table will be all 1's which will
	 * lead to no loss of data due to quantization (however chrominace
	 * subsampling, if used, and roundoff error in the DCT will still
	 * degrade the image some what).<P>

	 * This is a linear manipulation of the standard Chrominance
	 * Q-Table.<P>

	 * <pre>Some guidelines: 0.75 high quality
	 *                 0.5  medium quality
	 *                 0.25 low quality
	 * </pre>
	 * @param quality 0.0-1.0 setting of desired quality level.
	 * @param forceBaseline force baseline quantization table
	 */
	public void setQuality(float quality, boolean forceBaseline ) {
		double q = quality;

		// Clamp user value to 0.01<->1.0 range
		if ( q <= 0.01 ) q=0.01;
		if ( q  > 1.0  ) q=1.0;

		/* The basic table is used as-is (scaling 1.0) for a quality
		 * of 0.5.  Qualities .5-1.0 are converted to a scaling factor
		 * 2.0 - 2*Q Note that at Q=1.0 the scaling is 0, which will
		 * cause all of the table entries to be 1 (hence, no
		 * quantization loss, still may have chominace subsampling).
		 * Qualities 0.0-.5 are converted to scaling percentage .5/Q
		 */
		if (q < .5 ) q = 0.5 / q;
		else         q = 2.0 - q*2;

		qTableMapping = new int[getNumComponents()];
		System.arraycopy(stdCompMapping[encodedColorID], 0, 
						 qTableMapping, 0, getNumComponents());
		
		JPEGQTable table;
		table = JPEGQTable.StdLuminance;
		qTables[0] = table.getScaledInstance((float)q, forceBaseline);

		table = JPEGQTable.StdChrominance;
		qTables[1] = table.getScaledInstance((float)q, forceBaseline);
		qTables[2] = null;
		qTables[3] = null;
	}

      /* This finds the first occurence of an APP0 marker that meets the
       * criteria for being a JFIF marker.
       */
      byte [] findAPP0() {
        byte[][] data=null;

        data = getMarkerData(APP0_MARKER);
        if (data == null) return null;

        for (int i=0; i<data.length; i++) {
          if ((data[i] != null) && checkAPP0(data[i])) {
	      return data[i];
	  }
	}
        return null;
      }

      static boolean checkAPP0(byte []data) {
		// not long enough
		if (data.length<app0Length) return false;
		if ((data[0] != 0x4A) || // 'J'
			(data[1] != 0x46) || // 'F'
			(data[2] != 0x49) || // 'I'
			(data[3] != 0x46) || // 'F'
			(data[4] != 0x0))  // '\0'
			return false;
		// check major version (possible problem if > 1, but there are
		// bad files out there).
		if (data[5] < 1) return false;

		// We'll accept any minor number (data[6])

		// It passed our tests it's a JFIF header we understand...
		return true;
	}

	// Length is 14 bytes.
	private final static int app0Length = 5 + 2 + 1 + 2 + 2 + 1 + 1;
	static byte[] createDefaultAPP0Marker() {
		
		byte[] ret = new byte[app0Length];

		ret[0] = 0x4A; // 'J'
		ret[1] = 0x46; // 'F'
		ret[2] = 0x49; // 'I'
		ret[3] = 0x46; // 'F'
		ret[4] = 0x0;  // '\0'

		ret[5] = 1;  // Major Version
		ret[6] = 1;  // Minor Version

		ret[7] = (byte)DENSITY_UNIT_ASPECT_RATIO;

		ret[8] = 0;  ret[9] = 1; // X Density...
		ret[10] = 0; ret[11] = 1; // Y Density...

		ret[12] = 0; ret[13] = 0; // We don't do thumbnails...
		return ret;
	}

	static byte[] copyArray(byte[] src) {
		if (src == null) return null;
		byte[] ret = new byte[src.length];
		System.arraycopy(src, 0, ret, 0, src.length);
		return ret;
	}

	static byte[][] copyArrays(byte[][] in) {
		if (in == null) return null;
		byte[][] ret = new byte[in.length][];

		for (int i=0; i<in.length; i++)
			if (in[i] != null) ret[i] = copyArray(in[i]);

		return ret;
	}

	static byte[][] appendArray(byte[][] in, byte[] add) {
		int len = 0;

		if (in != null) len = in.length;

		byte[][] ret = new byte[len+1][];
		for (int i=0; i<len; i++)
			ret[i] = in[i];

		if (add != null )
			ret[len] = copyArray(add);
		return ret;
	}

	static byte[][] buildArray(Vector vec) {
		if (vec == null) return null;

		int i=0;
		byte[][]ret = new byte[vec.size()][];
		Enumeration e = vec.elements();
		while (e.hasMoreElements()) {
			byte []src = (byte[])e.nextElement();
			if (src != null) ret[i++] = copyArray(src);
		}
		return ret;
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
	public static int getDefaultColorId(ColorModel cm) {
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
}
