/*
 * @(#)IntegrationService.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

/**
 * Provides desktop integration for JNLP applications. In particular, this
 * service provides API for programmatically request, query and remove
 * shortcuts and request, query and remove mimetype associations.
 *
 * @since 6.0.18
 */
public interface IntegrationService {

    /**
     * Requests that a shortcut is created for this application. It can be specified
     * to appear in a system menu, on the desktop or both. For the menu it is possible
     * to specify a submenu path where the shortcut should be placed.
     *
     * @param desktop {@code true} if the shortcut should appear on the desktop
     * @param menu    {@code true} if the shortcut should appear in a system menu
     * @param submenu the path to the submenu where to place the shortcut (only
     *                meaningful if menu is true). <br>The path is relative to the 
     *                (platform dependant) default menu path.
     *
     * @return {@code true} if all shortcuts have been created successfully,
     *         {@code false} otherwise
     */
    boolean requestShortcut(boolean desktop, boolean menu, String submenu);

    /**
     * Checks if the application has a desktop shortcut.
     *
     * @return {@code true} if the application has a desktop shortcut,
     *         {@code false} otherwise
     */
    boolean hasDesktopShortcut();

    /**
     * Checks if the application has a menu shortcut.
     *
     * @return {@code true} if the application has a menu shortcut,
     *         {@code false} otherwise
     */
    boolean hasMenuShortcut();

    /**
     * Requests that all shortcuts for the application are removed.
     *
     * @return {@code true} if the shortcuts have been removed successfully
     *         {@code false} otherwise
     */
    boolean removeShortcuts();

    /**
     * Requests that this application is registered as the handler for the
     * specified mime type and (optionally) filename extensions.
     *
     * @param mimetype the mimetype to register this application as handler for
     * @param extensions the filename extensions to register this application
     *        as handler for or {@code null}
     *
     * @return {@code true} if the app has been associated successfully,
     *         {@code false} otherwise
     *
     * @throws IllegalArgumentException if either {@code mimetype} or
     *         {@code extensions} is {@code null}, or if {@code mimetype} has
     *         illegal mime type syntax, or if {@code extensions} contains
     *         {@code null} or empty elements
     */
    boolean requestAssociation(String mimetype, String [] extensions);

    /**
     * Checks if this application is associated with the specified mimetype
     * and filename extensions.
     *
     * @param mimetype the mimetype to check association for
     * @param extensions the filename extensions to check association for
     *
     * @return {@code true} if this application is associated with the
     *         specified mimetype and filename extensions, {@code false}
     *         otherwise
     *
     * @throws IllegalArgumentException if either {@code mimetype} or
     *         {@code extensions} is {@code null}, or if {@code mimetype} has
     *         illegal mime type syntax, or if {@code extensions} contains
     *         {@code null} or empty elements
     */
    boolean hasAssociation(String mimetype, String [] extensions);

    /**
     * Remove an association for this application as handler for the specified
     * mimetype.
     *
     * @param mimetype the mimetype to remove association for
     * @param extensions the filename extensions to remove association for
     *
     * @return {@code true} if the association has been removed successfully,
     *         {@code false} otherwise
     *
     * @throws IllegalArgumentException if either {@code mimetype} or
     *         {@code extensions} is {@code null}, or if {@code mimetype} has
     *         illegal mime type syntax, or if {@code extensions} contains
     *         {@code null} or empty elements
     */
    boolean removeAssociation(String mimetype, String [] extensions);

}
