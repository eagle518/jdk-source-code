import java.awt.Color;
import java.awt.event.*;
import javax.swing.*;

public class SwingDragTest extends JApplet {
    public void init() {
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    JLabel label = new JLabel("Drag me", SwingConstants.CENTER);
                    label.setOpaque(true);
                    label.setBackground(Color.GRAY);
                    // Need a MouseListener (an empty one is OK) to
                    // provoke a certain class of bug in this fix
                    label.addMouseListener(new MouseAdapter() {});
                    add(label);
                }
            });
    }
}
