import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import javax.swing.*;
import javax.swing.border.*;

// Test of dragging an applet containing sub-components.
// Try dragging starting in all portions of the applet.

public class DragTest3 extends JApplet {
    public void init() {
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    Box vbox = Box.createVerticalBox();
                    Box hbox = Box.createHorizontalBox();
        
                    hbox.add(createPanel());
                    hbox.add(createPanel());
                    vbox.add(hbox);
        
                    hbox = Box.createHorizontalBox();
                    hbox.add(createPanel());
                    hbox.add(createPanel());
                    vbox.add(hbox);

                    add(vbox);
                }
            });
    }

    private JPanel createPanel() {
        JPanel panel = new JPanel();
        panel.setBackground(Color.BLACK);
        LineBorder border = new LineBorder(Color.WHITE, 1);
        panel.setBorder(border);
        Canvas c = new Canvas();
        c.setBackground(Color.RED);
        c.setSize(80, 80);
        panel.add(c);
        return panel;
    }
}
