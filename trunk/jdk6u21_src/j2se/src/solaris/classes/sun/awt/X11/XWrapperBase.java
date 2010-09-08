package sun.awt.X11;

// This class serves as the base class for all the wrappers. 
import java.util.logging.*;

abstract class XWrapperBase {
    static final Logger log = Logger.getLogger("sun.awt.X11.wrappers");

    public String toString() {
        String ret = "";
        
        ret += getName() + " = " + getFieldsAsString();
        
        return ret;
    }

    String getFieldsAsString() {
        return "";
    }

    String getName() {
        return "XWrapperBase";   
    }
    public void zero() {
        log.finest("Cleaning memory");
        if (getPData() != 0) {
            XlibWrapper.unsafe.setMemory(getPData(), (long)getDataSize(), (byte)0);
        }
    }
    public abstract int getDataSize();
    String getWindow(long window) { 
        XBaseWindow w = XToolkit.windowToXWindow(window);
        if (w == null) {
            return Long.toHexString(window); 
        } else {
            return w.toString();
        }
    }
    public abstract long getPData();
    public XEvent clone() {
        long copy = XlibWrapper.unsafe.allocateMemory(getDataSize());
        XlibWrapper.unsafe.copyMemory(getPData(), copy, getDataSize());
        return new XEvent(copy);
    }
}
