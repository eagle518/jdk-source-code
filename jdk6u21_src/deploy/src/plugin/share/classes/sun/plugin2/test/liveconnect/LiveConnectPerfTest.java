import java.applet.Applet;
import netscape.javascript.*;

public class LiveConnectPerfTest extends Applet {
    private static final int NUM_CALLS = 20000;

    private JSObject win;

    public void start() {
        win = JSObject.getWindow(this);

        // Test Java -> JavaScript downcalls
        long startTime = System.currentTimeMillis();
        for (int i = 0; i < NUM_CALLS; i++) {
            win.call("downcall", null);
        }
        long endTime = System.currentTimeMillis();
        printResult("Java-to-JavaScript downcalls", (endTime - startTime));

        // Test JavaScript -> Java upcalls
        startTime = System.currentTimeMillis();
        win.call("doUpcalls", new Object[] { new Integer(NUM_CALLS) });
        endTime = System.currentTimeMillis();
        printResult("JavaScript-to-Java upcalls", (endTime - startTime));
        
        // Test round-trip calls
        startTime = System.currentTimeMillis();
        for (int i = 0; i < NUM_CALLS; i++) {
            win.call("roundtrip", null);
        }
        endTime = System.currentTimeMillis();
        printResult("Java-to-JavaScript round trip calls", (endTime - startTime));
        
    }

    public void upcall() {
    }

    private void printResult(String kind, long duration) {
        String res =
            "Time for " + NUM_CALLS +
            " " + kind +
            " = " +
            (float) (duration / 1000.0) + " seconds<BR>";

        win.call("printResult",
                 new Object[] { res });
    }
}
