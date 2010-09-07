import java.awt.*;
import java.awt.event.*;
import java.awt.font.*;
import java.net.*;
import javax.swing.*;
import netscape.javascript.JSObject;

// Basic test of drag-and-drop of applets to the desktop as well as
// disconnected applet context functionality

public class DragTest2 extends JApplet {
    private URL documentURL;
    private ContentPanel panel;
    private ActionListener closeListener;

    private static final int  DRAG_BAR_HEIGHT = 12;
    private static final Font DRAG_BAR_FONT = new Font("SansSerif", Font.PLAIN, 10);
    private static final String TEXT = "Click And Drag Here";

    class ContentPanel extends JPanel {
        private boolean gotTextWidth;
        private int textWidth;

        public ContentPanel() {
            super();
            setBackground(Color.GRAY);
            addMouseListener(new MouseAdapter() {
                    public void mouseClicked(MouseEvent e) {
                        if (closeListener != null) {
                            if (e.getButton() == MouseEvent.BUTTON1 &&
                                e.getY() <= DRAG_BAR_HEIGHT &&
                                e.getX() >= (getWidth() - DRAG_BAR_HEIGHT)) {
                                // Shut ourselves down
                                closeListener.actionPerformed(null);
                            }
                        }
                    }
                });
        }

        protected void paintComponent(Graphics graphics) {
            super.paintComponent(graphics);
            Graphics2D g = (Graphics2D) graphics;
            int width = getWidth();
            int height = getHeight();
            g.setColor(Color.RED);
            g.fillRect(0, 0, width, DRAG_BAR_HEIGHT);
            g.setColor(Color.WHITE);
            g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
            g.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
            g.setFont(DRAG_BAR_FONT);
            if (!gotTextWidth) {
                FontMetrics fm = g.getFontMetrics();
                textWidth = fm.charsWidth(TEXT.toCharArray(), 0, TEXT.length());
                gotTextWidth = true;
            }
            g.drawString(TEXT, (width - textWidth) / 2, DRAG_BAR_HEIGHT - 1);

            // Draw the close region
            int xLeft   = getWidth() - DRAG_BAR_HEIGHT + 1;
            int xRight  = getWidth() - 2;
            int xTop    = 1;
            int xBottom = DRAG_BAR_HEIGHT - 2;

            g.drawLine(xLeft, xTop, xRight, xBottom);
            g.drawLine(xLeft, xBottom, xRight, xTop);
        }

        public boolean isAppletDragStart(MouseEvent e) {
            return ((e.getSource() == this ||
                     ((e.getSource() instanceof Container) &&
                      (((Container) e.getSource()).isAncestorOf(this)))) &&
                    e.getY() <= DRAG_BAR_HEIGHT &&
                    e.getX() < (getWidth() - DRAG_BAR_HEIGHT) &&
                    !e.isAltDown() &&
                    !e.isAltGraphDown() &&
                    !e.isControlDown() &&
                    !e.isMetaDown() &&
                    !e.isShiftDown());
        }
    }

    public void init() {
        JSObject win = JSObject.getWindow(this);
        try {
            documentURL =
                new URL((String) win.eval("document.URL"));
            JButton button = new JButton("Reopen Web Page");
            button.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        getAppletContext().showDocument(documentURL);
                    }
                });
            panel = new ContentPanel();
            panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
            panel.add(Box.createVerticalGlue());
            Box box = Box.createHorizontalBox();
            box.add(Box.createHorizontalGlue());
            box.add(button);
            box.add(Box.createHorizontalGlue());
            panel.add(box);
            panel.add(Box.createVerticalGlue());
            add(panel);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    //----------------------------------------------------------------------
    // Methods for interacting with the drag-and-drop support
    //

    public boolean isAppletDragStart(MouseEvent e) {
        if (panel == null) {
            return false;
        }
            
        return panel.isAppletDragStart(e);
    }

    public void setAppletCloseListener(ActionListener l) {
        closeListener = l;
    }

    public void appletRestored() {
        closeListener = null;
    }
}
