/*
 * @(#)CUPSPrinter.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import java.net.URL;
import java.net.HttpURLConnection;
import java.io.OutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import sun.print.IPPPrintService;
import sun.print.CustomMediaSizeName;
import sun.print.CustomMediaTray;
import javax.print.attribute.standard.Media;
import javax.print.attribute.standard.MediaSizeName;
import javax.print.attribute.standard.MediaSize;
import javax.print.attribute.standard.MediaTray;
import javax.print.attribute.standard.MediaPrintableArea;
import javax.print.attribute.Size2DSyntax;
import javax.print.attribute.Attribute;
import javax.print.attribute.EnumSyntax;
import javax.print.attribute.standard.PrinterName;


public class CUPSPrinter  {

    private static final double PRINTER_DPI = 72.0;
    private static boolean initialized;
    private static native String getCupsServer();
    private static native int getCupsPort();
    private static native boolean canConnect(String server, int port);
    private static native boolean initIDs();
    // These functions need to be synchronized as
    // CUPS does not support multi-threading.
    private static synchronized native String[] getMedia(String printer);
    private static synchronized native float[] getPageSizes(String printer); 
    //public static boolean useIPPMedia = false; will be used later

    private MediaPrintableArea[] cupsMediaPrintables;
    private MediaSizeName[] cupsMediaSNames;
    private CustomMediaSizeName[] cupsCustomMediaSNames;
    private MediaTray[] cupsMediaTrays;

    static {
 	//initialize AWT for native code to function properly
	java.awt.Toolkit.getDefaultToolkit(); 
	libFound = initIDs(); 
    }

    public  int nPageSizes = 0;
    public  int nTrays = 0;

    private  String[] media;
    private  float[] pageSizes;
    private String printer;
    private static boolean libFound;

    CUPSPrinter (String printerName) {
	if (printerName == null) {
	    throw new IllegalArgumentException("null printer name");
	}
	printer = printerName;
	cupsMediaSNames = null;
	cupsMediaPrintables = null;
	cupsMediaTrays = null;
	initialized = false;
	
	if (!libFound) {
	    throw new RuntimeException("cups lib not found");
	} else {
	    // get page + tray names
	    media =  getMedia(printer); 
	    if (media == null) {
		// either PPD file is not found or printer is unknown
		throw new RuntimeException("error getting PPD");
	    }

	    // get sizes
	    pageSizes = getPageSizes(printer);
	    if (pageSizes != null) {
		nPageSizes = pageSizes.length/6;

		nTrays = media.length/2-nPageSizes;
		assert (nTrays >= 0);
	    }
	}
    }
    

    /**
     * Returns array of MediaSizeNames derived from PPD.
     */
    public MediaSizeName[] getMediaSizeNames() {
	initMedia();
	return cupsMediaSNames;
    }


    /**
     * Returns array of Custom MediaSizeNames derived from PPD.
     */
    public CustomMediaSizeName[] getCustomMediaSizeNames() {
	initMedia();
	return cupsCustomMediaSNames;
    }


    /**
     * Returns array of MediaPrintableArea derived from PPD.
     */
    public MediaPrintableArea[] getMediaPrintableArea() {
	initMedia();
	return cupsMediaPrintables;
    }

    /**
     * Returns array of MediaTrays derived from PPD.
     */
    public MediaTray[] getMediaTrays() {
	initMedia();
	return cupsMediaTrays;
    }


    /**
     * Initialize media by translating PPD info to PrintService attributes.
     */
    private void initMedia() {
	if (initialized) {
	    return;
	} else {
	    initialized = true;
	}

	if (pageSizes == null) {
	    return;
	}

	cupsMediaPrintables = new MediaPrintableArea[nPageSizes];
	cupsMediaSNames = new MediaSizeName[nPageSizes];	
	cupsCustomMediaSNames = new CustomMediaSizeName[nPageSizes];	
	
	CustomMediaSizeName msn;
	MediaPrintableArea mpa;
	float length, width, x, y, w, h;
	
	// initialize names and printables
	for (int i=0; i<nPageSizes; i++) {
	    // media width and length
	    width = (float)(pageSizes[i*6]/PRINTER_DPI);
	    length = (float)(pageSizes[i*6+1]/PRINTER_DPI);
	    // media printable area
	    x = (float)(pageSizes[i*6+2]/PRINTER_DPI);
	    h = (float)(pageSizes[i*6+3]/PRINTER_DPI);
	    w = (float)(pageSizes[i*6+4]/PRINTER_DPI);
	    y = (float)(pageSizes[i*6+5]/PRINTER_DPI);
	   
	    msn = new CustomMediaSizeName(media[i*2], media[i*2+1],
					  width, length);

	    // add to list of standard MediaSizeNames
	    if ((cupsMediaSNames[i] = msn.getStandardMedia()) == null) {
		// add custom if no matching standard media
		cupsMediaSNames[i] = msn;

		// add this new custom msn to MediaSize array
		if ((width > 0.0) && (length > 0.0)) {
		    new MediaSize(width, length, 
				  Size2DSyntax.INCH, msn);		
		} 
	    }
	    
	    // add to list of custom MediaSizeName 
	    // for internal use of IPPPrintService
	    cupsCustomMediaSNames[i] = msn;

	    mpa = null;
	    try {
		mpa = new MediaPrintableArea(x, y, w, h, 
					     MediaPrintableArea.INCH);
	    } catch (IllegalArgumentException e) {
		if (width > 0 && length > 0) {
		    mpa = new MediaPrintableArea(0, 0, width, length, 
					     MediaPrintableArea.INCH);
		}
	    } 
	    cupsMediaPrintables[i] = mpa;
	}

	// initialize trays
	cupsMediaTrays = new MediaTray[nTrays];		

	MediaTray mt;
	for (int i=0; i<nTrays; i++) {	    
	    mt = new CustomMediaTray(media[(nPageSizes+i)*2],
				     media[(nPageSizes+i)*2+1]);	  
	    cupsMediaTrays[i] = mt;
	}	

    }

    /**
     * Get CUPS default printer using IPP.
     */
    public static String getDefaultPrinter() {
	try {
	    URL url = new URL("http", getServer(), getPort(), "");
	    HttpURLConnection urlConnection = 
		IPPPrintService.getIPPConnection(url);
	    
	    if (urlConnection != null) {
		OutputStream os = urlConnection.getOutputStream();

		AttributeClass attCl[] = {
		    AttributeClass.ATTRIBUTES_CHARSET,
		    AttributeClass.ATTRIBUTES_NATURAL_LANGUAGE,
		    new AttributeClass("requested-attributes", 
				       AttributeClass.TAG_KEYWORD, 
				       "printer-name")
		};

		if (IPPPrintService.writeIPPRequest(os, 
					IPPPrintService.OP_CUPS_GET_DEFAULT, 
					attCl)) {
	
		    HashMap defaultMap = null;
		    InputStream is = urlConnection.getInputStream();
		    HashMap[] responseMap = IPPPrintService.readIPPResponse(
					 is);
		    is.close();
		   
		    if (responseMap.length > 0) {
			defaultMap = responseMap[0];
		    } 
		   
		    if (defaultMap == null) {
			os.close();
			urlConnection.disconnect();
			return null;
		    }

		    AttributeClass attribClass = (AttributeClass)
			defaultMap.get("printer-name");
		   
		    if (attribClass != null) {
			String nameStr = attribClass.getStringValue();
			os.close();
			urlConnection.disconnect();
 			return nameStr;
		    } 
		}
		os.close();
		urlConnection.disconnect();
	    }
	} catch (Exception e) {
	}
	return null;    
    }


    /**
     * Get list of all CUPS printers using IPP.
     */
    public static String[] getAllPrinters() {
	try {
	    URL url = new URL("http", getServer(), getPort(), ""); 

	    HttpURLConnection urlConnection = 
		IPPPrintService.getIPPConnection(url);

	    if (urlConnection != null) {
		OutputStream os = urlConnection.getOutputStream();
		AttributeClass attCl[] = {
		    AttributeClass.ATTRIBUTES_CHARSET,
		    AttributeClass.ATTRIBUTES_NATURAL_LANGUAGE,
		    new AttributeClass("requested-attributes", 
				       AttributeClass.TAG_KEYWORD, 
				       "printer-name")
		};

		if (IPPPrintService.writeIPPRequest(os, 
				IPPPrintService.OP_CUPS_GET_PRINTERS, attCl)) {
	
		    InputStream is = urlConnection.getInputStream();
		    HashMap[] responseMap =  
			IPPPrintService.readIPPResponse(is);
		    
		    is.close();
		    os.close(); 
		    urlConnection.disconnect();

		    if (responseMap == null || responseMap.length == 0) {
			return null;
		    }
		    
		    ArrayList printerNames = new ArrayList();
		    for (int i=0; i< responseMap.length; i++) {
			AttributeClass attribClass = (AttributeClass)
			    responseMap[i].get("printer-name");
			
			if (attribClass != null) {
			    String nameStr = attribClass.getStringValue();
			    printerNames.add(nameStr);
			} 
		    }
		    return (String[])printerNames.toArray(new String[] {});
		} else {
		    os.close();
		    urlConnection.disconnect();
		}
	    }
	    
	} catch (Exception e) {
	}
	return null;
    
    }

    /**
     * Returns CUPS server name.
     */
    public static String getServer() {
	if (libFound) {
	    return getCupsServer();
	}
	return null;
    }

    /**
     * Returns CUPS port number.
     */
    public static int getPort() {
	if (libFound) {
	    return getCupsPort();
	}
	return 0;
    }

    /**
     * Detects if CUPS is running.
     */
    public static boolean isCupsRunning() {
	IPPPrintService.debug_println("libFound "+libFound);
	if (libFound) {
	    IPPPrintService.debug_println("CUPS server "+getServer()+
					  " port "+getPort());
	    return canConnect(getServer(), getPort());
	} else {
	    return false;
	}
    }

   
}


