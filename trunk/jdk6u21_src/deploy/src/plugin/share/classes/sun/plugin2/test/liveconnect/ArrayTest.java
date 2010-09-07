import java.applet.Applet;
import netscape.javascript.*;
import javax.swing.*;

/** Regression test for 6611904 */

public class ArrayTest extends Applet {
    public void start() {
        try {
            JSObject window = JSObject.getWindow(this);
            JSObject array  = (JSObject) window.eval("getTestArray()");
            int length = ((Number) array.getMember("length")).intValue();
            for (int i = 0; i < length; i++) {
                Object element = null;
                try {
                    element = array.getSlot(i);
                } catch (JSException e) {
                    System.out.println("For debugging:");
                    e.printStackTrace();
                }
                System.out.print("Element " + i + ": " + element);
                if (element != null) {
                    System.out.print(" (a " + element.getClass().getName() + ")");
                }
                System.out.println();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void passIntArray(int[] array) {
        // Note that we know implicitly what the data is coming in from JavaScript
        if (array.length != 3) {
            fail();
        }
        if (array[0] != 5) {
            fail();
        }
        if (array[1] != 0) {
            fail();
        }
        if (array[2] != 7) {
            fail();
        }
        pass("passIntArray");
    }

    public void passStringArray(String[] array) {
        // Note that we know implicitly what the data is coming in from JavaScript
        if (array.length != 2) {
            fail();
        }
        if (!"Hello".equals(array[0])) {
            fail();
        }
        if (!"world".equals(array[1])) {
            fail();
        }
        pass("passStringArray");
    }

    public void expect(int numPasses) {
        // Automation hook
        // Would get rid of dialog in fully automated mode
        final String title = "6611904 Regression Test Result";
        String message = null;
        int type = 0;
        if (this.numPasses != numPasses) {
            message = "TEST FAILED";
            type = JOptionPane.ERROR_MESSAGE;
        } else {
            message = "Test passed.";
            type = JOptionPane.INFORMATION_MESSAGE;
        }        
        final String fmessage = message;
        final int ftype = type;
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    JOptionPane.showMessageDialog(null, fmessage, title, ftype);
                }
            });
        if (this.numPasses != numPasses) {
            fail();
        }
    }

    private int numPasses;

    private void fail() {
        throw new RuntimeException("Test failed");
    }

    private void pass(String testName) {
        System.out.println("Test \"" + testName + "\" passed.");
        ++numPasses;
    }
}
