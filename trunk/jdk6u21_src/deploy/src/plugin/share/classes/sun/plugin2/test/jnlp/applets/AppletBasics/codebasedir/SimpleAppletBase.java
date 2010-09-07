/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.awt.Graphics;

public abstract class SimpleAppletBase extends Applet implements SimpleAppletTestIf1
{
    protected String version_product=null, version_platform=null, vendor=null;
    protected long   maxHeapSize=-1;
    protected String jvm_args = null;
    protected int    isRelaunch = 0;

    private   boolean isApplet;
    private String appletName = null;
    private boolean testResult = false;
    protected boolean initOk = false;
    protected boolean startOk = false;

    public SimpleAppletBase()
    {
        this.isApplet = true;
    }

    public SimpleAppletBase(String name)
    {
        this.isApplet = false;
        appletName = name;
    }

    public boolean getIsApplet () 
    { return isApplet; }

    public String getAppletName()
    { return appletName; }

    public boolean getTestResult () 
    { return testResult; }

    public String getVersionProduct()
    { return version_product; }

    public String getVersionPlatform()
    { return version_platform; }

    public String getVendor()
    { return vendor; }

    public long getMaxHeapSize()
    { return maxHeapSize; }

    public String getJVMArgs()
    { return jvm_args; }

    public boolean getIsAppletRelaunched () 
    { return isRelaunch==1; }

    public boolean getInitResult() {
        return initOk;
    }

    /**
     * Applet Lifecycle
     *
     * Called by the browser or applet viewer to inform this applet that it has been loaded into the system.
     */
    public void init()
    {
        System.out.println("init: isApplet: "+isApplet);

        initOk = true;

        if(isApplet) {
            appletName = getParameter("NAME"); // the std in applet name
        }

        try {
            vendor  = System.getProperty("java.vendor");
            version_product  = System.getProperty("java.version");
            version_platform = System.getProperty("java.specification.version");
            if(isApplet) {
                jvm_args = System.getProperty("javaplugin.vm.options");
                maxHeapSize = SimpleAppletUtil.parseMemorySpec(jvm_args);
            }
        } catch (Exception e) {
            System.out.println("\tfetching properties failed");
            e.printStackTrace();
            initOk = false;
        }

        if(isApplet) {
            try {
                String tmp;
                tmp = getParameter("__applet_relaunched");
                if ( tmp == null ) {
                    // This basically means that a relaunch didn't occur
                    System.out.println("Error: __applet_relaunched does not exist");
                    initOk = false;
                } else {
                    isRelaunch = (Boolean.valueOf(tmp).booleanValue()==true)?1:0;
                    System.out.println("isRelaunch: "+isRelaunch);
                }
            } catch (Exception e) {
                System.out.println("\tfetching parameter failed (internal)");
                e.printStackTrace();
                initOk = false;
            }
        }
		System.out.println("Applet "+appletName+" init (ok="+initOk+")");
        SimpleAppletUtil.printAppletInfo(this);

        Exception e = new Exception("Applet "+appletName+".init(): StackTrace:");
        e.printStackTrace();

        SimpleAppletUtil.printAppletInfo(this);
        startOk=true;

        validateTestResult();
    }


    /**
     * Applet Lifecycle
     *
     * Called by the browser or applet viewer to inform this applet that it should start its execution. 
     */
    public void start()
    {
		System.out.println("Applet "+appletName+" start ..(isApplet: "+isApplet+")");

        Exception e = new Exception("Applet "+appletName+".start(): StackTrace:");
        e.printStackTrace();
    }

    public void validateTestResult() {
        if(!initOk) {
            testResult=false;
        } else {
            testResult=test();
        }
        String res = testResult?T_PASSED:T_FAILED;
        System.out.println(T_TAG+" "+res);
        invalidate();
        validate();
        repaint();
    }

    /**
     * Applet Lifecycle
     *
     * Called by the browser or applet viewer to inform this applet that it should stop its execution.
     */
    public void stop()
    {
		System.out.println("Applet "+appletName+" stop ..");
        SimpleAppletUtil.printAppletInfo(this);

        Exception e = new Exception("Applet "+appletName+".stop(): StackTrace:");
        e.printStackTrace();
    }


    /**
     * Applet Lifecycle
     *
     * Called by the browser or applet viewer to inform this applet that it is being reclaimed and that it should destroy any resources that it has allocated.
     */
    public void destroy()
    {
		System.out.println("Applet "+appletName+" destroy ..");
        SimpleAppletUtil.printAppletInfo(this);

        Exception e = new Exception("Applet "+appletName+".destroy(): StackTrace:");
        e.printStackTrace();
    }


    /**
     * Object Lifecycle
     *
     */
    protected void finalize()
    	throws Throwable
    {
		System.out.println("Applet "+appletName+" finalize ..");
	  
        Exception e = new Exception("Applet "+appletName+".finalize(): StackTrace:");
        e.printStackTrace();

        super.finalize();
    }

    public void paint(Graphics g)
    {
        String res = (startOk)?(testResult?T_PASSED:T_FAILED):"pre test";
        g.drawString("Class "+getClass().getName()+": "+res, 20, 20);
        g.drawString("Test "+appletName+": "+res, 20, 40);
        g.drawString("prod: "+version_product+", plat: "+version_platform, 20, 60);
        g.drawString("rel.: "+isRelaunch+", args: "+jvm_args, 20, 80);
    }

    public abstract boolean test ();

}

