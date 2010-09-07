/*
 * @(#)LocalInstallHandlerFactory.java	1.3 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.deploy.association.*;
import com.sun.deploy.cache.*;
import com.sun.javaws.jnl.*;

/**
 * LocalInstallHandlerFactory for Mac OS X.
 */

public class LocalInstallHandlerFactory {
    /**
     * No functional LocalInstallHandler on Mac OS X yet (FIXME)
     */
    public static LocalInstallHandler newInstance() {
        return new LocalInstallHandler() {
            public boolean isLocalInstallSupported() {
                return false;
            }

            public boolean isAssociationSupported() {
                return false;
            }

            boolean isAssociationFileExtSupported(String ext) {
                return false;
            }

            public String getAssociationOpenCommand(String jnlpLocation) {
                return null;
            }

            public String getAssociationPrintCommand(String jnlpLocation) {
                return null;
            }

            public void registerAssociationInternal(Association assoc) 
                throws AssociationAlreadyRegisteredException, RegisterFailedException {
            }

            public void unregisterAssociationInternal(Association assoc) 
                throws AssociationNotRegisteredException, RegisterFailedException {
            }

            public String getDefaultIconPath() {
                return null;
            }

            public boolean isShortcutExists(LocalApplicationProperties lap) {
                return false;
            }

            public boolean[] whichShortcutsExist(LocalApplicationProperties lap) {
                return null;
            }

            protected void createShortcuts(LaunchDesc                 ld,
                                           LocalApplicationProperties lap,
                                           boolean[]                  which) {
            }

            protected void removeShortcuts(LaunchDesc                 ld,
                                           LocalApplicationProperties lap,
                                           boolean                    desktop) {
            }

            protected boolean removePathShortcut(String path) {
                return false;
            }

            protected void registerWithInstallPanel(LaunchDesc ld, 
                                                    LocalApplicationProperties lap) {
            }

            protected void removeFromInstallPanel(LaunchDesc ld, 
                                                  LocalApplicationProperties lap) {
            }
        };
    }
}
