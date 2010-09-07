/*
 * @(#)WDataTransferer.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Image;
import java.awt.Graphics2D;
import java.awt.Transparency;

import java.awt.color.ColorSpace;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.FlavorTable;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;

import java.awt.geom.AffineTransform;

import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.DataBufferInt;
import java.awt.image.DirectColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

import java.net.URL;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.SortedMap;

import sun.awt.Mutex;
import sun.awt.datatransfer.DataTransferer;
import sun.awt.datatransfer.ToolkitThreadBlockedHandler;

import sun.awt.image.ImageRepresentation;
import sun.awt.image.ToolkitImage;

/**
 * Platform-specific support for the data transfer subsystem.
 *
 * @author David Mendenhall
 * @author Danila Sinopalnikov
 * @version 1.23, 12/19/03
 *
 * @since 1.3.1
 */
public class WDataTransferer extends DataTransferer {
    private static final String[] predefinedClipboardNames = {
        "",
        "TEXT",
        "BITMAP",
        "METAFILEPICT",
        "SYLK",
        "DIF",  
        "TIFF",
        "OEM TEXT",
        "DIB",
        "PALETTE",
        "PENDATA",
        "RIFF",
        "WAVE",
        "UNICODE TEXT",
        "ENHMETAFILE",
        "HDROP",
        "LOCALE"
    };

    private static final Map predefinedClipboardNameMap;
    static {
        Map tempMap = new HashMap(16, 1.0f);
        for (int i = 1; i < predefinedClipboardNames.length; i++) {
            tempMap.put(predefinedClipboardNames[i], new Long(i));
        }
        predefinedClipboardNameMap = Collections.synchronizedMap(tempMap);
    }

    /**
     * from winuser.h
     */
    public static final int CF_TEXT = 1;
    public static final int CF_METAFILEPICT = 3;
    public static final int CF_DIB = 8;
    public static final int CF_ENHMETAFILE = 14;
    public static final int CF_HDROP = 15;
    public static final int CF_LOCALE = 16;

    public static final long CF_HTML = registerClipboardFormat("HTML Format");
    public static final long CFSTR_INETURL = registerClipboardFormat("UniformResourceLocator");
    public static final long CF_PNG = registerClipboardFormat("PNG");
    public static final long CF_JFIF = registerClipboardFormat("JFIF");
    
    private static final Long L_CF_LOCALE = (Long)
      predefinedClipboardNameMap.get(predefinedClipboardNames[CF_LOCALE]);

    private static final DirectColorModel directColorModel = 
        new DirectColorModel(24,
                             0x00FF0000,  /* red mask   */
                             0x0000FF00,  /* green mask */
                             0x000000FF); /* blue mask  */   

    private static final int[] bandmasks = new int[] {
        directColorModel.getRedMask(),
        directColorModel.getGreenMask(),
        directColorModel.getBlueMask() };

    /**
     * Singleton constructor
     */
    private WDataTransferer() {
    }

    private static WDataTransferer transferer;

    public static WDataTransferer getInstanceImpl() {
        if (transferer == null) {
            synchronized (WDataTransferer.class) {
                if (transferer == null) {            
                    transferer = new WDataTransferer();
                }
            }
        }
        return transferer;
    }

    public SortedMap getFormatsForFlavors(DataFlavor[] flavors, FlavorTable map) {
        SortedMap retval = super.getFormatsForFlavors(flavors, map);

        // The Win32 native code does not support exporting LOCALE data, nor
        // should it.
        retval.remove(L_CF_LOCALE);

        return retval;
    }

    public String getDefaultUnicodeEncoding() {
        return "utf-16le";
    }

    public byte[] translateTransferable(Transferable contents,
                                        DataFlavor flavor,
                                        long format) throws IOException
    {
        byte[] bytes = super.translateTransferable(contents, flavor, format);

        if (format == CF_HTML) {
            bytes = HTMLSupport.convertToHTMLFormat(bytes);
        }

        return bytes;
    }

