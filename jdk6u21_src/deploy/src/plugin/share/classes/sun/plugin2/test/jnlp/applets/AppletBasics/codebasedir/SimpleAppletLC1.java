/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;

public class SimpleAppletLC1 extends SimpleAppletBase 
{
    private String postInitValue = null;
    private boolean ok = false;

    public String getPostInitValue()
    { return postInitValue; }

    public void setTestResult(boolean ok)
    { 
      this.ok = ok; 
      startOk=true;
      validateTestResult();
    }

    public void init()
    {
        postInitValue = "ERROR";

        super.init();
        initOk=false;

        try {
            Thread.sleep(5000); // sleep 5s
        } catch (InterruptedException ie) {}

        postInitValue = "OK";
        initOk=true;
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

