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

public class JRERelaunch1 extends SimpleAppletBase 
{
    public JRERelaunch1()
    {
        super();
    }

    public JRERelaunch1(String name)
    {
        super(name);
    }

    public boolean test ()
    {
        boolean ok=true;

        if(!getIsApplet()) {
            return ok;
        }

        String check_vendor  = getParameter("check.java.vendor");
        String check_version_product  = getParameter("check.java.version_product");
        String check_version_platform = getParameter("check.java.version_platform");
        String check_jvm_args = getParameter("check.java.jvm_args");
        long  check_maxHeapSize = -1; // don't check
        int check_isRelaunch = -1; // don't check
        try {
            String tmp = getParameter("check.java.maxheapsize");
            if(tmp!=null && tmp.length()>0) {
                check_maxHeapSize = Long.parseLong(tmp);
                System.out.println("\tparam check.java.maxheapsize: "+tmp+","+check_maxHeapSize);
            }
            tmp = getParameter("check.java.isRelaunch");
            if(tmp!=null && tmp.length()>0) {
                check_isRelaunch = (Boolean.valueOf(tmp).booleanValue()==true)?1:0;
                System.out.println("\tparam check.java.isRelaunch: "+tmp+","+check_isRelaunch);
            }
        } catch (Exception e) {
            System.out.println("\tfetching parameter failed (custom)"); 
            ok = false;
        }

        if ( check_jvm_args!=null && jvm_args.indexOf(check_jvm_args)<0 ) {
            System.out.println("\tjvm_args check failed !(current contains desired): <"+jvm_args+"> , <"+check_jvm_args+">");
            ok = false;
        } else {
            System.out.println("\tjvm_args check OK (current contains desired): <"+jvm_args+"> >= <"+check_jvm_args+">");
        }

        if ( check_vendor!=null && !vendor.equals(check_vendor) ) {
            System.out.println("\tvendor check failed (current!=desired): "+vendor+" != "+check_vendor);
            ok = false;
        } else {
            System.out.println("\tvendor check OK (current, desired): "+vendor+" , "+check_vendor);
        }

        if ( check_version_product!=null && !version_product.startsWith(check_version_product) ) {
            System.out.println("\tversion_product check failed (current.startsWith(desired)): "+version_product+" >= "+check_version_product);
            ok = false;
        } else {
            System.out.println("\tversion_product check OK (current.startsWith(desired)): "+version_product+" , "+check_version_product);
        }

        if ( check_version_platform!=null && !version_platform.equals(check_version_platform) ) {
            System.out.println("\tversion_platform check failed (current!=desired): "+version_platform+" != "+check_version_platform);
            ok = false;
        } else {
            System.out.println("\tversion_platform check OK (current,desired): "+version_platform+" , "+check_version_platform);
        }

        if ( check_maxHeapSize>0 && check_maxHeapSize != maxHeapSize ) {
            System.out.println("\tmaxHeapSize check failed (current!=desired): "+maxHeapSize+" != "+check_maxHeapSize);
            ok = false;
        } else {
            System.out.println("\tmaxHeapSize check OK (current,desired): "+maxHeapSize+" , "+check_maxHeapSize);
        }

        if ( check_isRelaunch>0 && check_isRelaunch != isRelaunch ) {
            System.out.println("\tisRelaunch check failed (current!=desired): "+isRelaunch+" != "+check_isRelaunch);
            ok = false;
        } else {
            System.out.println("\tisRelaunch check OK (current,desired): "+isRelaunch+" , "+check_isRelaunch);
        }

        return ok;
    }

    public static void main( String args[] ) {
        JRERelaunch1 applet = new JRERelaunch1("JRERelaunch1");

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

