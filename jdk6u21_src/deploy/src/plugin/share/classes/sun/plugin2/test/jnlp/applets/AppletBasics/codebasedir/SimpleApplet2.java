/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.lang.reflect.*;
import java.util.*;
import java.net.*;

public class SimpleApplet2 extends SimpleAppletBase 
{
    public boolean test ()
    {
        int w = getWidth();
        int h = getHeight();
        String name = getParameter("NAME"); // the std in applet name
        String p1v = getParameter("param1");
        String p2v = getParameter("param2");
        String pjnlp = getParameter("jnlp_href");
        URL cb = getCodeBase();

        System.out.println("SimpleApplet2 testing .. ");
        boolean ok=true;

        if(h!=120) {
            ok=false;
            System.out.println("height failed");
        }

        if(name.equals("SimpleApplet2Num1")) {
            if(!p1v.equals("param1_value1")) {
                ok=false;
                System.out.println("param1_value1 failed");
            }
            if(!p2v.equals("param2_value1")) {
                ok=false;
                System.out.println("param2_value1 failed");
            }
        } else if (name.equals("SimpleApplet2Num2")) {
            if(!p1v.equals("param1_value2")) {
                ok=false;
                System.out.println("param1_value2 failed");
            }
            if(!p2v.equals("param2_value2")) {
                ok=false;
                System.out.println("param2_value2 failed");
            }
        } else {
            System.out.println("applet name failed");
            ok=false;
        }

        AppletContext ac = getAppletContext();

        Applet a1 = ac.getApplet("SimpleApplet2Num1");
        Applet a2 = ac.getApplet("SimpleApplet2Num2");


	String a1Name = getAppletName(a1, "getAppletName");
 	String a2Name = getAppletName(a2, "getAppletName");
	
        if(null!=a1 && a1Name != null && !a1Name.equals("SimpleApplet2Num1")) {
	    ok=false;
	    System.out.println("applet1 name failed");
	}
	if(null!=a2 && a2Name != null && !a2Name.equals("SimpleApplet2Num2")) {
	    ok=false;
	    System.out.println("applet2 name failed");
        }
	
        return ok;
    }

    private String getAppletName(Applet a, String methodName) {
	try {
            Method m = a .getClass().getMethod(methodName,
					       new Class[] {});
            if (m != null) {
		return (String) m.invoke(a , new Object[] {});
            }
	} catch (Exception e) {
	    System.out.println(e);
	    e.printStackTrace();
	} catch (Error err) {
	    System.out.println(err);
	    err.printStackTrace();
	}

	return null;
    }

}

