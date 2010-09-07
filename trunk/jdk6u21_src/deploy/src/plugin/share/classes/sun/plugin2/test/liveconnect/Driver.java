import java.applet.Applet;
import java.util.*;
import javax.swing.*;

public class Driver extends Applet {
    private List/*<String>*/ failures = new ArrayList();
    private String header;

    public void setHeader(String header) {
        this.header = header;
    }

    public void pass(String message) {
        System.out.println("Passed: " + header + ": " + message);
    }

    public void fail(String message) {
        System.out.println("FAILED: " + header + ": " + message);
        failures.add(message);
    }

    public void exception(String message) {
        failures.add("EXCEPTION: " + header + "\n" + message);
    }

    // Automation hooks
    // Would get rid of dialog in fully automated mode
    public void checkResults() {
        String message = null;
        int type = 0;

        if (failures.isEmpty()) {
            message = "Test passed.";
            type = JOptionPane.INFORMATION_MESSAGE;
            System.out.println(message);
        } else {
            message = "TEST FAILED";
            type = JOptionPane.ERROR_MESSAGE;
            System.out.println(message);
            for (Iterator iter = failures.iterator(); iter.hasNext(); ) {
                System.out.println((String) iter.next());
            }
        }

        final String title = "6611904 Regression Test Result";
        final String fmessage = message;
        final int ftype = type;
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    JOptionPane.showMessageDialog(null, fmessage, title, ftype);
                }
            });
    }
}
