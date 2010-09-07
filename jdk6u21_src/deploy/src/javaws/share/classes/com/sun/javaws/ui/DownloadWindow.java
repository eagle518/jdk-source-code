/*
 * @(#)DownloadWindow.java	1.125 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 *  DownloadWindow
 *
 *  Implementation of the standard Download Progress Dialog
 *
 *  Call sequence:
 *
 *      DownloadWindow dw = new DownloadWindow();
 *      ...
 *      dw.initialize(parent, launchDesc, exit, ok);
 *
 *      then: 
 *
 *      dw.setVisible(true);
 *      ...
 *      dw.showProgressScreen();
 *      ...
 *      dw.setVisible(false);  / dw.disposeWindow()
 *
 */
package com.sun.javaws.ui;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.*;
import javax.swing.border.*;
import java.security.*;
import java.net.URL;
import java.io.File;
import com.sun.javaws.*;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.jnl.IconDesc;
import com.sun.javaws.exceptions.ExitException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.*;
import com.sun.javaws.progress.ProgressListener;

public class DownloadWindow extends WindowAdapter implements 
        ProgressListener, ActionListener {
    
    /**  Reference to the window */
    private ProgressDialog progressDialog = null;

    /** the answer **/
    private int answer = UIFactory.ERROR;

    /** local visability flag **/
    private boolean isVisible = false;

    private boolean allowVisible = false;
    
    /** App. info */
    private AppInfo ainfo = null;
    
    // Download statistics
    private int percentComplete = 0;
    private URL currentUrl = null;
    
    // Cancel does not do a System.exit(-1) if it invoked from 
    // the DownloadService, or as part of re-install or launch-prompt
    private boolean isCanceled = false;
    private boolean exitOnCancel = false;
    private boolean includeOk = false;

    // Estimated download time timer
    private String statusString = null;
    private static final int TIMER_UPDATE_RATE = 1500;     // Update every <N> seconds
    private static final int TIMER_INITIAL_DELAY = 8000;   // Don't show for <N> sec.
    private static final int TIMER_RECENT_SIZE  = 10;      // Length of recent list

    javax.swing.Timer    timerObject =  null;
    private int[]    timerDownloadPercents = new int[TIMER_RECENT_SIZE];
    private int      timerCount = 0;
    private int      timerLastPercent = 0;
    private boolean  timerOn = false;   // Time is turned on if file found in cache

    private String pendingHeading;

    /**
     * DownloadWindow() - 0 arg constructor
     * When using this constructor, need to call initialise() later.
     */
    public DownloadWindow() {
    }

    /** 
     * initialize() - creates and sets up the Progress Dialog.
     * owner - can be null
     * LaunchDesc ld - can be null
     * exit - if true, application will exit when <cancel> button pressed.
     * okBtn - if true, will also show the ok button.
     */
    public void initialize(Component owner, LaunchDesc ld, 
           boolean exitOnCancel, boolean includeOk, boolean includeVendor) {

        // Cancel button behavior
        isCanceled = false;
        this.exitOnCancel = exitOnCancel;
        this.includeOk = includeOk;
        
        if (ld != null) {
            ainfo = ld.getAppInfo();
        } else {
            ainfo = new AppInfo();
            ainfo.setTitle(" ");
            if (includeVendor) {
                ainfo.setVendor(" ");
            }
        }

        String title = ResourceManager.getString(
                        "product.javaws.name", "");

        if (progressDialog == null) {
            // Create the dialog through the UIFactory
             progressDialog = UIFactory.createProgressDialog(ainfo, owner, 
                        title, null, includeOk);
        } else {
            progressDialog.reset(ainfo, title, includeOk);
        }

        progressDialog.getDialog().addWindowListener(this);
    
    }

    public void initialize(LaunchDesc ld, 
                           boolean exitOnCancel, boolean includeOk) {
        initialize(null, ld, exitOnCancel, includeOk, true);
    }

    public Component getOwner() { 
        if (progressDialog == null) {
            return null;
        }
        return (Component) progressDialog.getDialog();
    }

    public void showLoadingProgressScreen() {
        setHeading(ResourceManager.getString("progress.downloading"), true);
        // Setup timer to calculate download time
        timerObject = new javax.swing.Timer(TIMER_UPDATE_RATE, this);
        timerCount = 0;
        timerObject.start();
    }
    
    public void setStatus(String text) {
        statusString = text;
        setStatusStringText(text);
    }

    private void setEstimatedTime(String text) {
        if (statusString == null || statusString.length() == 0) {
            setStatusStringText(text);
        }
    }
    
    private void setStatusStringText(final String text) {
        Runnable action = new Runnable() {
            public void run() {
                // The window might get cleared before this is invoked. 
                // Everything is synchronized on the event-dispatcher thread
                if (progressDialog != null) {
                    progressDialog.setProgressStatusText(text);
                }
            }
        };
        JavawsSysRun.invokeLater(action);
    }

    public void setHeading(final String text, final boolean singleLine) {
        Runnable action = new Runnable() {
            public void run() {
                // The window might get cleared before this is invoked. 
                // Everything is synchronized on the event-dispatcher thread
                if (progressDialog != null) {
                    progressDialog.setMasthead((text == null) ? " " : text, 
                    singleLine);
                }
            }
        };
        pendingHeading = null;
        JavawsSysRun.invokeLater(action);
    }
    
    public void setHeadingLater(String text) {
        if (timerObject != null && timerObject.isRunning()) {
            pendingHeading = text;
        } else {
            setHeading(text, true);
        }
    }

    /**
     * Sets the visibility of the progress bar.
     */
    public void setProgressBarVisible(final boolean isVisible) {
        JavawsSysRun.invokeLater(new Runnable() {
            public void run() {
                // The window might get cleared before this is invoked
                if (progressDialog != null) {
                    int value = (isVisible ? 0 : ProgressDialog.INVISIBLE);
                    progressDialog.showProgress(value);
                }
            }
        });
    }
    
    public void setProgressBarValue(final int value) {
        JavawsSysRun.invokeLater(new Runnable() {
            public void run() {
                // The window might get cleared before this is invoked
                if (progressDialog != null) {
                    progressDialog.showProgress(value);
                }
            }
        });
    }

    /** Changes the progressbar to show a indeterminate status*/
    public void setIndeterminate(final boolean value) {
        JavawsSysRun.invokeLater(new Runnable() {
            public void run() {
                // The window might get cleared before this is invoked
                if (progressDialog != null) {
                    progressDialog.setIndeterminate(value);
                }
            }
        });
    }
    
    public void showLaunchingApplication(final String title) {
        final ProgressDialog pd = progressDialog;
        if (pd != null) {
            JavawsSysRun.invokeLater(new Runnable() {
                public void run() {
                    // The window might get cleared before this is invoked. 
                    pd.setTitle(title);
                    pd.setMasthead(ResourceManager.
                        getString("progress.launching"), true);
                    pd.showProgress(100);
                }
            });
        }
    }
    
    /** Removes all content from the window, but keeps the default
     *  windowClose listener. Make sure to serialize this on the 
     *  event-dispatching thread. Using synchronized might deadlock.
     */
    public void clearWindow() {
        if (SwingUtilities.isEventDispatchThread()) {
            clearWindowHelper();
        } else {
            try {                
                SwingUtilities.invokeAndWait(new Runnable() {
                    public void run() {
                        clearWindowHelper();
                    }
                });
            } catch(Exception e) {
                Trace.ignoredException(e);
            }            
        } 
    }
    
    private void clearWindowHelper() {        
        reset();
    }
    
    
    /** Removes window from the screen */
    public void disposeWindow() {
        if (progressDialog != null) {
            clearWindow();
            exitOnCancel = false;
            progressDialog.getDialog().removeWindowListener(this);
            setVisible(false);
            progressDialog = null;
        }
    }
    
    /** Resets the status's displayed in the window to empty. */
    public void reset() {
        final ProgressDialog d = progressDialog;
        if (d != null) {
            stopTimer();
            JavawsSysRun.invokeLater(new Runnable() {
                public void run() {
                    d.setMasthead("", true);
                    d.showProgress(ProgressDialog.INVISIBLE);
                    d.setProgressStatusText(null);
                }
            });
        }
    }
    

    public void setTitle(final String title) {
       JavawsSysRun.invokeLater(new Runnable() {
           public void run() {
               if (progressDialog != null) {
                   progressDialog.setTitle(title);
               }
           }
       });
    }

    private void showDownloadWindow() {
        // show download window if allowed and not visible yet
        if (allowVisible == true && isVisible() == false) {
            setVisible(true);
        }
    }
    
    public void progress(URL rc, String version, 
                         long readSoFar, long totalSize, int percent) {
        showDownloadWindow();

        timerOn = true;

        percentComplete = percent;

        // Update text fields if neccesary
        if (rc != currentUrl && rc != null) {
            setHeadingLater(ResourceManager.getString("progress.downloading"));
            currentUrl = rc;
        }
        percentComplete = percent;
        
        if (progressDialog != null) {
            if (totalSize == -1) {
                progressDialog.showProgress(ProgressDialog.INVISIBLE);
            } else {
                progressDialog.showProgress(percent);
            }
        }
    }
    
    /**
     * Invoked while an old version is being patched to a new version.
     */
    public void upgradingArchive(URL rc, String version, int patchPercent,
                         int percent) {
        showDownloadWindow();

        if (currentUrl != rc || patchPercent == 0) {
            // only set "Patching" headding if there isn't another pending
            // we want to make sure "Downloading" gets a chance between
            // several { downloading , verifying , patching } cycles.
            if (pendingHeading == null) {
                setHeadingLater(ResourceManager.getString("progress.patching"));
            }
            currentUrl  = rc;
        }
    }
    
    /**
     * This is invoked when resource identified by <code>rc</code> is being
     * verified, i.e., scanned for certificates and checked that potential 
     * signing is coherent
     */
    public void validating(URL rc, String version, long count, 
                           long total, int percent) {
        showDownloadWindow();

        if (currentUrl != rc || count == 0) {
            // only set "Validating" headding if there isn't another pending
            // we want to make sure "Downloading" gets a chance between
            // several { downloading , verifying , patching } cycles.
            if (pendingHeading == null) {
                setHeadingLater(ResourceManager.getString(
                    "progress.verifying"));
            }
            currentUrl  = rc;
        }
    }
    
    /**
     * This is invoked when resource failed to be loaded.
     */
    public void downloadFailed(URL url, String version) {
        stopTimer();
        // Update display
        setHeading(ResourceManager.getString("progress.download.failed"), true);
        if (progressDialog != null) {
            progressDialog.showProgress(ProgressDialog.INVISIBLE);
        }
    }
    
    /**
     * This is invoked when extension-descs are downloaded
     */
    public void extensionDownload(String name, int remaining) {
        // ignore
    }
    
    /**
     * This is invoked when a JRE is downloaded
     */
    public void jreDownload(String version, URL location) {
        setHeading(ResourceManager.getString(
            "progress.download.jre", version), true);
    }

    /** This event is called by the timer */
    public void actionPerformed(ActionEvent e) {

        // Check if a pending heading needs changing
        if (pendingHeading != null) {
            setHeading(pendingHeading, true);
        }

        // Check if timer should be shown
        if (!timerOn || percentComplete <= 0) {
            return;
        }
        timerCount++;

        int timeSoFar = timerCount * TIMER_UPDATE_RATE;

        // Calculate delta and put it into the sliding average array.
        int delta = percentComplete - timerLastPercent;
        timerLastPercent = percentComplete;
        timerDownloadPercents[timerCount % TIMER_RECENT_SIZE] = delta;

        // Check if we should update the text
        if (timeSoFar > TIMER_INITIAL_DELAY) {
            // Calculate current average
            int recentPercent = 0;
            int count = Math.min (TIMER_RECENT_SIZE, timerCount); 
            for (int i = 0; i < count; i++) {
                recentPercent += timerDownloadPercents[i];
            }

            if (percentComplete == 100) {
                setEstimatedTime("");
            } else if (recentPercent == 0) {
                // Stalled download
                // The original message "Download stalled" gives bad user
                // experience
                // do not display anything in this case
            } else if (percentComplete > 0) {
                int totalSecsLeft = ((timeSoFar / 1000) *
                    (100 - percentComplete)) / percentComplete;
                int mins  = totalSecsLeft  / 60;
                int secs = totalSecsLeft - (mins * 60);

                String key, text;
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
                    if (secs == 1) {
                        key = "progress.time.left.second";
                    } else {
                        key = "progress.time.left.seconds";
                    }
                    text = ResourceManager.getString(key, "" + secs);
                }

                setEstimatedTime(text);
            }
        }
    }

    private void stopTimer() {
        timerOn = false;
        if (timerObject != null) {
            timerObject.stop();
        }
        if (pendingHeading != null) {
            setHeading(pendingHeading, true);
        }
    }
    
    /**
     * WindowListener method. Indicates user doesn't want the app to
     * be launched.
     */
    public void windowClosing(WindowEvent we) {
        cancelAction();
        resetCancled();
    }

    private void cancelAction() {
        isVisible = false;
        if (exitOnCancel) {
            // This might be executed with a security manager turned on
            AccessController.doPrivileged(new PrivilegedAction() {
               public Object run() {
                    try {
                       Main.systemExit(-1); 
                    } catch (ExitException ee) { 
                        Trace.println("systemExit: "+ee, TraceLevel.BASIC);
                        Trace.ignoredException(ee);
                    }
                   return null; // Nothing to return
               }
           });
        } else {
            isCanceled = true;            
        }
    }
    
    /* Use by the DownloadService impl. */
    public boolean isCanceled() { return isCanceled; }
    
    /* Resets all arg used in window for a second download. This is mostly for
     * the estimated time and cancel state. */
    public void resetCancled() { 
        isCanceled = false;         
    }

    public void setAllowVisible(final boolean show) {
        allowVisible = show;
    }

    public void setVisible(final boolean show) {
        
        final ProgressDialog d = progressDialog;

        if (show == false) {
            exitOnCancel = false;
            stopTimer();
        }

        if (d != null && show != isVisible) {
            isVisible = show;
            JavawsSysRun.invokeLater(new Runnable() {
                public void run() {
                    if (show) {
                        com.sun.javaws.ui.SplashScreen.hide();

                        // always show dialog thru UIFactory
                        UIFactory.showProgressDialog(d);

                        // now it is a modal dialog, so answer available after
                        // the return from showing
                        answer = d.getUserAnswer();
                        if (answer == UIFactory.CANCEL) {
                            cancelAction();
                        }
                    } else {
                        UIFactory.hideProgressDialog(d);
                    }
                }
            });
        }  else if (d != null && show == false) {
            // we need explicit dispose here for app to exit properly
            // because sometimes we create this dialog w/o ever showing
            UIFactory.hideProgressDialog(d);
        }
    }

    public boolean isVisible() {
        return (progressDialog != null && isVisible);
    }

    public void setApplication(String n, String p, String u) {
        progressDialog.setApplication(n, p, u);
    }


    public int showConfirmDialog(LaunchDesc ld, String message, String title) {
        initialize(null, ld, false, true, true);
        progressDialog.setMasthead(message, false);
        setVisible(true);

        // first wait for dialog to be visible
        do {
            try { 
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
        } while (!this.isVisible());
        
        // now wait for either answer to be given, or dialog to go invisible
        do {
            try { 
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
        } while ((progressDialog.getUserAnswer() == UIFactory.ERROR) &&
                 (this.isVisible()));

        if (this.isVisible()) {
            // alow reuse of this dialog for download progress
            initialize(null, ld, true, false, true);
        }
         return progressDialog.getUserAnswer();
    }
}


