/**
 * Test-Auto: console
 * 
 */

import java.applet.*;
import java.lang.*;
import java.util.*;
import java.net.*;
import java.io.*;

import javax.jnlp.*;

public class JNLPExtendedService1 extends SimpleAppletBase 
{
    static final String datum = "0123456789";

    public boolean test ()
    {
        ExtendedService es = null;
        boolean ok = true;

        String tmpfilename = getParameter("tmpfile");

        try {
           // Lookup the javax.jnlp.ExtendedService object
           es = (ExtendedService)ServiceManager.lookup("javax.jnlp.ExtendedService");
        } catch(UnavailableServiceException ue) { 
           System.out.println(ue) ; ue.printStackTrace(); 
           // Service is not supported
           ok=false;
        }
        if (!ok) return false;

        // Open a specific file in the local machine
    
        File tmpfile = new File(tmpfilename);

        // Java Web Start will pop up a dialog asking the user to grant permission
        // to read/write the file 'tempfile'
    
        try {
            FileContents fc_tmpfile = es.openFile(tmpfile);

            if(!tmpfilename.equals(fc_tmpfile.getName()))
            {
              System.out.println("\t tmpfile(out): "+tmpfilename+", unequal to fc-filename: "+fc_tmpfile.getName()+" - info");
              // ok=false;
              // return ok;
            }

            if(!fc_tmpfile.canWrite())
            {
              System.out.println("\t outfile: "+tmpfilename+", no write access (may not exist yet) - info");
            }

            // You can now use the FileContents object to read/write the file
            
            java.io.OutputStream sout = fc_tmpfile.getOutputStream(true);

            BufferedWriter bwsout = new BufferedWriter( new OutputStreamWriter(sout) );

            bwsout.write(datum, 0, datum.length());
            bwsout.flush();
            bwsout.close();
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("\t Error while IO write - failed");
            ok=false;
            return ok;
        }


        // read back ..
        try {
            FileContents fc_tmpfile = es.openFile(tmpfile);

            if(!tmpfilename.equals(fc_tmpfile.getName()))
            {
              System.out.println("\t tmpfile(in): "+tmpfilename+", unequal to fc-filename: "+fc_tmpfile.getName()+" - info");
              // ok=false;
              // return ok;
            }

            if(!fc_tmpfile.canRead())
            {
              System.out.println("\t outfile: "+tmpfilename+", read access failed");
              ok=false;
            }

            // You can now use the FileContents object to read/write the file
            
            java.io.InputStream  sin  = fc_tmpfile.getInputStream();
            BufferedReader       brsin = new BufferedReader( new InputStreamReader(sin) ) ;
            String               in    = brsin.readLine();

            if(!in.equals(datum))
            {
              System.out.println("\t file content <"+in+"> does not match <"+datum+"> - failed");
              ok=false;
            }
            brsin.close();
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("\t Error while IO read - failed");
            ok=false;
            return ok;
        }

        return ok;
    } 
}