    protected Object translateBytesOrStream(InputStream str, byte[] bytes,
                                            DataFlavor flavor, long format,
                                            Transferable localeTransferable)
        throws IOException
    {
        if (format == CF_HTML && flavor.isFlavorTextType()) {
            if (str == null) {
                str = new ByteArrayInputStream(bytes);
                bytes = null;
            }

            str = new HTMLDecodingInputStream(str);
        }

        if (format == CFSTR_INETURL && 
            URL.class.equals(flavor.getRepresentationClass()))
        {
            if (bytes == null) {
                bytes = inputStreamToByteArray(str);
                str = null;
            }
            String charset = getDefaultTextCharset();
            if (localeTransferable != null && localeTransferable.
                isDataFlavorSupported(javaTextEncodingFlavor))
            {
                try {
                    charset = new String((byte[])localeTransferable.
                                   getTransferData(javaTextEncodingFlavor),
                                   "UTF-8");
                } catch (UnsupportedFlavorException cannotHappen) {
                }
            }
            return new URL(new String(bytes, charset));
        }

        return super.translateBytesOrStream(str, bytes, flavor, format,
                                            localeTransferable);
    }

    public boolean isLocaleDependentTextFormat(long format) {
        return format == CF_TEXT || format == CFSTR_INETURL;
    }

    public boolean isFileFormat(long format) {
        return format == CF_HDROP;
    }

    protected Long getFormatForNativeAsLong(String str) {
        Long format = (Long)predefinedClipboardNameMap.get(str);
        if (format == null) {
            format = new Long(registerClipboardFormat(str));
        } 
        return format;
    }

    protected String getNativeForFormat(long format) {
        return (format < predefinedClipboardNames.length)
            ? predefinedClipboardNames[(int)format]
            : getClipboardFormatName(format);
    }

    private final ToolkitThreadBlockedHandler handler = 
        new WToolkitThreadBlockedHandler();

    public ToolkitThreadBlockedHandler getToolkitThreadBlockedHandler() {
        return handler;
    }

   /**
     * Calls the Win32 RegisterClipboardFormat function to register
     * a non-standard format.
     */
    private static native long registerClipboardFormat(String str);

    /**
     * Calls the Win32 GetClipboardFormatName function which is
     * the reverse operation of RegisterClipboardFormat.
     */
    private static native String getClipboardFormatName(long format);

    public boolean isImageFormat(long format) {
        return format == CF_DIB || format == CF_ENHMETAFILE ||
               format == CF_METAFILEPICT || format == CF_PNG ||
               format == CF_JFIF;
    }

