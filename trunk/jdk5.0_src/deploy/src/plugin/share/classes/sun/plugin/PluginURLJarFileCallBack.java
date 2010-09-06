/*
 * @(#)PluginURLJarFileCallBack.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Enumeration;
import java.util.jar.JarFile;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;
import java.security.cert.Certificate;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import sun.net.www.protocol.jar.URLJarFile;
import sun.net.www.protocol.jar.URLJarFileCallBack;


/*
 * This class implements function retrieve, in order to add functionality
 * to URLJarFile and handle cache failure when downloading JARJAR file.
 * 
 */

public class PluginURLJarFileCallBack implements URLJarFileCallBack
{

    private static int BUF_SIZE = 8192;
                
    public PluginURLJarFileCallBack()
    {}

    public JarFile retrieve(final URL url) throws IOException
    {
        JarFile result = null;

        /* get the stream before asserting privileges */
	final URLConnection conn =  url.openConnection();
	conn.getInputStream();

	try 
        { 
	    result = (JarFile)
		AccessController.doPrivileged(new PrivilegedExceptionAction() {
		    public Object run() throws IOException 
		    {
			InputStream is = null;
			OutputStream os = null;

			try 
			{
			    is = new BufferedInputStream(conn.getInputStream());

                            // Create a temp file in cache.
			    File tmpFile = File.createTempFile("jar_cache", null);
			    os = new BufferedOutputStream(new FileOutputStream(tmpFile));
			    
			    int read = 0;

			    byte[] buf = new byte[BUF_SIZE];

			    while ((read = is.read(buf)) != -1) {
				os.write(buf, 0, read);
			    }

			    os.close();
			    os = null;

                            //Check if this is the JARJAR file...
                            if (url.toString().toUpperCase().endsWith(".JARJAR"))
                            {
                                /* 
                                 * It is.  See what is inside the JARJAR file
                                 * There should be only manifest (optional) and
                                 * only one JAR file.
                                 */
                                URLJarFile tmpJar = new URLJarFile(tmpFile);
                                Enumeration e = tmpJar.entries();
                                ZipEntry ze = null;
                                
                                // Track number of JAR files inside the JARJAR.  If there is more
                                // then one, nothing should load, and exception should be thrown.
                                int jarCounter = 0;
                                
                                while(e.hasMoreElements())
                                {                                
                                    ze = (ZipEntry) e.nextElement();
                                    if (ze.getName().toUpperCase().startsWith("META-INF") )
                                        //This is a manifest file, ignore it.
                                        ;
                                    else if (! ze.toString().toUpperCase().endsWith(".JAR"))
                                        // There must not be anything but a JAR inside a JARJAR file.
                                        throw new IOException("Invalid entry in jarjar file");
                                    
                                    else
                                    {
                                        //Increment JAR Counter.
                                        jarCounter++;  
                                        if (jarCounter>1)
                                            break;
                                    }
                                } // end while

                                if ( jarCounter > 1)
                                {
                                    //There is more then one JAR file inside the JARJAR file,
                                    //throw an exception.
                                    throw new IOException ("Multiple JAR files inside JARJAR file");
                                }
                                
                                else
                                {
                                        //Extract the nested JAR file to the temp directory
                                        InputStream nestedIn = null;
                                        OutputStream nestedOut = null;

                                        try
					{
                                            File nestedJar = File.createTempFile("jar_cache", null);
                                            nestedIn = new BufferedInputStream(tmpJar.getInputStream(ze));
                                            nestedOut = new BufferedOutputStream(new FileOutputStream(nestedJar));
                                            read = 0;

                                            while ((read = nestedIn.read(buf)) != -1)
                                                nestedOut.write(buf, 0, read);

					    nestedOut.close();
					    nestedOut = null;

                                            tmpJar = new URLJarFile(nestedJar);
                                        }
                                        finally
                                        {
                                            if (nestedIn != null)
                                                nestedIn.close();
                                        
                                            if (nestedOut != null)
                                                nestedOut.close();
                                        }
                                    }                             
                                return tmpJar;
                                
                              
                            }  
                            // This is not a JARJAR file, just a JAR, return it.
			    return new URLJarFile(tmpFile);
			} 
                        finally {
			    if (is != null) {
				is.close();
			    }
			    if (os != null) {
				os.close();
			    }
			}
		    }
		});
	} catch (PrivilegedActionException pae) {
	    throw (IOException) pae.getException();
	}

        return result;
    }
}
