/*
 * @(#)IcoEncoder.java	1.24 10/03/31
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.io.*;
import java.awt.Image;
import java.awt.image.PixelGrabber;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.javaws.jnl.*;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.ImageLoader;
import com.sun.deploy.util.PerfLogger;

/* IcoEncoder create a windows icon file (.ico) from a Image */
public class IcoEncoder {

    private final static boolean DEBUG = false;

    private Image _awtImage;
    private byte _size;
    private byte[] _andData;
    private byte[] _xorData;

    /** 
     * IcoEncoder - the encoder for a particular image and size.
     * the method Encoder.createBitmaps will fill in the other 
     * two instance vars _andData and _xorData (above).
     */
    public IcoEncoder(Image awtImage, byte size) {
        _size = size;
        _awtImage = awtImage;
    }

    /**
     * Returns the path to the icon to use for the application identified
     * by <code>ld</code>. This will download and create a new BMP if
     */
    public static String getIconPath(LaunchDesc ld, boolean isDesktop) {
        return getIconPath(ld);
    }

    final static int DEFAULT_SIZES [] = {32, 16, 48, 64};

    /**
     * We only create one icon, if multiple icon elements are listed in 
     * the LaunchDesc, we may create one .ico files with multiple images in it.
     */
    public static String getIconPath(LaunchDesc ld) {
        ArrayList sizes = new ArrayList();

        Integer desktopSize =
            new Integer(Config.getInstance().getSystemShortcutIconSize(true));
        Integer menuSize =
            new Integer(Config.getInstance().getSystemShortcutIconSize(false));
        for (int i=0; i<DEFAULT_SIZES.length; i++) {
            Integer size = new Integer(DEFAULT_SIZES[i]);
            if (!sizes.contains(size)) { 
                sizes.add(size); 
            }
        }
        if (!sizes.contains(desktopSize)) {
            sizes.add(desktopSize);
        }
        if (!sizes.contains(menuSize)) {
            sizes.add(menuSize);
        }

        Iterator it = sizes.iterator();
        IconDesc iconIDs[] = new IconDesc[6];
        byte iconSizes[] = new byte[6];
        int iconCount = 0;

        while (it.hasNext()) {
            int sz = ((Integer)it.next()).intValue();
            if (sz < 16) { sz = 16; }
            if (sz > 64) { sz = 64; }

            // first try for kind="shortcut"
            IconDesc id = ld.getInformation().getIconLocation(sz,
                                              IconDesc.ICON_KIND_SHORTCUT);
            // if none there use default
            if (id == null) {
                id = ld.getInformation().getIconLocation(sz,
                                         IconDesc.ICON_KIND_DEFAULT);
            }
            if (id != null) {
                boolean found = false;
                for (int i=0; (i<iconCount && !found); i++) {
                    if (id.equals(iconIDs[i])) { 
                        found = true; // don't to make two scales of same image
                    }
                }
                if (!found) {
                    iconIDs[iconCount] = id;
                    // if it has given size - use that
                    int w = id.getWidth();
                    int h = id.getHeight();
                    if (w == h && w >= 16 && w <= 64) {
                        iconSizes[iconCount] = (byte) w;
                    } else {
                        iconSizes[iconCount] = (byte) sz;
                    }
                    iconCount++;
                }
            }
        }

        String finalIcoFilename = null;
        IcoEncoder encoders[] = new IcoEncoder[6];

        for (int i=0; i<iconCount; i++) {
            String filename = null;
            File file = null;
            try {
                File cachedFile = DownloadEngine.getCachedShortcutImage(
                              iconIDs[i].getLocation(), iconIDs[i].getVersion());
                if (cachedFile == null) {
                    cachedFile = DownloadEngine.getUpdatedShortcutImage(
                             iconIDs[i].getLocation(), iconIDs[i].getVersion());
                }
                if (cachedFile != null) {
                    if (Config.getInstance().isPlatformIconType(
                        iconIDs[i].getLocation().toString())) {
                        filename = cachedFile.toString();
                        file = cachedFile;
                    } else {
                        filename = cachedFile.getPath() + ".ico";
                        file = new File(filename);
                    }
                    if (file.exists()) {
                        // we have already done all this and can exit
                        finalIcoFilename = filename;
                        return finalIcoFilename;
                    }
                    if (finalIcoFilename == null) {
                        finalIcoFilename = filename;
                    } 
                    PerfLogger.setTime("before ico creation for " + filename);
                    Image awtImage = ImageLoader.getInstance().loadImage(
                                                 cachedFile.getPath());
                    encoders[i] = new IcoEncoder(awtImage, iconSizes[i]);
                    encoders[i].createBitmaps();
                    PerfLogger.setTime("after ico creation for " + filename);
                }
            } catch (IOException ioe) {
                Trace.ignored(ioe);
            }
        }
        // now put them all together
        BufferedOutputStream bos = null;
        if ((finalIcoFilename != null) && (iconCount > 0)) {
            int offset[] = new int[6];
            offset[0] = 6 + 16 * iconCount;
            for (int i=1; i<iconCount; i++) {
                offset[i] = offset[i-1] + 40 +
                    encoders[i-1].getXorData().length +
                    encoders[i-1].getAndData().length;
            }

            try {
                bos = new BufferedOutputStream(
                    new FileOutputStream(new File (finalIcoFilename)));
         
                IcoStreamWriter isw = encoders[0].getIcoStreamWriter(bos);
                isw.writeIcoHeader(iconCount);
                for (int i=0; i<iconCount; i++) {
                    encoders[i].writeIconDirEntry(isw, offset[i]);
                }
                for (int i=0; i<iconCount; i++) {
                    encoders[i].writeInfoHeader(isw);
                    bos.write(encoders[i].getXorData());
                    bos.write(encoders[i].getAndData());
                }
                bos.flush();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            } finally {
                if (bos != null) {
                    try {
                         bos.close();
                    } catch (Exception e) {
                    }
                }
            }
            if (DEBUG) {
                try {
                    showIconFile(new File(finalIcoFilename));
                } catch (Exception e) { }
            }  
        }
        return finalIcoFilename;
    }

