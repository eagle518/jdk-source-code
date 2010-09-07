import java.applet.Applet;

/** Regression test for 6618920 <br>
    Plugin2 : Add classloader caching support to new Plug-in */

public class ClassLoaderCacheTest extends Applet {
    private static Object  lock = new Object();
    private static volatile boolean latch;

    private boolean server;

    public void init() {
        server = (getParameter("server") != null);
    }

    public void start() {
        if (server) {
            synchronized(lock) {
                latch = true;
                lock.notifyAll();
                try {
                    lock.wait(1000);
                } catch (InterruptedException e) {
                }
                if (latch) {
                    System.out.println("Server half -- TEST FAILED -- client did not clear latch");
                } else {
                    System.out.println("Test passed.");
                }
            }
        } else {
            synchronized(lock) {
                if (!latch) {
                    try {
                        lock.wait(1000);
                    } catch (InterruptedException e) {
                    }
                }
                if (!latch) {
                    System.out.println("Client half -- TEST FAILED -- server did not set latch");
                } else {
                    latch = false;
                    lock.notifyAll();
                }
            }
        }
    }
}
