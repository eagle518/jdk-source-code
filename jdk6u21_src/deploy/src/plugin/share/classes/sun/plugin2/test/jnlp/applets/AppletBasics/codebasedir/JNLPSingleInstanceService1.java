/**
 * http://download.java.net/jdk7/docs/technotes/guides/javaws/developersguide/examples.html
 * 
 * Using a SingleInstanceService Service
 * 
 * The javax.jnlp.SingleInstanceService provides a set of methods for applications to register themselves as singletons, and to register listener(s) for handling arguments passed in from different instances of applications.
 * 
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;

import javax.jnlp.*;

public class JNLPSingleInstanceService1 extends SimpleAppletBase 
{
    SingleInstanceService sis = null;
    SISListener sisL = null;
    int activations = 0;
    int activations_expected = 0;
    int sleep = 0;

    /**
     * Applet Lifecycle
     *
     * Called by the browser or applet viewer to inform this applet that it has been loaded into the system.
     */
    public void init()
    {
        super.init();

        String stmp = getParameter("activations_expected");
        try {
            activations_expected = Integer.parseInt(stmp);
        } catch (Exception e) { }
        stmp = getParameter("sleep");
        try {
            sleep = Integer.parseInt(stmp);
        } catch (Exception e) { }
        System.out.println("JNLPSingleInstanceService1.init() 1 - activations_expected: "+activations_expected+", sleep: "+sleep);

        try {
            Thread.currentThread().sleep(sleep*1000); 
        } catch (Exception e) { }
        System.out.println("JNLPSingleInstanceService1.init() 2 - sleep done");

        try {
            // Lookup the javax.jnlp.SingleInstanceService object
            sis = (SingleInstanceService)ServiceManager.lookup("javax.jnlp.SingleInstanceService");

            // Register the single instance listener at the start of your application

            sisL = new SISListener();
            System.out.println("JNLPSingleInstanceService1.init() 3 - pre addSingleListener");
            sis.addSingleInstanceListener(sisL);
            System.out.println("JNLPSingleInstanceService1.init() 4 - post addSingleListener");

        } catch(UnavailableServiceException ue) { 
           sis=null;
           System.out.println(ue) ; ue.printStackTrace(); 
           // Service is not supported
        }
        System.out.println("JNLPSingleInstanceService1.init() 5 - done");
    }

    /**
     * Applet Lifecycle
     *
     * Called by the browser or applet viewer to inform this applet that it should stop its execution.
     */
    public void stop()
    {
        super.stop();

        // Remember to remove the listener before your application exits

        if(sis!=null && sisL!=null) {
            sis.removeSingleInstanceListener(sisL);
        }
    }


    public boolean test ()
    {
       boolean ok=true;
       int i=30;

       if(sis==null)  { System.out.println("\t service failed"); ok=false; }
       if(sisL==null) { System.out.println("\t service-listener failed"); ok=false; }

       while ( i>0 && activations != activations_expected ) { 
          System.out.println("\t"+i+": waiting [activations: "+activations+", expected: "+activations_expected+"]");
          i--;
          try {
              Thread.sleep(1000); // pause for 1s
          } catch (Exception e) { }
       }
       if ( activations != activations_expected ) { 
          System.out.println("\t activations: "+activations+", unequal expected: "+activations_expected+" - failed");
          ok=false;
       }

       return ok;
   }

    // Implement the SingleInstanceListener for your application
    
   class SISListener implements SingleInstanceListener {
        public void newActivation(String[] params) {
            
            // your code to handle the new arguments here
            for(int i=0; i<params.length; i++) {
                System.out.println("SISListener.newActivation param "+i+": "+params[i]);
            }
            
            activations++;

            System.out.println("SISListener.newActivation done num: "+activations);
        }
   }

}