    /*
     * This is a special case version for Association data ico.
     * where the URL and version is available, but not from the ld
     * here we want to create an ico with one 32x32 image.
     */
    public static String getIconPath(URL loc, String version) {
        String filename = null;
        File file;
        IcoEncoder encoder = null;
        try {
            File cachedFile = 
                DownloadEngine.getCachedShortcutImage(loc, version);
            if (cachedFile == null) {
                cachedFile = 
                    DownloadEngine.getUpdatedShortcutImage(loc, version);
            }
            if (cachedFile != null) {
                if (Config.getInstance().isPlatformIconType(loc.toString())) {
                    filename = cachedFile.toString();
                    file = cachedFile;
                } else {
                    filename = cachedFile.getPath() + ".ico";
                    file = new File(filename);
                }
                if (file.exists()) {
                    // we have already done all this and can exit
                    return filename;
                }
                Image awtImage = ImageLoader.getInstance().loadImage(
                                             cachedFile.getPath());
                encoder = new IcoEncoder(awtImage, (byte)32); // 32 bit image
                encoder.createBitmaps();
            }
        } catch (IOException ioe) {
            Trace.ignored(ioe);
        }
        if (filename != null && encoder != null) {
            BufferedOutputStream bos = null;
            int offset = 6 + 16 * 1;  // one image here;
            try {
                bos = new BufferedOutputStream(
                    new FileOutputStream(new File (filename)));
                IcoStreamWriter isw = encoder.getIcoStreamWriter(bos);
                isw.writeIcoHeader(1);
                encoder.writeIconDirEntry(isw, offset);
                encoder.writeInfoHeader(isw);
                bos.write(encoder.getXorData());
                bos.write(encoder.getAndData());
                bos.flush();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            } finally {
                if (bos != null) {
                    try {
                         bos.close();
                    } catch (Exception e) {
                    }
                }
            }
            if (DEBUG) {
                try {
                    showIconFile(new File(filename));
                } catch (Exception e) { }
            }
        }
        return filename;
    }

