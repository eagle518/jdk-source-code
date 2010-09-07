/*
 * %W% %E%
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.image.*;
import java.util.*;
import java.beans.*;

/**
 * Implements Theme Support for Windows XP.
 *
 * @version %I% %G%
 * @author Sergey Salishev
 */
public class ThemeReader {
    private static native int[] getBitmapResource(String path, String resource);
    private static native String getTextResourceByName(String path, String resource, String resType);
    private static native String getTextResourceByInt(String path, int resource, String resType);


    private static volatile boolean isLoaded = false;

    private static final String textResourcesKey = "win.xpstyle.resources.strings";
    private static Map textResources;

    private static final String imageResourcesKey = "win.xpstyle.resources.images";
    private static Map imageResources;

    private static final String dllNameKey = "win.xpstyle.dllName";
    private static final String sizeNameKey = "win.xpstyle.sizeName";
    private static final String colorNameKey = "win.xpstyle.colorName";
    
    /* This method is called from synchronized Toolkit.getDesktopProperty
       no need for additional sync 
     */
    static Map loadResources(String name) {
        if (!isLoaded) {
            loadResources();
        }
        if (name.equals(textResourcesKey)) {
            return textResources;
        }
        if (name.equals(imageResourcesKey)) {
            return imageResources;
        }
        return null;
    }

    static boolean isXPThemeResources(String name) {
        return name.equals(textResourcesKey) ||
               name.equals(imageResourcesKey);
    }

    private static class StyleChangeListener implements PropertyChangeListener {
        public void propertyChange(PropertyChangeEvent evt) {
            WToolkit toolkit = (WToolkit)Toolkit.getDefaultToolkit();
            synchronized (toolkit) {
                toolkit.clearDesktopProperty(textResourcesKey);
                toolkit.clearDesktopProperty(imageResourcesKey);
                textResources = null;
                imageResources = null;
                isLoaded = false;
            }
        }
    }

    private static StyleChangeListener styleChangeListener = null;
           
    private static void loadResources() {
        Toolkit toolkit = Toolkit.getDefaultToolkit();
        String styleFile = (String)toolkit.getDesktopProperty(dllNameKey);
        String sizeName  = (String)toolkit.getDesktopProperty(sizeNameKey);
        String colorName = (String)toolkit.getDesktopProperty(colorNameKey);
        if (styleFile != null && sizeName != null && colorName != null) {
            textResources = new TextResources(styleFile, sizeName, colorName);
            imageResources = new ImageResources(styleFile);
            isLoaded = true;
            if (styleChangeListener == null) {
                styleChangeListener = new StyleChangeListener();
                toolkit.addPropertyChangeListener(dllNameKey, styleChangeListener);
                toolkit.addPropertyChangeListener(colorNameKey, styleChangeListener);
                toolkit.addPropertyChangeListener(sizeNameKey, styleChangeListener);
            }
        }
    }
        
    private static class TextResources extends HashMap {
        TextResources(String styleFile, String sizeName, String colorName) {            
            String themeFile = null;
            String[] sizeNames =
                splitTextResource(getTextResourceByInt(styleFile, 1, "SIZENAMES"));
            String[] colorNames =
                splitTextResource(getTextResourceByInt(styleFile, 1, "COLORNAMES"));
            String[] themeFileNames =
                splitTextResource(getTextResourceByInt(styleFile, 1, "FILERESNAMES"));
            exit: 
            for (int color = 0; color < colorNames.length; color++) {
                for (int size = 0; size < sizeNames.length; size++) {
                    if (sizeName.equals(sizeNames[size]) &&
                        colorName.equals(colorNames[color]) &&
                        (color * sizeNames.length + size) < themeFileNames.length) 
                    {
                        themeFile = themeFileNames[color * sizeNames.length + size];
                        break exit;
                    }
                }
            }
            if (themeFile != null) {
                super.put("themeFile", themeFile);
                String themeData = getTextResourceByName(styleFile, themeFile, "TEXTFILE");
                if (themeData != null) {
                     parse(themeData);
                }
            }            
        }

        public synchronized Object get(Object key) {
            return super.get(key);
        }

