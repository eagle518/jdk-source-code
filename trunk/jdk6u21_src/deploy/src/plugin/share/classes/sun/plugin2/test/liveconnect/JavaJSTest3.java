import java.applet.*;
import netscape.javascript.*;

public class JavaJSTest3 extends Applet {
    public void start() {
        try {
            JSObject window = JSObject.getWindow(this);
            System.out.println("Calling JavaScript getString();");
            String res = (String) window.eval("getString();");
            System.out.println("Got string from JavaScript: \"" + res + "\"");
            if (!res.equals("Hello, world!")) {
                throw new RuntimeException("TEST FAILED");
            }
            Number num = (Number) window.eval("getNumber()");
            System.out.println("Got number from JavaScript: " + num);
            if (num.intValue() != 5) {
                throw new RuntimeException("TEST FAILED");
            }
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
