package sun.awt;

import java.awt.color.ColorSpace;
import java.awt.image.*;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.regex.Pattern;


public abstract class UNIXToolkit extends SunToolkit
{
    /** 
     * List of icons loaded by default. This array gets reallocated after
     * the default icons are loaded
     */ 
    private String[] gtkIcons = {
        "gtk-cancel.4", "gtk-dialog-error.6", "gtk-dialog-info.6",
        "gtk-dialog-question.6", "gtk-dialog-warning.6", "gtk-no.4",
        "gtk-ok.4", "gtk-yes.4", 
    };

    /**
     * GTK icons load status. <code>NOT_LOADED</code> indicates that no attempt
     * to load icons were made yet. <code>FAILURE</code> indicates that the
     * first attempt to load icons failed (probably because GTK is not
     * present), and we should not make further attempts. <code>SUCCESS</code>
     * indicates that icons were successfully loaded.
     */
    private enum GTKLoadStatus { NOT_LOADED, FAILURE, SUCCESS };
    private GTKLoadStatus loadedGTKIcons = GTKLoadStatus.NOT_LOADED;

    private static String GTK_HELPER_NAME = "gtkhelper";
    private static String gtkHelperPath = null;
    private static final int[] gtkBandOffsets = { 0, 1, 2, 3 }; 
    private static ColorModel colorModel = null;

    /** Used to check icon names for validity */
    private static Pattern iconNameRegex =
            Pattern.compile("[a-z\\-]+\\.[0-9]+\\.(ltr|rtl)");
    
    
    /**
     * Overridden to handle GTK icon loading
     */
    protected Object lazilyLoadDesktopProperty(String name) {
        if (name.startsWith("gtk.icon.")) {
            return lazilyLoadGTKIcon(name);
        }
	return super.lazilyLoadDesktopProperty(name);
    }

    /**
     * Attempt to load a GTK stock icon. When called first time, before loading
     * the icon requested, load icons from <code>gtkIcons</code> list and set
     * <code>loadedGTKIcons</code>. Subsequent calls check
     * <code>loadedGTKIcons</code> and return <code>null</code> if it is 
     * <code>FAILURE</code>.
     *
     * @param longname a desktop property name. This contains icon name, size
     *        and orientation, e.g. <code>"gtk.icon.gtk-add.4.rtl"</code>
     * @return an <code>Image</code> for the icon, or <code>null</code> if the
     *         icon could not be loaded
     */
    protected Object lazilyLoadGTKIcon(String longname) {
        if (loadedGTKIcons == GTKLoadStatus.FAILURE) {
            // We couldn't load the icons
            return null;
        }

        String shortname = longname.substring(9);
        if (loadedGTKIcons == GTKLoadStatus.NOT_LOADED) {
            // This is the first attempt to load. We load all the default
            // icons plus the requested one.
            
            // build path to gtkhelper
            String path = (String)AccessController.doPrivileged(
                    new PrivilegedAction() {
                        public Object run() {
                            String arch = System.getProperty("os.arch");
                            if (arch != null && arch.equals("x86")) {
                                arch = "i386";
                            }
                            return System.getProperty("java.home") +
                                    "/lib/" + arch;
                        }
                    });
            gtkHelperPath = path + "/" + GTK_HELPER_NAME;
            
            // load default icons, using requested orientation as default
            String orn = shortname.endsWith("rtl") ? ".rtl" : ".ltr";
            for (int i=0; i<gtkIcons.length; i++) {
                gtkIcons[i] += orn;
            }
            int count = loadGTKIcons(gtkIcons);
            boolean loaded = count > 0;
            
            if (loaded) {
                loadedGTKIcons = GTKLoadStatus.SUCCESS;
                gtkIcons = new String[1];
                gtkIcons[0] = shortname;
                loadGTKIcons(gtkIcons);
            } else {
                loadedGTKIcons = GTKLoadStatus.FAILURE; 
                gtkIcons = null;
                return null;
            }
        } else {
            // loadedGTKIcons == SUCCESS. We've successfully loaded default
            // icons and are now loading a non-default one. 
            gtkIcons[0] = shortname;
            loadGTKIcons(gtkIcons);
        }
        return desktopProperties.get(longname);
    }
    

    /**
     * Load GTK stock icons.
     *
     * @param shortnames an array of icon names. A name contains icon name,
     *        size, and orientation, e.g. <code>"gtk-add.4.rtl"</code>
     * @return number of icons loaded, may be less than length of the <code>
     *         names</code> array
     */
    private int loadGTKIcons(String[] shortnames) {
        if (!checkGTKIconNames(shortnames)) {
            return 0;
        }
        
        byte[] data = readGTKIconData(gtkHelperPath, shortnames);
        if (data == null || data.length == 0) {
            return 0;
        }
        
        int n = 0;
        int count = 0;
        while (n < data.length) {
            byte len = data[n++];
            StringBuilder buf = new StringBuilder(len);
            for (int i=0; i<len; i++) {
                buf.append((char)data[n++]);
            }
            String id = buf.toString();
            byte size = data[n++];
            int w = 0xff & data[n++];
            int h = 0xff & data[n++];
            int rowStride = 0xff & data[n++];
            int pixelcount = h * rowStride;
            
// This natural way doesn't work: see 4954405
//            DataBuffer databuf = new DataBufferByte(data, pixelcount, n);
            byte[] datac = new byte[pixelcount];
            System.arraycopy(data, n, datac, 0, pixelcount);
            DataBuffer databuf = new DataBufferByte(datac, pixelcount);
            WritableRaster raster = Raster.createInterleavedRaster(databuf,
                    w, h, rowStride, 4, gtkBandOffsets, null);

            if (colorModel == null) {
                colorModel = new ComponentColorModel(
                        ColorSpace.getInstance(ColorSpace.CS_sRGB), true, false,
                        ColorModel.TRANSLUCENT, DataBuffer.TYPE_BYTE);
            }
            BufferedImage img = new BufferedImage(colorModel, raster, false, null);

            String propName = "gtk.icon." + id + "." + size; 
            setDesktopProperty(propName + ".ltr", img);
            setDesktopProperty(propName + ".rtl", img);

            n += pixelcount;
            count++;
        }
        return count;
    }
    
    /**
     * Check an icon name for validity. A valid name should have the form
     * <code><i>"name.size.orientation"</i></code>, where:
     * <ul>
     * <li><code>name</code> is GTK stock ID, e.g. <code>gtk-add</code>;
     * <li><code>size</code> is a number, currently GTK supports sizes 1
     *                       through 6;
     * <li><code>orientation</code> is either
     *                              <code>ltr</code> or <code>rtl</code>;
     * </ul>
     * 
     * @param shortnames array of icon names
     * @return <code>true</code> if all names in the array were valid;
     *         <code>false</code> otherwise
     */
    private boolean checkGTKIconNames(String[] shortnames) {
        for (int i=0; i<shortnames.length; i++) {
            if (!iconNameRegex.matcher(shortnames[i]).matches()) {
                return false;
            }
        }
        return true;
    }
    
    private native byte[] readGTKIconData(String helperPath, String[] names);

    public native void sync();
}