        public Object put(Object key, Object value)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public void putAll(Map t)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public Object remove(Object key)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public void clear()  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }

        private static final int EXPECT_KEY = 0;
        private static final int EXPECT_CATEGORY = 1;
        private static final int EXPECT_VALUE = 2;

        private void parse(String bytes) {
            StringTokenizer worder = new StringTokenizer(bytes, "", true);
            String category = "";            
            String key = "";
            int state = EXPECT_KEY;
            boolean skipStringSufix = false;

            try {
                while (worder.hasMoreTokens()) {
                    String word = worder.nextToken("\r\n[];=").trim();
                    if (word.length() == 0) {
                        continue;
                    }
                    switch (word.charAt(0)) {
                    case '[': // [category]
                        state = EXPECT_CATEGORY;
                        category = "";
                        break;
                    case ']':
                    case ';': // ;comment
                        skipStringSufix = true;
                        break;
                    case '=':
                        if (key.length() > 0) {
                            state = EXPECT_VALUE;
                        } else {
                            skipStringSufix = true;
                        }
                        break;
                    default: // string
                        switch (state) {
                        case EXPECT_KEY:
                            key = word.toLowerCase();
                            break;
                        case EXPECT_CATEGORY:
                            category = word.toLowerCase() + ".";
                            skipStringSufix = true;
                            break;
                        case EXPECT_VALUE:
                            String value = word;
                            super.put(category + key, value);
                            skipStringSufix = true;
                            break;
                        }                                                    
                    }
                    if (skipStringSufix) {
                        skipStringSufix = false;
                        state = EXPECT_KEY;
                        key = "";
                        worder.nextToken("\r\n");
                    }
                }
            } catch (NoSuchElementException e) {
            }
        }

        private String[] splitTextResource(String str) {
            if (str == null) {
                return new String[0];
            }
            StringTokenizer tok = new StringTokenizer(str, "\0");
            String[] array = new String[tok.countTokens()];
            for (int i = 0; tok.hasMoreTokens(); i++) {
                array[i] = tok.nextToken();
            }
            return array;
        }
    }

    private static class ImageResources implements Map {
        String styleFile;

        ImageResources(String styleFile) {
            this.styleFile = styleFile;
        }

        public synchronized Object get(Object key) {
            // Replace \ and . with _ and convert to uppercase
            StringBuffer imageName = new StringBuffer((String)key);
            for (int i = 0; i < imageName.length(); i++) {
                char ch = imageName.charAt(i);
                if (ch == '\\' || ch == '.') {
                    imageName.setCharAt(i, '_');
                }
            }

            int[] bits = getBitmapResource(styleFile, imageName.toString().toUpperCase());
            if (bits == null) {
                return null;
            }

            // The first two ints in the array are the width and the transparency value
            int width = bits[0];
            int height = (bits.length - 2) / width;
            int transparency = bits[1];
        
            GraphicsConfiguration gc = GraphicsEnvironment.getLocalGraphicsEnvironment().
                getDefaultScreenDevice().getDefaultConfiguration();
            BufferedImage image = (BufferedImage)gc.
                createCompatibleImage(width, height, transparency);

            image.setRGB(0, 0, width, height, bits, 2, width);
            return image;
        }

        public void clear()  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public boolean containsKey(Object key)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public boolean containsValue(Object value)  {
            throw new UnsupportedOperationException("Only get(Object key) is supported");
        }
        public Set entrySet()  {
            throw new UnsupportedOperationException("Only get(Object key) is supported");
        }
        public boolean isEmpty()  {
            throw new UnsupportedOperationException("Only get(Object key) is supported");
        }
        public Set keySet()  {
            throw new UnsupportedOperationException("Only get(Object key) is supported");
        }
        public Object put(Object key, Object value)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public void putAll(Map t)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public Object remove(Object key)  {
            throw new UnsupportedOperationException("ReadOnly Map");
        }
        public int size()  {
            throw new UnsupportedOperationException("Only get(Object key) is supported");
        }
        public Collection values()  {
            throw new UnsupportedOperationException("Only get(Object key) is supported");
        }
    }
}