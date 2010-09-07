import java.applet.*;
import netscape.javascript.*;

public class JavaJSTest2 extends Applet {
    public void start() {
        try {
            JSObject window = JSObject.getWindow(this);
            System.out.println("Calling JavaScript testArray;");
            JSObject res = (JSObject) window.eval("testArray;");
            System.out.println("Finished calling JavaScript testArray;");
            System.out.println("testArray[0] = " + res.getSlot(0));
            System.out.println("testArray[1] = " + res.getSlot(1));
            System.out.println("Trying to define testArray[2]");
            res.setSlot(2, "baz");
            System.out.println("testArray[2] = " + res.getSlot(2));
            System.out.println("Test passed.");
        } catch (JSException e) {
            e.printStackTrace();
            System.out.println("TEST FAILED");
        } catch (Exception e2) {
            e2.printStackTrace();
            System.out.println("TEST FAILED");
        }
    }
}