    protected byte[] imageToPlatformBytes(Image image, long format)
      throws IOException {
        String mimeType = null;
        if (format == CF_PNG) {
            mimeType = "image/png";
        } else if (format == CF_JFIF) {
            mimeType = "image/jpeg";
        }
        if (mimeType != null) {
            return imageToStandardBytes(image, mimeType);
        }

        int width = 0; 
        int height = 0;

        if (image instanceof ToolkitImage) {
            ImageRepresentation ir = ((ToolkitImage)image).getImageRep();
            ir.reconstruct(ImageObserver.ALLBITS);
            width = ir.getWidth();
            height = ir.getHeight();
        } else {
            width = image.getWidth(null);
            height = image.getHeight(null);
        }

        // Fix for 4919639.
        // Some Windows native applications (e.g. clipbrd.exe) do not handle
        // 32-bpp DIBs correctly.
        // As a workaround we switched to 24-bpp DIBs.
        // MSDN prescribes that the bitmap array for a 24-bpp should consist of
        // 3-byte triplets representing blue, green and red components of a
        // pixel respectively. Additionally each scan line must be padded with
        // zeroes to end on a LONG data-type boundary. LONG is always 32-bit.
        // We render the given Image to a BufferedImage of type TYPE_3BYTE_BGR
        // with non-default scanline stride and pass the resulting data buffer
        // to the native code to fill the BITMAPINFO structure.
        int mod = (width * 3) % 4;
        int pad = mod > 0 ? 4 - mod : 0;

        ColorSpace cs = ColorSpace.getInstance(ColorSpace.CS_sRGB);
        int[] nBits = {8, 8, 8};
        int[] bOffs = {2, 1, 0};
        ColorModel colorModel = 
            new ComponentColorModel(cs, nBits, false, false,
                                    Transparency.OPAQUE, DataBuffer.TYPE_BYTE);
        WritableRaster raster = 
            Raster.createInterleavedRaster(DataBuffer.TYPE_BYTE, width, height,
                                           width * 3 + pad, 3, bOffs, null);        
        
        BufferedImage bimage = new BufferedImage(colorModel, raster, false, null);

        // Some Windows native applications (e.g. clipbrd.exe) do not understand 
        // top-down DIBs.
        // So we flip the image vertically and create a bottom-up DIB. 
        AffineTransform imageFlipTransform = 
            new AffineTransform(1, 0, 0, -1, 0, height);

        Graphics2D g2d = bimage.createGraphics();

        try {
            g2d.drawImage(image, imageFlipTransform, null);
        } finally {
            g2d.dispose();
        }

        DataBufferByte buffer = (DataBufferByte)raster.getDataBuffer();

        byte[] imageData = buffer.getData();
        return imageDataToPlatformImageBytes(imageData, width, height, format);
    }

   /**
    * Returns a byte array which contains data special for the given format
    * and for the given image data.
    */
    private native byte[] imageDataToPlatformImageBytes(byte[] imageData, 
                                                        int width, int height,
                                                        long format);

    /**
     * Translates either a byte array or an input stream which contain
     * platform-specific image data in the given format into an Image.
     */
    protected Image platformImageBytesOrStreamToImage(InputStream str, 
                                                      byte[] bytes, 
                                                      long format) 
      throws IOException {
        String mimeType = null;
        if (format == CF_PNG) {
            mimeType = "image/png";
        } else if (format == CF_JFIF) {
            mimeType = "image/jpeg";
        }
        if (mimeType != null) {
            return standardImageBytesOrStreamToImage(str, bytes, mimeType);
        }

        if (bytes == null) {
            bytes = inputStreamToByteArray(str);
        }

        int[] imageData = platformImageBytesToImageData(bytes, format);
        if (imageData == null) {
            throw new IOException("data translation failed");
        }
        
        int len = imageData.length - 2;
        int width = imageData[len];
        int height = imageData[len + 1];
        
        DataBufferInt buffer = new DataBufferInt(imageData, len);
        WritableRaster raster = Raster.createPackedRaster(buffer, width,
                                                          height, width, 
                                                          bandmasks, null);
            
        return new BufferedImage(directColorModel, raster, false, null);
    }

    /**
     * Translates a byte array which contains platform-specific image data in
     * the given format into an integer array which contains pixel values in 
     * ARGB format. The two last elements in the array specify width and
     * height of the image respectively.
     */
    private native int[] platformImageBytesToImageData(byte[] bytes,
                                                       long format)
      throws IOException;

    protected native String[] dragQueryFile(byte[] bytes);
}

final class WToolkitThreadBlockedHandler extends Mutex 
                       implements ToolkitThreadBlockedHandler {

    public void enter() {
        if (!isOwned()) {
            throw new IllegalMonitorStateException();
        }
        unlock();
        startSecondaryEventLoop();
        lock();
    }

    public void exit() {
        if (!isOwned()) {
            throw new IllegalMonitorStateException();
        }
        WToolkit.quitSecondaryEventLoop();
    }

    private native void startSecondaryEventLoop();
}

final class HTMLSupport {
    public static final String ENCODING = "UTF-8";