    /*
     * CreateBitmap:
     *    Object is to create the 24 bit XOR bitmap and the 1 bit AND bitmap
     * used in the ico format.
     *
     *    It is noted that the use of Image.getScaledInstance(), and 
     * PixelGrabber are Obsolete methos that should be replaced in a future
     * cleanup of this code by more modern java2d interfaces.
     */
    private void createBitmaps() throws IOException {

        int w = _size; // icon width and height
        int h = _size;
        int scanSize = getXorScanSize((int) _size);
        int andScanSize = getAndScanSize((int) _size);
      
        int k = 0;

        // original image pixels
        int[] pixels = new int [w*h];
        
        // XOR bitmap
        byte[] xorPixels = new byte [h * scanSize];
        _xorData = new byte[h * scanSize];
        
        // AND bitmap
        byte[] andPixels = new byte[h * andScanSize];
        _andData = new byte[h * andScanSize];

        Image scaledImage = _awtImage.getScaledInstance( w, h,
            _awtImage.SCALE_SMOOTH);

        // Scale the orignal image to icon size and grab the pixels
        PixelGrabber pg = new PixelGrabber(scaledImage, 0, 0, w, h, 
                                                        pixels, 0, w);

        try {
            pg.grabPixels();
        } catch (InterruptedException e) {            
            e.printStackTrace();
        }

        // Reorganize original image pixels into XOR bitmap and AND bitmap
        for (int y=0; y<h; y++) {
            int andScan = y * andScanSize;
            int andIndex = 0;
            int xorScan = y * scanSize;
            int xorIndex = 0;
            byte maskByte=0;
            int bitcount=0;

            for (int x=0; x<w; x++) {
                int j=(y*w) + x;

                int alpha = (pixels[j] >> 24) & 0xff;
                int red = (pixels[j] >> 16) & 0xff;
                int green = (pixels[j] >> 8) & 0xff;
                int blue = (pixels[j]) & 0xff;
           
                // AND bitmap (1bit for each pixel)
                if (alpha == 0) { 
                    maskByte |= (0x80 >> bitcount);
                }    
                bitcount++;
                if ((bitcount==8) || (x == w-1)) {
                    andPixels[andScan + andIndex++] = maskByte;
                    maskByte=0;
                    bitcount=0;
                }

                // XOR bitmap  (windows bitmap uses BGR format)
                xorPixels[xorScan + xorIndex++] = (byte)blue;
                xorPixels[xorScan + xorIndex++] = (byte)green;
                xorPixels[xorScan + xorIndex++] = (byte)red;
            }
            // pad out AND bitmap;
            while (andIndex < andScanSize) {
                andPixels[andScan + andIndex++] = 0;
            }

            // pad out XOR bitmap;
            while (xorIndex < scanSize) {
                xorPixels[xorScan + xorIndex++] = 0;
            }
        }

        int ssi;  // source scan index
        int dsi;  // destination scan index

        // reverse the scanline for proper orientation
        for (int scan=0; scan<h; scan++) {
            
            // XOR bitmap
            dsi = scan * scanSize;
            ssi = (h - scan - 1) * scanSize;
          
            // XOR bitmap
            for (int z=0; z<scanSize; z++) {
                _xorData[dsi+z] = xorPixels[ssi+z];
            }

            // AND bitmap
            dsi = scan * andScanSize;
            ssi = (h - scan - 1) * andScanSize;
          
            // AND bitmap
            for (int z=0; z<andScanSize; z++) {
                _andData[dsi+z] = andPixels[ssi+z];
            }
        }
    }

    private byte[] getXorData() {
        return _xorData;
    }

    private byte[] getAndData() {
        return _andData;
    }

    private void writeInfoHeader(IcoStreamWriter isw) throws IOException {

        // size of header
        isw.writeDWord(40);
        
        // width 
        isw.writeDWord(_size);
        
        // height (of both XOR and AND bitmap)
        isw.writeDWord(2 * _size);
        
        // number of planes
        isw.writeWord(1);
        
        // Bits Per Pixel
        isw.writeWord(24);

        // compression
        isw.writeDWord(0);
        
        // imageSize
        isw.writeDWord(0);
        
        // xPelsPerMeter
        isw.writeDWord(0);
        
        // yPelsPerMeter
        isw.writeDWord(0);
        
        // Colors Used
        isw.writeDWord(0);
        
        // Colors Important
        isw.writeDWord(0);
    }

