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

public class JNLPPersistenceService1 extends SimpleAppletBase 
{
    String newdatum = null;
    String olddatum = null;

    PersistenceService ps = null;
    BasicService bs = null;

    URL codebase;
    int [] tags;
    String [] muffins;
    URL []    muffinURLs;

    public boolean test ()
    {
        boolean ok = true;
        int i;
        String str;

        try {
           // Lookup the javax.jnlp.PersistenceService object
           bs = (BasicService)ServiceManager.lookup("javax.jnlp.BasicService");
           ps = (PersistenceService)ServiceManager.lookup("javax.jnlp.PersistenceService");
        } catch(UnavailableServiceException ue) { 
           System.out.println(ue) ; ue.printStackTrace(); 
           // Service is not supported
           ok=false;
           return ok;
        }

        newdatum = getParameter("newdatum");
        olddatum = getParameter("olddatum");

        // init and browse all muffins
        try {
            // find all the muffins for our URL
            codebase = bs.getCodeBase();
            muffins  = ps.getNames(codebase);

            System.out.println("Muffins available for url: "+codebase+": "+muffins.length);

            if ( muffins.length == 0 && olddatum.length()>0 ) {
                    System.out.println("Error: No Muffins available for url: "+codebase+": "+muffins.length+"; but having olddatum, ie not init mode - failed");
                    return false;
            }

            if ( olddatum.length()==0) {
                muffins = new String[1];
                muffins[0] = new String("");
            }

            // get the attributes (tags) for each of these muffins. 
            // update the server's copy of the data if any muffins 
            // are dirty 
            tags = new int[muffins.length]; 
            muffinURLs = new URL[muffins.length]; 
            for (i = 0; i < muffins.length; i++) { 
                muffinURLs[i] = new URL(codebase.toString() + muffins[i]); 
                if ( i==0 && olddatum.length()==0) {
                    if ( ! writeCreateMuffin (-1, 0, "", 1024, true) ) {
                        System.out.println("Error: Initial write/creation of Muffins failed");
                        return false;
                    }
                    System.out.println("Initial reset/creation of Muffin");
                }
                tags[i] = ps.getTag(muffinURLs[i]); 

                System.out.println("muffin["+i+"]: "+muffins[i]+" -> "+muffinURLs[i]+", "+ tag2str(tags[i]));
                str = readMuffin (0, i, true);
                System.out.println("\tmuffin["+i+"]: "+str);

                // update the server if anything is tagged DIRTY 
                if (tags[i] == PersistenceService.DIRTY) { 
                    ok=doUpdateServer(i); 
                }
            } 
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("Error while init/browse all muffin");
            // return false;
        }

        // read in the contents of a muffin and then delete it 
        try {
            str = readMuffin (1, 0, true);
            if ( olddatum.length()>0 && !str.equals(olddatum)) {
                System.out.println("0 - muffin old data verify failed");
                ok=false;
            }
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("1 - muffin old data verify failed (exception)");
            ok=false;
        }

        if ( newdatum.length()>0) {
            // rewrite the muffin
            if ( ! writeMuffin (2, 0, newdatum, true) ) {
                    System.out.println("2 - write of Muffins newdatum failed");
                    ok=false;
            }

            str = readMuffin (3, 0, true);
            if (!str.equals(newdatum)) {
                System.out.println("3 - muffin data write verify failed");
                ok=false;
            }
        } else {
            // delete the muffin ..
            try {
                ps.delete(muffinURLs[0]); 
            } catch (Exception e) { 
                System.out.println(e) ; e.printStackTrace(); 
                System.out.println("X - muffin delete failed (exception)");
                ok=false;
            }
        }

        return ok;
    } 

    public String tag2str (int tag)
    {
            if (tag == PersistenceService.DIRTY) { 
                return "DIRTY";
            } else if (tag == PersistenceService.TEMPORARY) { 
                return "TEMPORARY";
            } else if (tag == PersistenceService.CACHED) { 
                return "CACHED";
            } else {
                return "UNKNOWN/ERROR";
            }
    }

    public String readMuffin (int n, int idx, boolean verbose)
    {
        String in=null;
        try {
            FileContents fc = ps.get(muffinURLs[idx]); 
            long maxsize = fc.getMaxLength(); 
            BufferedReader brsin = new BufferedReader( new InputStreamReader(fc.getInputStream()) ) ;
            in = brsin.readLine();
            brsin.close(); 
            if(verbose)
                System.out.println("read-"+n+": muffin["+idx+"]: "+in+", maxsize:"+maxsize);
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("\t Error while IO read - "+n+": muffin["+idx+"]");
        }
        return in;
    }

    public boolean writeCreateMuffin (int n, int idx, String datum, long maxsize, boolean verbose)
    {
        FileContents fc;

        try {
            ps.delete(muffinURLs[idx]); 
        } catch (Exception e) { } // ignore message

        try {
            ps.create(muffinURLs[idx], maxsize); 
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("\t Error while IO create - "+n+": muffin["+idx+"]");
            return false;
        }

        return writeMuffin (n, idx, datum, verbose);
    }

    public boolean writeMuffin (int n, int idx, String datum, boolean verbose)
    {
        FileContents fc;
        long maxsize;

        try {
            fc = ps.get(muffinURLs[idx]);  // reget the new created
            maxsize = fc.getMaxLength();

            // don't append - overwrite!
            BufferedWriter bwsout = new BufferedWriter( new OutputStreamWriter(fc.getOutputStream(true)) ); 
            bwsout.write(datum, 0, datum.length());
            bwsout.flush();
            bwsout.close();

            if(verbose)
                System.out.println("write-"+n+": muffin["+idx+"]: "+datum+", maxsize:"+maxsize);
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("\t Error while IO write - "+n+": muffin["+idx+"]");
            return false;
        }
        return true;
    }

    boolean doUpdateServer(int idx)
    {
        try {
            // update the server's copy of the persistent data 
            // represented by the given URL 
            ps.setTag(muffinURLs[idx], PersistenceService.CACHED); 
        } catch (Exception e) { 
            System.out.println(e) ; e.printStackTrace(); 
            System.out.println("\t Error while updating muffin["+idx+"] URL: "+muffinURLs[idx]);
            return false;
        }
        return true;
    } 

}