    public static final String VERSION = "Version:";
    public static final String START_HTML = "StartHTML:";
    public static final String END_HTML = "EndHTML:";
    public static final String START_FRAGMENT = "StartFragment:";
    public static final String END_FRAGMENT = "EndFragment:";
    public static final String START_FRAGMENT_CMT = "<!--StartFragment-->";
    public static final String END_FRAGMENT_CMT = "<!--EndFragment-->";
    public static final String EOLN = "\r\n";

    private static final String VERSION_NUM = "0.9";
    private static final String HTML_START_END = "-1";

    private static final int PADDED_WIDTH = 10;

    private static final int HEADER_LEN =
        VERSION.length() + VERSION_NUM.length() + EOLN.length() +
        START_HTML.length() + HTML_START_END.length() + EOLN.length() +
        END_HTML.length() + HTML_START_END.length() + EOLN.length() +
        START_FRAGMENT.length() + PADDED_WIDTH + EOLN.length() +
        END_FRAGMENT.length() + PADDED_WIDTH + EOLN.length() +
        START_FRAGMENT_CMT.length() + EOLN.length();
    private static final String HEADER_LEN_STR =
        toPaddedString(HEADER_LEN, PADDED_WIDTH);

    private static final String TRAILER = END_FRAGMENT_CMT + EOLN + '\0';

    private static String toPaddedString(int n, int width) {
        String string = "" + n;
        int len = string.length();
        if (n >= 0 && len < width) {
            char[] array = new char[width - len];
            Arrays.fill(array, '0');
            StringBuffer buffer = new StringBuffer();
            buffer.append(array);
            buffer.append(string);
            string = buffer.toString();
        }
        return string;
    }
    
    public static byte[] convertToHTMLFormat(byte[] bytes) {
        StringBuffer header = new StringBuffer(HEADER_LEN);
        header.append(VERSION);
        header.append(VERSION_NUM);
        header.append(EOLN);
        header.append(START_HTML);
        header.append(HTML_START_END);
        header.append(EOLN);
        header.append(END_HTML);
        header.append(HTML_START_END);
        header.append(EOLN);
        header.append(START_FRAGMENT);
        header.append(HEADER_LEN_STR);
        header.append(EOLN);
        header.append(END_FRAGMENT);
        // Strip terminating NUL byte from array
        header.append(toPaddedString(HEADER_LEN + bytes.length - 1,
                                     PADDED_WIDTH));
        header.append(EOLN);
        header.append(START_FRAGMENT_CMT);
        header.append(EOLN);

        byte[] headerBytes = null, trailerBytes = null;

        try {
            headerBytes = new String(header).getBytes(ENCODING);
            trailerBytes = TRAILER.getBytes(ENCODING);
        } catch (UnsupportedEncodingException cannotHappen) {
        }

        byte[] retval = new byte[headerBytes.length + bytes.length - 1 +
                                 trailerBytes.length];

        System.arraycopy(headerBytes, 0, retval, 0, headerBytes.length);
        System.arraycopy(bytes, 0, retval, headerBytes.length,
                       bytes.length - 1);
        System.arraycopy(trailerBytes, 0, retval,
                         headerBytes.length + bytes.length - 1,
                         trailerBytes.length);

        return retval;
    }
}

/**
 * This stream takes an InputStream which provides data in CF_HTML format,
 * strips off the description and context to extract the original HTML data.
 */
class HTMLDecodingInputStream extends InputStream {

    private final BufferedInputStream bufferedStream;
    private boolean descriptionParsed = false;
    private boolean closed = false;
    private int index;
    private int end;

     // InputStreamReader uses an 8K buffer. The size is not customizable.
    public static final int BYTE_BUFFER_LEN = 8192;

    // CharToByteUTF8.getMaxBytesPerChar returns 3, so we should not buffer
    // more chars than 3 times the number of bytes we can buffer.
    public static final int CHAR_BUFFER_LEN = BYTE_BUFFER_LEN / 3;

    private static final String FAILURE_MSG =
        "Unable to parse HTML description: ";
    private static final String INVALID_MSG = " invalid";
    
