import java.applet.*;
import java.net.*;
import netscape.javascript.*;

public class ShowDocumentTest extends Applet {
    private boolean pass1;
    private boolean fail1;
    private boolean pass2;
    private boolean fail2;

    private boolean amMainInstance;
    private static ShowDocumentTest mainInstance;

    private static synchronized void setMainInstance(ShowDocumentTest applet) {
        mainInstance = applet;
    }

    // Support reloading in Firefox where page isn't destroyed before new one starts loading
    private static synchronized void resetMainInstance(ShowDocumentTest applet) {
        if (mainInstance == applet) {
            mainInstance = null;
        }
    }

    public void start() {
        if (getParameter("main") != null) {
            amMainInstance = true;
            setMainInstance(this);

            // Try to view a document in the other frame
            try {
                getAppletContext().showDocument(new URL(getDocumentBase(), "test.html"),
                                                "used-to-not-work");
                // If this test case is served up from a web server,
                // the browsers will block the opening of this other
                // window. If it is served up off the local disk, IE
                // will allow the popup but Firefox won't (and won't
                // provide any way to temporarily allow the popup) --
                // the only solution is to turn off the pop-up
                // blocker.
                if (getParameter("test_popups") != null) {
                    getAppletContext().showDocument(new URL(getDocumentBase(), "test2.html"),
                                                    "nonexistent-frame-name");
                } else {
                    pass2();
                }
            } catch (MalformedURLException e) {
                throw new RuntimeException(e);
            }
            // Start watchdog timer waiting for results
            new Thread(new Runnable() {
                    public void run() {
                        synchronized(ShowDocumentTest.this) {
                            if (!passed()) {
                                try {
                                    ShowDocumentTest.this.wait(10000);
                                } catch (InterruptedException e) {
                                }
                            }
                        }
                        reportResult();
                    }
                }).start();
        } else {
            mainInstance.pass2();
            try {
                JSObject win = JSObject.getWindow(this);
                win.eval("window.close()");
            } catch (JSException e) {
                e.printStackTrace();
                System.out.println("TEST 2 FAILED");
                mainInstance.fail2(); // Probably too late
            }
        }
    }

    public void stop() {
        if (amMainInstance) {
            resetMainInstance(this);
        }
    }

    public synchronized void pass1() {
        pass1 = true;
        fail1 = false;
        checkResult();
    }

    public synchronized void fail1() {
        pass1 = false;
        fail1 = true;
        checkResult();
    }

    public synchronized void pass2() {
        pass2 = true;
        fail2 = false;
        checkResult();
    }

    public synchronized void fail2() {
        pass2 = false;
        fail2 = true;
        checkResult();
    }

    private void checkResult() {
        if ((pass1 || fail1) && (pass2 || fail2)) {
            System.out.println("Notifying all");
            notifyAll();
        }
    }

    private synchronized boolean passed() {
        return (pass1 && pass2);
    }

    private String description() {
        StringBuffer desc = new StringBuffer();
        if (!pass1 || !pass2) {
            desc.append("<ul>");
            if (!pass1 || fail1) {
                desc.append("<li> Attempt to show document in existing frame target containing hyphens failed");
            }
            if (!pass2 || fail2) {
                desc.append("<li> Attempt to show document in non-existent frame target containing hyphens failed");
            }
            desc.append("</ul>");
        }
        return desc.toString();
    }

    private void reportResult() {
        try {
            JSObject win = JSObject.getWindow(this);
            win.call("reportResult", new Object[] {
                    Boolean.valueOf(passed()),
                    description()
                });
        } catch (JSException e) {
            e.printStackTrace();
            System.out.println("TEST FAILED");
        }
    }
}
