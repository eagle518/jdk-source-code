import java.applet.Applet;
import javax.swing.*;

public class StringTest extends Applet {
    private boolean testPassed;

    public void javaMethod(String s1, String s2) {
        System.out.println("javaMethod(\"" + s1 + "\", \"" + s2 + "\")");
        testPassed = true;
    }

    public void beginTest() {
        testPassed = false;
    }

    public void endTest() {
        final String title = "6624949 Regression Test Result";
        String message = null;
        int type = 0;
        // Automation hooks
        if (testPassed) {
            message = "Test passed.";
            type = JOptionPane.INFORMATION_MESSAGE;
        } else {
            message = "TEST FAILED";
            type = JOptionPane.ERROR_MESSAGE;
        }
        System.out.println(message);
        final String fmessage = message;
        final int ftype = type;
        // Can easily remove this for full automation
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    JOptionPane.showMessageDialog(null, fmessage, title, ftype);
                }
            });
    }
}
