/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;

public class SimpleApplet1 extends SimpleAppletBase 
{
    public boolean test ()
    {
        int w = getWidth();
        int h = getHeight();
        String name = getParameter("NAME"); // the std in applet name
        String p1v = getParameter("param1");
        String p2v = getParameter("param2");
        URL cb = getCodeBase();

        boolean ok=true;

        // if(w!=800) ok=false;
        if(h!=120) ok=false;

        if(!name.equals("SimpleApplet1Num1")) ok=false;
        if(!p1v.equals("param1_value")) ok=false;
        if(!p2v.equals("param2_value")) ok=false;

        return ok;
    }
}

