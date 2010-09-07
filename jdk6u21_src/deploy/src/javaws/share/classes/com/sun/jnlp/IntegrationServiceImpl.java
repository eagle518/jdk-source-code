/*
 * @(#)
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import com.sun.deploy.association.Association;
import com.sun.deploy.association.AssociationNotRegisteredException;
import com.sun.deploy.association.RegisterFailedException;
import com.sun.deploy.cache.AssociationDesc;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.javaws.LocalInstallHandler;
import com.sun.javaws.LocalInstallHandlerFactory;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.ShortcutDesc;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import javax.jnlp.IntegrationService;

public class IntegrationServiceImpl implements IntegrationService {

    private JNLPClassLoaderIf jnlpClassLoader;

    public IntegrationServiceImpl(JNLPClassLoaderIf loader) {
        jnlpClassLoader = loader;
    }

    public boolean requestShortcut(boolean desktop, boolean menu,
                                   String submenu) {

        // Modify the shortcut hints of the LaunchDesc.
        final LaunchDesc ld = jnlpClassLoader.getLaunchDesc();
        InformationDesc id = ld.getInformation();
        id.setShortcut(new ShortcutDesc(menu, desktop, menu, submenu));

        // Call current localinstallhandler to set the requested shortcuts.
        final LocalInstallHandler lih =
                LocalInstallHandlerFactory.newInstance();
        final LocalApplicationProperties lap =
                getLocalApplicationProperties(ld);
        Boolean success = (Boolean) AccessController.doPrivileged(
                new PrivilegedAction() {
                    public Object run() {
                        boolean success = lih.performIntegration(null, ld, lap,
                                                                 false, true,
                                                                 false);
                        return Boolean.valueOf(success);
            }
        });
        return success.booleanValue();
    }

    public boolean hasDesktopShortcut() {
        return hasShortcut(LocalInstallHandler.DESKTOP_INDEX);
    }

    public boolean hasMenuShortcut() {
        return hasShortcut(LocalInstallHandler.MENU_INDEX);
    }

    private boolean hasShortcut(int index) {
        LaunchDesc ld = jnlpClassLoader.getLaunchDesc();
        final LocalInstallHandler lih = LocalInstallHandlerFactory.newInstance();
        final LocalApplicationProperties lap = getLocalApplicationProperties(ld);
        boolean[] which = (boolean[]) AccessController.doPrivileged(
                new PrivilegedAction() {
                    public Object run() {
                        return lih.whichShortcutsExist(lap);
                    }
                });
        return which[index];
    }

    private LocalApplicationProperties getLocalApplicationProperties(final LaunchDesc ld) {
        LocalApplicationProperties lap = (LocalApplicationProperties)
                AccessController.doPrivileged(new PrivilegedAction() {

                    public Object run() {
                        URL home = ld.getCanonicalHome();
                        return Cache.getLocalApplicationProperties(home);
                    }
                });
        return lap;
    }

    public boolean removeShortcuts() {

        final LaunchDesc ld = jnlpClassLoader.getLaunchDesc();
        final LocalInstallHandler lih =
                LocalInstallHandlerFactory.newInstance();
        final LocalApplicationProperties lap =
                getLocalApplicationProperties(ld);
        Boolean success = (Boolean) AccessController.doPrivileged(
                new PrivilegedAction() {

                    public Object run() {
                        boolean success = lih.uninstallShortcuts(ld, lap);
                        return Boolean.valueOf(success);
                    }
                });
        return success.booleanValue();
    }

    public boolean requestAssociation(String mimetype, String[] extensions) {

        validateAssociationArguments(mimetype, extensions);

        // Modify association descriptor.
        final LaunchDesc ld = jnlpClassLoader.getLaunchDesc();
        InformationDesc id = ld.getInformation();
        final AssociationDesc association =
                createAssociationDesc(mimetype, extensions);
        id.setAssociation(association);

        // Call current localinstallhandler to set the requested
        // associations.
        final LocalApplicationProperties lap =
                getLocalApplicationProperties(ld);
        final LocalInstallHandler lih =
                LocalInstallHandlerFactory.newInstance();
        return ((Boolean) AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                boolean succ = lih.performIntegration(null, ld, lap, false,
                                                      false, true);
                return Boolean.valueOf(succ);
            }
        })).booleanValue();
    }

    public boolean hasAssociation(String mimetype, String[] extensions) {

        validateAssociationArguments(mimetype, extensions);

        final Association association =
                createAssociation(mimetype, extensions);
        final LocalInstallHandler lih =
                LocalInstallHandlerFactory.newInstance();
        return ((Boolean) AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                return Boolean.valueOf(lih.hasAssociation(association));
            }
        })).booleanValue();
    }

    public boolean removeAssociation(String mimetype, String[] extensions) {

        validateAssociationArguments(mimetype, extensions);

        final Association association =
                createAssociation(mimetype, extensions);
        final LocalInstallHandler lih =
                LocalInstallHandlerFactory.newInstance();
        return ((Boolean) AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                Boolean success = Boolean.TRUE;
                try {
                    lih.unregisterAssociationInternal(association);
                } catch (AssociationNotRegisteredException ex) {
                    success = Boolean.FALSE;
                } catch (RegisterFailedException ex) {
                    success = Boolean.FALSE;
                }
                return success;
            }
        })).booleanValue();
    }

    private void validateAssociationArguments(String mimeType,
                                              String[] extensions) {
        validateMimeType(mimeType);
        validateExtensions(extensions);
    }

    private void validateExtensions(String[] extensions) {
        if (extensions == null) {
            throw new IllegalArgumentException("Null extensions array not allowed");
        }
        for (int i = 0; i < extensions.length; i++) {
            String ext = extensions[i];
            if (ext == null) {
                throw new IllegalArgumentException("Null extension not allowed");
            }
            if (ext.equals("")) {
                throw new IllegalArgumentException("Empty extension not allowed");
            }
        }
    }

    /**
     * Performs basic syntactical validation of the mime type.
     *
     * @param mimeType the mime type to validate
     *
     * @throws IllegalArgumentException if mime type is invalid
     */
    private void validateMimeType(String mimeType) {
        if (mimeType == null) {
            throw new IllegalArgumentException("Null mimetype not allowed");
        }
        // NOTE: This implementation is adopted from
        // java.awt.datatransfer.MimeType.
        int slashIndex = mimeType.indexOf('/');
        int semIndex = mimeType.indexOf(';');
        String primaryType;
        String subType;
        if((slashIndex < 0) && (semIndex < 0)) {
            //    neither character is present, so treat it
            //    as an error
            throw new IllegalArgumentException("Unable to find a sub type.");
        } else if((slashIndex < 0) && (semIndex >= 0)) {
            //    we have a ';' (and therefore a parameter list),
            //    but no '/' indicating a sub type is present
            throw new IllegalArgumentException("Unable to find a sub type.");
        } else if((slashIndex >= 0) && (semIndex < 0)) {
            //    we have a primary and sub type but no parameter list
            primaryType = mimeType.substring(0,
slashIndex).trim().toLowerCase();
            subType = mimeType.substring(slashIndex + 1).trim().toLowerCase();
        } else if (slashIndex < semIndex) {
            //    we have all three items in the proper sequence
            primaryType = mimeType.substring(0, slashIndex).trim().toLowerCase();
            subType = mimeType.substring(slashIndex + 1, semIndex).trim().toLowerCase();
        } else {
            //    we have a ';' lexically before a '/' which means we have a primary type
            //    & a parameter list but no sub type
            throw new IllegalArgumentException("Unable to find a sub type.");
        }

        //    now validate the primary and sub types

        //    check to see if primary is valid
        if(! isValidToken(primaryType)) {
            throw new IllegalArgumentException("Primary type is invalid.");
        }

        //    check to see if sub is valid
        if(!isValidToken(subType)) {
            throw new IllegalArgumentException("Sub type is invalid.");
        }
    }

    /**
     * Determines whether or not a given string is a legal token.
     *
     * @throws NullPointerException if <code>s</code> is null
     */
    private boolean isValidToken(String s) {
        int len = s.length();
        if(len > 0) {
            for (int i = 0; i < len; ++i) {
                char c = s.charAt(i);
                if (!isTokenChar(c)) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    /**
     * A string that holds all the special chars.
     */
    private static final String TSPECIALS = "()<>@,;:\\\"/[]?=";

    /**
     * Determines whether or not a given character belongs to a legal token.
     */
    private static boolean isTokenChar(char c) {
        return ((c > 040) && (c < 0177)) && (TSPECIALS.indexOf(c) < 0);
    }

    /**
     * Creates an Association object out of a mimetype and extensions array.
     *
     * @param mimetype the mimetype
     * @param extensions the filename extensions
     *
     * @return the Association object
     */
    private Association createAssociation(String mimetype, String[] extensions) {
        Association association = new Association();
        association.setMimeType(mimetype);
        for (int i = 0; i < extensions.length; i++) {
            association.addFileExtension(extensions[i]);
        }
        LaunchDesc ld = jnlpClassLoader.getLaunchDesc();
        association.setName(getAssociationDescription(ld));
        return association;
    }

    private AssociationDesc createAssociationDesc(String mimetype, String[] exts) {
        StringBuilder extString = new StringBuilder();
        for (int i = 0; i < exts.length; i++) {
            extString.append(exts[i]);
            if (i < i - 1) {
                extString.append(' ');
            }
        }
        LaunchDesc ld = jnlpClassLoader.getLaunchDesc();
        // TODO: Better icon?
        AssociationDesc assoc =
                new AssociationDesc(extString.toString(), mimetype,
                                    getAssociationDescription(ld),
                                    null);
        return assoc;
    }

    private String getAssociationDescription(LaunchDesc ld) {
        // This is what is used in other places too, so we need to do the same.
        // TODO: At some point we need to consolidate that into one helper
        // method, and make a better name, the title is not really reliable,
        // contains spaces, strange characters, etc. What we probably want
        // is a hash over the codebase or some other unique identifier that
        // is not likely to change.
        return ld.getInformation().getTitle();
    }
}
