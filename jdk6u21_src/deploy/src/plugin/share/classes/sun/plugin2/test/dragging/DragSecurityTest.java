import java.applet.*;
import java.awt.*;
import java.awt.event.*;

public class DragSecurityTest extends Applet {
    public void start() {
        setBackground(Color.RED);
        new EventThread().start();
    }

    // Do this work on a background thread to defeat event coalescing
    class EventThread extends Thread {
        public void run() {
            Applet applet = DragSecurityTest.this;
            try {
                sleep(200);
            } catch (InterruptedException e) {}
            // Pop the applet out
            postEvent(new MouseEvent(applet,
                                     MouseEvent.MOUSE_PRESSED,
                                     System.currentTimeMillis(),
                                     InputEvent.BUTTON1_DOWN_MASK | InputEvent.ALT_DOWN_MASK,
                                     10, 10,
                                     0,
                                     false, MouseEvent.BUTTON1));

            try {
                sleep(200);
            } catch (InterruptedException e) {}

            // Note that there's no point in posting fake drag events
            // because the mechanism in the DragListener to query the
            // mouse pointer's absolute position will defeat them --
            // but this doesn't matter from the security perspective
            // because at this point the applet has an undecorated
            // Frame with no warning banner and could programmatically
            // move it around the screen

            // Release it
            postEvent(new MouseEvent(applet,
                                     MouseEvent.MOUSE_RELEASED,
                                     System.currentTimeMillis(),
                                     InputEvent.ALT_DOWN_MASK,
                                     30, 30,
                                     0,
                                     false, MouseEvent.BUTTON1));
        }

        private void postEvent(final MouseEvent ev) {
            EventQueue.invokeLater(new Runnable() {
                    public void run() {
                        EventQueue queue = Toolkit.getDefaultToolkit().getSystemEventQueue();
                        queue.postEvent(ev);
                    }
                });
        }
    }
}
