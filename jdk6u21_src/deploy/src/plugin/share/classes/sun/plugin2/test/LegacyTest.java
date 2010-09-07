import java.applet.Applet;

import javax.swing.*;

/** Regression test for 6618971 */

public class LegacyTest extends Applet {
    private int numInits;
    private int numStarts;
    private int numStops;
    private int numDestroys;

    // No-op method to get the parent page to wait for us to load
    public void waitForLoad() {
    }

    public void init() {
        ++numInits;
    }

    public void start() {
        ++numStarts;
    }

    public void stop() {
        ++numStops;
    }

    public void destroy() {
        ++numDestroys;
    }

    private String state = "(passed)";

    public boolean check(int numInits, int numStarts, int numStops, int numReloads) {
        if (this.numInits == numInits &&
            this.numStarts == numStarts &&
            this.numStops == numStops &&
            this.numDestroys == numDestroys) {
            return true;
        } else {
            state = ("   numInits = "    + this.numInits    + " (expected " + numInits + ")\n" +
                     "   numStarts = "   + this.numStarts   + " (expected " + numStarts + ")\n" +
                     "   numStops = "    + this.numStops    + " (expected " + numStops + ")\n" +
                     "   numDestroys = " + this.numDestroys + " (expected " + numDestroys + ")\n");
            return false;
        }
    }

    public String state() {
        return state;
    }

    private static final String title = "6618971 Regression Test Result";

    // Automation hooks
    // Would remove dialog in case of full automation
    public void pass() {
        final String message = "Test passed.";
        System.out.println(message);
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    JOptionPane.showMessageDialog(null, message, title, JOptionPane.INFORMATION_MESSAGE);
                }
            });
    }

    public void fail(String otherState) {
        final String message = "TEST FAILED" +
            "\nApplet 1 state:\n" + state() +
            "\nApplet 2 state:\n" + otherState;
        System.out.println(message);
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    JOptionPane.showMessageDialog(null, message, title, JOptionPane.ERROR_MESSAGE);
                }
            });
    }
}