    private void writeIconDirEntry(IcoStreamWriter isw, int dataOffset) 
        throws IOException {
        int andScanSize = getAndScanSize((int) _size);
        int scanSize = getXorScanSize(_size);

        try {
            // width
            isw.write( _size);

            // height
            isw.write(_size);
        
            // number of colors ( 0 means 256)
            isw.write((byte)0);

            // reserved
            isw.write((byte)0);

            // planes
            isw.writeWord((byte)1);

            // bit count (1,4,8)
            isw.writeWord(24);
        
            // Size in bytes (xor size + and size + 40)
            int length = (_size * scanSize) + (_size * andScanSize) + 40;
            isw.writeDWord(length);

            // FileOffset  -- FIXME
            isw.writeDWord(dataOffset);
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }

    private IcoStreamWriter getIcoStreamWriter(BufferedOutputStream bos) {
        return new IcoStreamWriter(bos);
    }

    private class IcoStreamWriter {

        BufferedOutputStream _bos;

        private IcoStreamWriter(BufferedOutputStream bos) {
            _bos = bos;
        }
        
        private void writeIcoHeader(int count) throws IOException {
            short output;

            try {
                //write "reserved" Word
                writeWord(0);

                // write "type" Word
                writeWord(1);

                // write "number of entries"
                writeWord(count);

            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }

        public void write(byte b) throws IOException {
            _bos.write(b);
        }

        // Methods for little-endian writing
        public void writeWord(int word) throws IOException {
            _bos.write(word & 0xff);
            _bos.write((word & 0xff00) >> 8);
        }
    
        public void writeDWord(int dword) throws IOException {
            _bos.write(dword & 0xff);
            _bos.write((dword & 0xff00) >> 8);
            _bos.write((dword & 0xff0000) >> 16);
            _bos.write((dword & 0xff000000) >> 24);
        } 
    }

    private static int getAndScanSize(int size) {
        int bytes = (size + 7) / 8;
        // each scanline is multiple of 4 bytes ( one bit per pel )
        int dwords = (4 * ((bytes + 3)/4));
        return dwords;
    }

    private static int getXorScanSize(int size) {
        int bytes = size * 3 ;
        // each scanline is multiple of 4 bytes (3 bytes per pel )
        int dwords = (4 * ((bytes + 3)/4));
        return dwords;
    }

/*
** debugging - only call this in debug mode
*/
    public static void showIconFile(File f) {

Trace.println("Icon: " + f);
      try {
        FileInputStream fis = new FileInputStream(f);
        byte b[] = new byte[16];
        int len;
        byte h[] = new byte [6];
        fis.read(h);
Trace.println("header: " + h[0] + ", " + h[1] + ", " + h[2] + ", " + 
      h[3] + ", " + h[4] + ", " + h[5]);
        byte count = h[4];
        int i;
        for (i=0; i<count; i++) {
            fis.read(b);
Trace.println("Dir entry " + i + ": " +
              b[0] + ", " + b[1] + ", " + b[2] + ", " + b[3] + ", " + 
              b[4] + ", " + b[5] + ", " + b[6] + ", " + b[7] + ", " +
              b[8] + ", " + b[9] + ", " + b[10] + ", " + b[11] + ", " +
              b[12] + ", " + b[13] + ", " + b[14] + ", " + b[15]);
        }
        i = 0;
Trace.println("InfoHeader: ");
        byte ih[] = new byte[40];
        fis.read(ih);
        for (i=0; i<40; i++) { Trace.print(ih[i] + ","); }
        Trace.println("\n");
Trace.println("the rest: ");
        while ((len = fis.read(b)) > 0) {
Trace.println(" line " + i++ + " : " +
              b[0] + ", " + b[1] + ", " + b[2] + ", " + b[3] + ", " + 
              b[4] + ", " + b[5] + ", " + b[6] + ", " + b[7] + ", " +
              b[8] + ", " + b[9] + ", " + b[10] + ", " + b[11] + ", " +
              b[12] + ", " + b[13] + ", " + b[14] + ", " + b[15]);
        }

      } catch (Exception e) {
      }
    }

}
