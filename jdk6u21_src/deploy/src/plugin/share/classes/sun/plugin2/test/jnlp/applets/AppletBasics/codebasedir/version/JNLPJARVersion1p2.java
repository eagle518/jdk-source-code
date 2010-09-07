/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;

public class JNLPJARVersion extends SimpleAppletBase 
{
    public static final String version = "1.2";

    public JNLPJARVersion()
    {
        super();
    }

    public JNLPJARVersion(String name)
    {
        super(name);
    }

    public String getParameter(String name) {
        if(name.equals("NAME")) {
            return super.getParameter(name) + ", version "+version; 
        }
        return super.getParameter(name);
    }

    public boolean test ()
    {
        boolean ok=true;

        if(!getIsApplet()) {
            return ok;
        }

        String check_version = getParameter("check.version");
        if ( check_version!=null && !check_version.equals(version) ) {
            System.out.println("\tversion check failed !(current contains desired): <"+version+"> , <"+check_version+">");
            ok = false;
        } else {
            System.out.println("\tversion check OK (current contains desired): <"+version+"> >= <"+check_version+">");
        }

        return ok;
    }

    public static void main( String args[] ) {
        JNLPJARVersion applet = new JNLPJARVersion("JNLPJARVersion");

        Frame mainFrame = new Frame("Simple Applet Base Application Helper");
        mainFrame.addWindowListener( new WindowAdapter() {
                public void windowClosed(WindowEvent e) {
                    System.exit(0);
                }
                public void windowClosing(WindowEvent e) {
                    System.exit(0);
                }
            } );

        mainFrame.setLayout(new BorderLayout());
        mainFrame.setSize(500,200);
        applet.setSize(400, 120);
        mainFrame.add("Center", applet);

        mainFrame.setVisible(true);

        applet.init();
        applet.start();
    }

}

