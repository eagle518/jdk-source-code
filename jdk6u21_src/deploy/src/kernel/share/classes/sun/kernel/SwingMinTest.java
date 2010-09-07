package sun.kernel;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import java.util.*;

/** 
 * Determines the set of "core" AWT and Swing classes needed to run minimal
 * Swing application, applet and webstart application.  
 * Run by SplitJRE with the -verbose JVM parameter.
 */
public class SwingMinTest {
    public static void touchLAF() {
        UIManager.LookAndFeelInfo[] lafInfo = UIManager.getInstalledLookAndFeels();

        for (int counter = 0; counter < lafInfo.length; counter++) {
            try { 
                Class lnfClass = Class.forName(lafInfo[counter].getClassName());
                LookAndFeel newLAF = (LookAndFeel)(lnfClass.newInstance());
                boolean junk = newLAF.isSupportedLookAndFeel();
            } catch(Exception e) {}
        }
    }

    public static void touchJavaWSDialogs(Component owner) throws Exception {
        final Class factoryClass = Class.forName("com.sun.deploy.ui.UIFactory");
        final Class appInfoClass = Class.forName("com.sun.deploy.ui.AppInfo");
        final Class dialogClass  = Class.forName("com.sun.deploy.ui.ProgressDialog");

        final Method createProgressDialog = factoryClass.getDeclaredMethod(
            "createProgressDialog", 
            new Class[] {appInfoClass, java.awt.Component.class, 
                         String.class, String.class, boolean.class});
        final Method showProgressDialog = factoryClass.getDeclaredMethod(
            "showProgressDialog", 
            new Class[] {dialogClass}); 
        final Method cancelAction = dialogClass.getMethod(
            "cancelAction", new Class[0]);
        final Method showProgress = dialogClass.getMethod("showProgress", 
            new Class[] {int.class});


        final Object appInfo = appInfoClass.newInstance();
        final Object dialog = createProgressDialog.invoke(factoryClass, 
                  new Object[] {appInfo, owner, "Content string", 
                                "Title", Boolean.TRUE});

        System.out.println("SwingMinTest: WS dialogs created");
        SwingUtilities.invokeAndWait(new Runnable() {
            public void run() {
                try {
                    showProgress.invoke(dialog, new Object[] {new Integer(10)});
                }
                catch (Exception e) {
                    e.printStackTrace();
                    System.exit(196);
                }
            }
        });
        System.out.println("SwingMinTest: showProgress.invoke finished");
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                try {
                    System.out.println("SwingMinTest: showProgressDialog.invoke");
                    showProgressDialog.invoke(factoryClass, 
                                              new Object[] {dialog});
                    System.out.println("SwingMinTest: showProgressDialog.invoke finished");
                } catch (Exception e) {
                    e.printStackTrace();
                    System.exit(197);
                }
            }
        });
        System.out.println("SwingMinTest: Thread.sleep");
        Thread.sleep(2000);
        System.out.println("SwingMinTest: cancelAction.invoke");
        SwingUtilities.invokeAndWait(new Runnable() {
            public void run() {
                try {
                    cancelAction.invoke(dialog, new Object[0]);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    System.exit(198);
                }
            }
        });
        System.out.println("SwingMinTest: cancelAction.invoke finished");
    }
    
    
    private static void startWatchdog() {
        Thread watchdog = new Thread("Watchdog") {
            public void run() {
                try {
                    Thread.sleep(5 * 60 * 1000); // 5 minutes
                    System.out.println("Watchdog triggered: hung");
                    // if we hit this point, evidently we're hung
                    Class helper = Class.forName("com.sun.deploy.util.ConsoleHelper");
                    Method dumpAllStacks = helper.getMethod("dumpAllStacks", new Class[0]);
                    String trace = (String) dumpAllStacks.invoke(helper, new Object[0]);
                    System.out.println(trace);
                    System.out.println("Calling System.exit");
                    System.exit(0);
                    System.out.println("Hmmm. Still running.");
                }
                catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };
        watchdog.setDaemon(true);
        watchdog.start();
    }


    public static void main(String[] arg) throws Exception {
        System.out.println("SwingMinTest: startup");
        startWatchdog();
        touchLAF();

        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        System.out.println("SwingMinTest: LAF set");

        final JFrame f = new JFrame();
        //even smallest real Swing apps use scrollpane
        f.add(new JScrollPane());
        f.show();

        final JDialog d = new JDialog(f);
        
        d.show();
        final JWindow w = new JWindow(d);
        w.setSize(600, 600);
        w.setLayout(new FlowLayout());
        System.out.println("SwingMinTest: Dialogs created");

//        System.out.println("SwingMinTest: touchJavaWSDialogs");
//        touchJavaWSDialogs(w);
//        System.out.println("SwingMinTest: touchJavaWSDialogs finished");

        final Class animClass = Class.forName("sun.plugin.util.AnimationPanel");
        final Component animPanel = (Component) animClass.newInstance();
        System.out.println("SwingMinTest: animPanel created");
        animPanel.setSize(300, 300);
        w.add(animPanel);
        w.setVisible(true);
        SwingUtilities.invokeAndWait(new Runnable() 
        {
            public void run() 
            {
                try 
                {
                    System.out.println("SwingMinTest: calling updateComponentTreeUI");
                    SwingUtilities.updateComponentTreeUI(w);
                    Method startAnimation = animClass.getMethod(
                        "startAnimation", new Class[0]);
                    System.out.println("SwingMinTest: calling startAnimation");
                    startAnimation.invoke(animPanel, new Object[0]);
                    System.out.println("SwingMinTest: animation started");
                }
                catch (Exception e) 
                {
                    e.printStackTrace();
                    System.exit(199);
                }
            }
        });
        try {
            Thread.sleep(20000);
            Method stopAnimation = animClass.getMethod("stopAnimation", 
                                                       new Class[0]);
            stopAnimation.invoke(animPanel, new Object[0]);
            System.out.println("SwingMinTest: animation stopped");
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(200);
        }
        System.out.println("SwingMinTest: returned from invokeAndWait");
        
        w.dispose();
        d.dispose();
        f.dispose();
        System.out.println("SwingMinTest: dialogs disposed");
        
        // do not exit manually, so that AWT auto shutdown kicks in 
        // and loads its classes
        System.out.println("SwingMinTest: finished");
    }
}
