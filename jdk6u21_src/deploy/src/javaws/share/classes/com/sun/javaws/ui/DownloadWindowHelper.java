/*
 * @(#)DownloadWindowHelper.java	1.7 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import com.sun.deploy.resources.ResourceManager;
import com.sun.javaws.Globals;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.ui.ComponentRef;
import java.awt.Component;
import com.sun.javaws.progress.Progress;
import com.sun.javaws.progress.ProgressListener;

/*
   Lazy version of DownloadWindow that is used to postpone creation of 
   Swing objects and initialization of Swing.
*/
public class DownloadWindowHelper {
    DownloadWindow dw;
    boolean disabled = false;
    private LaunchDesc _launchDesc;
    boolean b1, b2;

    public DownloadWindow get() {

        if (dw == null && !disabled) {
            if (!Progress.usingCustomProgress()) {
                dw = new DownloadWindow();
                if (!Globals.isSilentMode()) {
                    dw.initialize(_launchDesc, true, false);
                }
            }
        }
        return dw;
    }

    public ProgressListener getProgressListener() {
        if (Progress.usingCustomProgress()) {
            return Progress.getCustomProgress();
        } else {
            return get();
        }
    }
    
    public void initialize(LaunchDesc _launchDesc, boolean b1, boolean b2) { 
        this._launchDesc = _launchDesc;
        this.b1 = b1;
        this.b2 = b2;
    }
    
    public void disable() {
        disabled = true;
    }
    
    public ComponentRef getOwnerRef() {
        return new ComponentRef() {
            public Component get() {
                return DownloadWindowHelper.this.getOwner();
            }
        };                    
    }
    
    public Component getOwner() {
        DownloadWindow d = get();            
        if (d == null) {
            return null;
        }
        return d.getOwner();
    }
    
    public void reset() {
        if (dw != null) {
            dw.reset();
        }
    }

    public void setVisible(boolean vis) {
        DownloadWindow d = get();            
        if (d != null) {
            d.setVisible(vis);
        }
    }

    public void setAllowVisible(boolean vis) {
        DownloadWindow d = get();
        if (d != null) {
            d.setAllowVisible(vis);
        }
    }
    
    public void disposeWindow() {
        if (dw != null) {
           dw.disposeWindow();
        } else if (Progress.usingCustomProgress()) {
            ProgressListener p = Progress.getCustomProgress();
            if (p != null) {
                p.validating(null, null, 1, 1, 100);
            }
        }
        disposed = true;
    }

    private boolean disposed = false;

    public void showLoadingProgressScreen() {
        if (dw != null) {
            dw.showLoadingProgressScreen();
        } else {
        }
    }

    //Assume that title is name of property in the resource bundle
    public void showLaunchingApplication(final String title) {
        final DownloadWindowHelper h = this;
        new Thread(new Runnable() {
            public void run() {
                try {
                    Thread.sleep(1000);
                } catch(Exception e) {}
                if (!h.disposed) {
                    DownloadWindow dw = h.get();
                    if (dw != null) {
                        dw.showLaunchingApplication(ResourceManager.getString(title));
                    }
                }
            }
        }).start();
    }
}
