/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;
import netscape.javascript.*;

public class SimpleAppletLC2 extends SimpleAppletBase 
{
    private boolean ok = false;

    public void setTestResult(boolean ok)
    { this.ok = ok; 
      startOk=true;
      validateTestResult();
    }

    public void init()
    {
        super.init();

        JSObject win = JSObject.getWindow(this);
        win.eval("runTest()");
    }

    public void start()
    {
		System.out.println("Applet "+getAppletName()+" start ..(isApplet: "+getIsApplet()+")");
        SimpleAppletUtil.printAppletInfo(this);

        // no test validation yet ..

        Exception e = new Exception("Applet "+getAppletName()+".start(): StackTrace:");
        e.printStackTrace();
    }

    public boolean test ()
    {
        return ok;
    }
}

