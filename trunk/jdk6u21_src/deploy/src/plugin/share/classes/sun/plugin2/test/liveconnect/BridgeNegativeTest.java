import com.sun.java.browser.plugin2.liveconnect.v1.*;

public class BridgeNegativeTest extends java.applet.Applet {
    private boolean passed = false;

    public void init() {
        try {
            BridgeFactory.getBridge(this);
        } catch (SecurityException e) {
            passed = true;
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public boolean passed() {
        return passed;
    }
}
