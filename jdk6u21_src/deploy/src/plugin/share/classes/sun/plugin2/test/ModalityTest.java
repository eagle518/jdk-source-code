import java.applet.*;
import java.awt.event.*;
import javax.swing.*;

public class ModalityTest extends Applet {
    public void init() {
        JButton button = new JButton("Show Dialog from Java");
        button.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    showDialog();
                }
            });
        add(button);
    }

    public void showDialog() {
        JOptionPane.showMessageDialog(null, "Now try to switch to the\nbrowser window; you shouldn't\nbe able to");
    }
}
