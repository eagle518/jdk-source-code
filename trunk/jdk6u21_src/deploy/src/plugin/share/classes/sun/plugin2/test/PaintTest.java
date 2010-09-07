import java.awt.Graphics;
import javax.swing.*;

// Regression test for 6611903

public class PaintTest extends JApplet {
    private boolean testPassed;

    public void start() {
         SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    getContentPane().add(new TestButton("Button text should be visible"));
                }
            });
         new Thread(new Runnable() {
                 public void run() {
                     try {
                         Thread.sleep(2000);
                     } catch (InterruptedException e) {
                     }
                     String title = "6611903 Regression Test Result";
                     if (testPassed) {
                         // Automation hooks -- can easily get rid of these dialogs
                         System.out.println("Test passed.");
                         JOptionPane.showMessageDialog(null, "Test passed.", title, JOptionPane.INFORMATION_MESSAGE);
                     } else {
                         System.out.println("TEST FAILED");
                         JOptionPane.showMessageDialog(null, "TEST FAILED", title, JOptionPane.ERROR_MESSAGE);
                     }
                 }
             }).start();
    }

    class TestButton extends JButton {
        public TestButton(String text) {
            super(text);
        }

        protected void paintComponent(Graphics g) {
            testPassed = true;
            super.paintComponent(g);
        }
    }
}
