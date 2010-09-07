/*
 * @(#)CacheUpdateProgressDialog.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.ui;

import javax.swing.SwingUtilities;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.Environment;

public class CacheUpdateProgressDialog {

    public static class CanceledException extends Exception {
    }

    private static boolean dialogWasVisible = false;
    
    private static ProgressDialog dialog = null;
    private static boolean systemCache = false;
    private static final int TIMER_INITIAL_DELAY = 8000;   // Don't show for <N> sec.
    private static String JAVA_VERSION = "6";

    public static void dismiss() {
	if (dialog != null) {
	    final ProgressDialog pd = dialog;
	    SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			pd.setVisible(false);
		    }
	        });
	}
	dialog = null;
    }

    public static void setSystemCache(boolean system) {
        systemCache = system;
    }
    
    private static int currentPercent = -1;
    private static long startTime = -1;
    
    public static void showProgress(int done, int total) 
        throws CanceledException {
        String text = null;
	int percent = (100 * done) / ((total > 0) ? total : 1);
        if (currentPercent == -1) {
            currentPercent = percent;
            startTime = System.currentTimeMillis();
        } else if (currentPercent != percent) {
            currentPercent = percent;
            long currentTime = System.currentTimeMillis();
            long timeUsed = currentTime - startTime;
            long totalTime = timeUsed * 100 / percent;
            // time left in seconds
            int totalSecsLeft = (int)((totalTime - timeUsed) / 1000);
            int mins  = totalSecsLeft  / 60;
            int secs = totalSecsLeft - (mins * 60);
            
            String key;
            if (mins > 0) {
                if (mins == 1) {
                    if (secs == 1) {
                        key = "progress.time.left.minute.second";
                    } else {
                        key = "progress.time.left.minute.seconds";
                    }
                } else {
                    if (secs == 1) {
                        key = "progress.time.left.minutes.second";
                    } else {
                        key = "progress.time.left.minutes.seconds";
                    }
                }
                text = ResourceManager.getString(key,
                        "" + mins, "" + secs);
            } else {
                if (secs == 1 || secs == 0) {
                    key = "progress.time.left.second";
                } else {
                    key = "progress.time.left.seconds";
                }
                text = ResourceManager.getString(key, "" + secs);
            }
        }
     
	if (dialog == null) {
	    AppInfo ainfo = new AppInfo();
	    String title = ResourceManager.getString(Environment.isJavaPlugin() ? 
		"cache.upgrade.title.javapi" : "cache.upgrade.title.javaws");
            String masthead = ResourceManager.getString(Environment.isJavaPlugin() ?
		"cache.upgrade.masthead.javapi" : "cache.upgrade.masthead.javaws");
	    String msg = ResourceManager.getString(Environment.isJavaPlugin() ?
		"cache.upgrade.message.javapi" : "cache.upgrade.message.javaws");
       
	    dialog = UIFactory.createProgressDialog(ainfo, null, title, msg, false);
	    dialog.setMasthead(masthead, true);
                    
	    final ProgressDialog pd = dialog;
	    try {
	        SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
		        UIFactory.showProgressDialog(pd);
		    }
	        });
	    } catch (Exception e) {
		Trace.ignored(e);
	    }
	} else if (dialog.getDialog().isVisible()) {
	    dialogWasVisible = true;
	} else if (dialogWasVisible) {
	    if (!dialog.getDialog().isVisible()) {
	        throw new CanceledException();
	    }
	}
        if (text != null && System.currentTimeMillis() >= 
                (startTime + TIMER_INITIAL_DELAY)) {             
            dialog.setProgressStatusText(text);
        }
	dialog.progress(percent);
      
    }
}
