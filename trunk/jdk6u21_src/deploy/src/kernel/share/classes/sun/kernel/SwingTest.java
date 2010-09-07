package sun.kernel;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;

/** 
 * Determines the set of "core" AWT and Swing classes.  Run
 * by SplitJRE with the -verbose JVM parameter.  
 */
public class SwingTest {
    public static void main(String[] arg) throws Exception {
        final JFrame f = new JFrame();
        f.show();
        final JDialog d = new JDialog(f);
        d.show();
        final JWindow w = new JWindow(d);
        w.setSize(600, 600);
        w.setLayout(new FlowLayout());
        final JTextField textField = new JTextField();
        textField.setPreferredSize(new Dimension(200, 50));
        JMenuBar mb = new JMenuBar();
        JMenu file = new JMenu("File");
        file.setMnemonic('F');
        Action action = new AbstractAction() {
            public void actionPerformed(ActionEvent e) {
            }
        };
        JMenuItem test = new JMenuItem(action);
        file.add(test);
        mb.add(file);
        f.setJMenuBar(mb);
        w.add(new JApplet());
        w.add(textField);
        w.add(new JTextArea());
        w.add(new JButton());
        w.add(new JCheckBox());
        w.add(new JRadioButton());
        w.add(new JProgressBar());
        w.add(new JSlider());
        w.add(new JProgressBar());
        w.add(new JScrollPane());
        final Class animClass = Class.forName("sun.plugin.util.AnimationPanel");
        final Component animPanel = (Component) animClass.newInstance();
        animPanel.setSize(300, 300);
        w.add(animPanel);
        Class viewerClass = Class.forName("sun.plugin.AppletViewer");
        Component viewer = (Component) viewerClass.newInstance();
        viewer.setSize(30, 30);
        w.add(viewer);
        w.setVisible(true);
        SwingUtilities.invokeAndWait(new Runnable() {
            public void run() {
                try {
                    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
                    SwingUtilities.updateComponentTreeUI(w);
                    Method startAnimation = animClass.getMethod("startAnimation", new Class[0]);
                    startAnimation.invoke(animPanel, new Object[0]);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    System.exit(199);
                }
            }
        });
        Thread.sleep(5000);
        Robot r = new Robot();
        Point coords = new Point(5, 5);
        SwingUtilities.convertPointToScreen(coords, textField);
        r.setAutoDelay(500);
        r.mouseMove(coords.x, coords.y);
        w.toFront();
        r.mousePress(MouseEvent.BUTTON1_MASK);
        r.mouseRelease(MouseEvent.BUTTON1_MASK);
        r.keyPress(KeyEvent.VK_H);
        r.keyRelease(KeyEvent.VK_H);
        r.keyPress(KeyEvent.VK_I);
        r.keyRelease(KeyEvent.VK_I);
        w.dispose();
        d.dispose();
        f.dispose();
        try {
            Method stopAnimation = animClass.getMethod("stopAnimation", new Class[0]);
            stopAnimation.invoke(animPanel, new Object[0]);
        }
        catch (Exception e) {
            e.printStackTrace();
            System.exit(200);
        }
       // do not exit manually, so that AWT auto shutdown kicks in and loads its classes
    }
}