    public HTMLDecodingInputStream(InputStream bytestream) throws IOException {
        bufferedStream = new BufferedInputStream(bytestream, BYTE_BUFFER_LEN);
    }

    private void parseDescription() throws IOException {
        bufferedStream.mark(BYTE_BUFFER_LEN);

        BufferedReader bufferedReader = new BufferedReader
            (new InputStreamReader(bufferedStream, HTMLSupport.ENCODING),
             CHAR_BUFFER_LEN);
        String version = bufferedReader.readLine().trim();
        if (version == null || !version.startsWith(HTMLSupport.VERSION)) {
            // Not MS-compliant HTML text. Return raw text from read().
            index = 0;
            end = -1;
            bufferedStream.reset();
            return;
        }

        String input;
        boolean startHTML, endHTML, startFragment, endFragment;
        startHTML = endHTML = startFragment = endFragment = false;

        try {
            do {
                input = bufferedReader.readLine().trim();
                if (input == null) {
                    close();
                    throw new IOException(FAILURE_MSG);
                } else if (input.startsWith(HTMLSupport.START_HTML)) {
                    int val = Integer.parseInt
                        (input.substring(HTMLSupport.START_HTML.length(),
                                         input.length()).trim());
                    if (val >= 0) {
                        index = val;
                        startHTML = true;
                    } else if (val != -1) {
                        close();
                        throw new IOException(FAILURE_MSG +
                                              HTMLSupport.START_HTML +
                                              INVALID_MSG);
                    }
                } else if (input.startsWith(HTMLSupport.END_HTML)) {
                    int val = Integer.parseInt
                        (input.substring(HTMLSupport.END_HTML.length(),
                                         input.length()).trim());
                    if (val >= 0) {
                        end = val;
                        endHTML = true;
                    } else if (val != -1) {
                        close();
                        throw new IOException(FAILURE_MSG +
                                              HTMLSupport.END_HTML +
                                              INVALID_MSG);
                    }
                } else if (!startHTML && !endHTML &&
                           input.startsWith(HTMLSupport.START_FRAGMENT)) {
                    index = Integer.parseInt
                        (input.substring(HTMLSupport.START_FRAGMENT.length(),
                                         input.length()).trim());
                    if (index < 0) {
                        close();
                        throw new IOException(FAILURE_MSG +
                                              HTMLSupport.START_FRAGMENT +
                                              INVALID_MSG);
                    }
                    startFragment = true;
                } else if (!startHTML && !endHTML &&
                           input.startsWith(HTMLSupport.END_FRAGMENT)) {
                    end = Integer.parseInt
                        (input.substring(HTMLSupport.END_FRAGMENT.length(),
                                         input.length()).trim());
                    if (end < 0) {
                        close();
                        throw new IOException(FAILURE_MSG +
                                              HTMLSupport.END_FRAGMENT +
                                              INVALID_MSG);
                    }
                    endFragment = true;
                }
            } while (!((startHTML && endHTML) ||
                       (startFragment && endFragment)));
        } catch (NumberFormatException e) {
            close();
            throw new IOException(FAILURE_MSG + e);
        }

        bufferedStream.reset();

        for (int i = 0; i < index; i++) {
            if (bufferedStream.read() == -1) {
                close();
                throw new IOException(FAILURE_MSG +
                                      "Byte stream ends in description.");
            }
        }
    }

    public int read() throws IOException {
        if (closed) {
            throw new IOException("Stream closed");
        }

        if (!descriptionParsed) {
            parseDescription(); // initializes 'index' and 'end'
            descriptionParsed = true;
        }

        if (end != -1 && index >= end) {
            return -1;
        }

        int retval = bufferedStream.read();
        if (retval == -1) {
            index = end = 0; // so future read() calls will fail quickly
            return -1;
        } else {
            index++;
            return retval;
        }
    }

    public void close() throws IOException {
        if (!closed) {
            closed = true;
            bufferedStream.close();
        }
    }
}
