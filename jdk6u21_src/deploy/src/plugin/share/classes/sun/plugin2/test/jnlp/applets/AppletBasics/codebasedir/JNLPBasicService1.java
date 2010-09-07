/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;

import javax.jnlp.*;

public class JNLPBasicService1 extends SimpleAppletBase 
{
    public boolean test ()
    {
        URL codeBase=null;
        boolean isOffline=false, isBrowserSupported=false;
        boolean ok=true;
        String showUrlStr = null;
        URL showUrl = null;

        try {
            isOffline = Boolean.valueOf(getParameter("offline")).booleanValue();
            isBrowserSupported = Boolean.valueOf(getParameter("browsersupported")).booleanValue();
            codeBase = getCodeBase();
            showUrlStr = getParameter("showurl");
            ok = true;
        } catch (Exception e) { System.out.println(e) ; e.printStackTrace(); }

        if (!ok) return false;

        try {
           // Lookup the javax.jnlp.BasicService object
           BasicService bs = (BasicService)ServiceManager.lookup("javax.jnlp.BasicService");
           URL bs_codeBase = bs.getCodeBase();
           boolean bs_isOffline = bs.isOffline();
           boolean bs_isBrowserSupported = bs.isWebBrowserSupported();
           try { 
               if(showUrlStr!=null) {
                   showUrl = new URL(bs_codeBase, showUrlStr);
               }
           } catch (Exception e) { System.out.println(e) ; e.printStackTrace(); }

           System.out.println("codeBase (applet): "+codeBase);
           System.out.println("codeBase (jnlpbs): "+bs_codeBase);
           System.out.println("isOffline(applet): "+isOffline);
           System.out.println("isOffline(jnlpbs): "+bs_isOffline);
           System.out.println("isBrowserSupported(applet): "+isBrowserSupported);
           System.out.println("isBrowserSupported(jnlpbs): "+bs_isBrowserSupported);
           System.out.println("showurl: "+showUrlStr+" -> "+showUrl);

           if(!bs_codeBase.equals(codeBase)) { System.out.println("\tcodebase failed\n\tbs_codeBase: " + bs_codeBase + "\n\tcodeBase: " + codeBase); ok=false; }
           if(bs_isOffline!=isOffline) { System.out.println("\tisOffline failed"); ok=false; }
           if(bs_isBrowserSupported!=isBrowserSupported) { System.out.println("\tisbrowserSupported failed"); ok=false; }

           if(showUrl!=null) {
               bs.showDocument(showUrl);
           }

       } catch(UnavailableServiceException ue) { 
           System.out.println(ue) ; ue.printStackTrace(); 
           // Service is not supported
           ok=false;
       }
       return ok;
    } 
}

