/*
 * @(#)CustomProgress.java	1.12 10/05/13
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.progress;

import com.sun.javaws.exceptions.JNLPException;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import javax.jnlp.DownloadServiceListener;
import java.net.URL;
import java.util.ArrayList;
import sun.awt.AppContext;

public class CustomProgress implements DownloadServiceListener, 
    ProgressListener {

    private DownloadServiceListener _dsp = null;

    /* do nothing till _appThreadGroup is set */
    private ThreadGroup _appThreadGroup = null;

    boolean isLoaded = false;
    Exception exception = null;

    public void waitTillLoaded() throws IOException, JNLPException {
        synchronized (this) {
          while (!isLoaded) {
             try {
                 wait();
             } catch (InterruptedException ioe) {
                AppContext c = AppContext.getAppContext();
                if (c == null || c.isDisposed()) {
                    return;
                }
             }
             if (exception != null) {
                 if (exception instanceof IOException) {
                     throw (IOException) exception;
                 } else if (exception instanceof JNLPException) {
                     throw (JNLPException) exception;
                 } else {
                     throw new RuntimeException(exception);
                 }
             }
          }
        }
        return;
    }

    public synchronized void markLoaded(Exception error) {
        isLoaded = true;
        exception = error;
        this.notifyAll();
    }

    /*
     * null constructor makes a dummy to show no progress
     */
    public CustomProgress() {
       _dsp = null;
       setupCleanupProcedure();
    }

    /* 
     * it becomes real after a call to setListener()
     * gives it a DownloadServiceListener.
     */
    public void setListener(DownloadServiceListener dsp) {
        _dsp = dsp;
    }

    public void setAppThreadGroup(ThreadGroup appThreadGroup) {
        _appThreadGroup = appThreadGroup;
    }

    public DownloadServiceListener  getListener() {
        return _dsp;
    }

    int rescaleBound = -1; //-1 == UNDEFINED, i.e. no notifications were delivered to actual progress yet

    private int rescale(int overall) {
        if (rescaleBound == -1) {
            rescaleBound = overall;
            return 0;
        } else {
            if (rescaleBound == 100) {
                return 100;
            }
            if (overall < rescaleBound) {
                return 0;
            }
            // scale "overall" from [rescaleBound .. 100] to [0 .. 100] range
            int result = (int) (100*(overall - rescaleBound))/(100-rescaleBound);
            return result;
        }
    }

    public void upgradingArchive(final URL url, final String version, 
                         final int patchPercent, final int overallPercent) {
        if (_dsp != null && isReady()) {
            final DownloadServiceListener dsp = _dsp;
            final int overall = rescale(overallPercent);
            doRun(new Runnable() {
                public void run() {
                    dsp.upgradingArchive(url, version, patchPercent, overall);
                }
            });
        }
    }

    // The percentage might not be readSoFar/total due to validation pass
    // The total and percentage will be -1 for unknown
    public void progress(final URL url, final String version, 
        final long readSoFar, final long total, final int overallPercent) {
        if (_dsp != null && isReady()) {
            final DownloadServiceListener dsp = _dsp;
            final int overall = rescale(overallPercent);
            Runnable runner = (new Runnable() {
                public void run() {
                    dsp.progress(url, version, readSoFar, total, overall);
                }
            });
            doRun(runner);
        }
    }

    public void validating(final URL url, final String version, final long entry,
                           final long total, final int overallPercent) {
        if (_dsp != null && isReady()) {
            final DownloadServiceListener dsp = _dsp;
            final int overall = rescale(overallPercent);
            Runnable runner = (new Runnable() {
                public void run() {
                    dsp.validating(url, version, entry, total, overall);
                }
            });
            if (overallPercent < 100) {
                doRun(runner);
            } else {
                forceRun(runner);
            }
        }
    }

    public void downloadFailed(final URL url, final String version) {
        if (_dsp != null) {
            final DownloadServiceListener dsp = _dsp;
            doRun(new Runnable() {
                public void run() {
                    dsp.downloadFailed(url, version);
                }
            });
        }
    }

    public void done() {
        // do nothing
    }

    public void extensionDownload(String name, int remaining) {
        // do nothing
    }

    public void jreDownload(String versionId, URL location) {
        // do nothing
    }

    public void setHeading(final String text, final boolean singleLine) {
        // do nothing
    }

    public void setStatus(String text) {
        // do nothing
    }

    public java.awt.Component getOwner() {
        return (java.awt.Component) null;
    }

    public void setVisible(final boolean show) {
        // do nothing
    }

    public void setProgressBarVisible(final boolean isVisible) {
        // do nothing
    }

    public void setProgressBarValue(final int value) {
        // do nothing
    }

    public void showLaunchingApplication(final String title) {
        // do nothing
    }

    // flush waits for the calls to the DownloadServiceListener to be
    // processed before returning.
    public void flush() {
        while (true) {
            synchronized(queue) {
                if (queue.isEmpty()) {
                    return;
                }
            }
            try {
                Thread.sleep(50);
            } catch (InterruptedException e) { 
                AppContext c = AppContext.getAppContext();
                if (c == null || c.isDisposed()) {
                    return;
                }
            }
        }
    }


    // progress queue
    private final ArrayList queue = new ArrayList();
    private Thread progressThread = null;
    private ProgressQueueChecker checker = null;
    private boolean disposed = false;

    private void setupCleanupProcedure() {
        AppContext c = AppContext.getAppContext();
        if (c != null) {
            c.addPropertyChangeListener(sun.awt.AppContext.DISPOSED_PROPERTY_NAME,
                    new PropertyChangeListener() {
                       public void propertyChange(PropertyChangeEvent evt) {
                           boolean isDisposed = ((Boolean) evt.getNewValue()).booleanValue();
                           if (isDisposed) {
                               synchronized (queue) {
                                   //can not delete queue as it used for synchronization
                                   //clear it, to avoid stuck in flush()
                                   queue.clear();
                                   queue.notifyAll();
                               }
                               checker = null;
                               progressThread = null;
                               disposed = true; //do not accept new jobs
                           }
                       }
           });
        }
    }

    private boolean isReady() {
        return (_appThreadGroup != null);
    }

    private void doRun(Runnable action) {
        if (isReady()) {
            if (progressThread == null) {
                if (checker == null) {
                    checker = new ProgressQueueChecker();
                }
                progressThread = new Thread(_appThreadGroup,
                                 checker, "javawsApplicationMain");
                progressThread.setName("ProgressChecker");
                progressThread.setDaemon(true);
                progressThread.start();
            }
            enQueue(action);
        }
    }

    private void forceRun(Runnable action) {
        if (_appThreadGroup != null) {
            if (progressThread == null) {
                if (checker == null) {
                    checker = new ProgressQueueChecker();
                }
                progressThread = new Thread(_appThreadGroup,
                                 checker, "javawsApplicationMain");
                progressThread.setDaemon(true);
                progressThread.start();
            }
            synchronized(queue) {
                if (!disposed) {
                    queue.clear();
                    queue.add(action);
                    queue.notifyAll();
                }
            }
            flush();
        }
    }

    private void enQueue(Runnable action) {
        synchronized(queue) {
            if (!disposed) {
                // Add element to queue
                queue.add(action);

                // Notify Trace thread
                queue.notifyAll();
            }
        }
    }

    private class ProgressQueueChecker implements Runnable {
        public void run() {
            Runnable action = null;
            while (true) {
              // check if there are any messages in the queue
              synchronized (queue) {
                if (queue.isEmpty()) {
                    try {
                        // queue is empty
                        queue.wait();
                    } catch (InterruptedException ie) {
                        AppContext c = AppContext.getAppContext();
                        if (c == null || c.isDisposed()) {
                            return;
                        }
                    }
                } else {
                    // Remove action from queue
                    try {
                        action = (Runnable) queue.remove(0);
                    } catch (ClassCastException cce) {
                        // non Runnable on this queue ?
                    }
                }
              }
              if (action != null) {
                  action.run();
              }
           }
       }
    }

}
