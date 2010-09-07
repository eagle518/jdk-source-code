/*
 * @(#)AppletImageFactory.java	1.21 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;

import java.awt.Toolkit;
import java.awt.Image;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.lang.ref.SoftReference;
import java.net.URL;
import sun.plugin.security.PluginClassLoader;

import com.sun.deploy.util.Trace;

/*
 * Helper class that create applet image.
 */

final class AppletImageFactory
{
    /**
     * Reconsitute the image.  Only called when the ref has been flushed.
     */
    static Image createImage(final URL url) 
    {
	Image img = (Image) AccessController.doPrivileged(
	    new PrivilegedAction() 
	    {
		public Object run() 
		{
		    // The following code is to resolve JDK 1.1
		    // compatibility with Internet Explorer.
		    // 
		    // In IE, resources are always searched in 
		    // the archives first before searching the
		    // codebase, but this is not the case in
		    // NS or appletviewer. 
		    //
		    // To enhance compatibility, Java Plug-in
		    // will search the archives first before
		    // the codebase.
		    //
		    // The following step has been taken:
		    //
		    // We check if the url is from the base URL
		    // of the classloader. If so, we try to 
		    // load the resources using getResourcesAsStream
		    // with the following sequences:
		    //
		    // 1. Load resources from archives
		    // 2. Load resources from codebase
		    //
		    // [stanleyh]
		    try
		    {
			Thread t = Thread.currentThread();
			ClassLoader cl = t.getContextClassLoader();

			if (cl != null && cl instanceof PluginClassLoader)
			{
			    PluginClassLoader pcl = (PluginClassLoader) cl;

			    // Determine classloader URL
			    String clsLoaderURLString = pcl.getBaseURL().toString();

			    // Separate path and resource name - resource name 
			    // may contain directory structure
			    //
			    String imageURLString = url.toString();
			    int i = imageURLString.indexOf(clsLoaderURLString);

			    if (i == 0) 
			    {
				String imageName;

				// This is to fix problem like 
				//
				// http://fdl.msn.com/zone/games/SAC/DIMI//image/click2.gif
				//
				// so the extra '/' is eliminated
				//
				if (imageURLString.length() > clsLoaderURLString.length())
				{
				   if (imageURLString.charAt(clsLoaderURLString.length()) == '/')
				      imageName = imageURLString.substring(clsLoaderURLString.length() + 1);
				   else
				      imageName = imageURLString.substring(clsLoaderURLString.length());
				}
				else 
				   return null;

				// Get resources from classloader as stream
				InputStream is = pcl.getResourceAsStreamFromJar(imageName);

				// If stream exists
				if (is != null) {
				   return getImage(is);
				}
			    }
			}

			// The base URL doesn't match or no
			// context classloader, so load
			// the resources from URL directly.
			//
                       
			Image image = Toolkit.getDefaultToolkit().createImage(url);
			return image;

		    } catch (java.io.FileNotFoundException e) {
                        // To maintain backward compatability a missing image
                        // at a given url should return an empty ImageObject
                        // instead of null
                        return Toolkit.getDefaultToolkit().createImage(new byte[0]);
                    } catch (Exception e) {
			Trace.ignoredException(e);

			// If we have any exception or unable to load the 
			// resource, return null;
			//
			return null;
		    }
		}
	    });

	return img;
    }
    
    private static Image getImage(InputStream is) throws Exception {
  
        BufferedInputStream bis = new BufferedInputStream(is);
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buffer = new byte[8192];
        
        int byteRead = 0;
        
        // Read the stream until it is EOF
        while ((byteRead = bis.read(buffer, 0, 8192)) != -1)
            bos.write(buffer, 0, byteRead);
        
        // Close input stream
        bis.close();
        
        // Convert to byte array
        byte[] data = bos.toByteArray();
        
        // Return image only if data length is not zero
        if (data != null && data.length > 0)
            return Toolkit.getDefaultToolkit().createImage(data);
        
        return null;
    }
}

