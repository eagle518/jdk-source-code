/*
 * @(#)ModalExclude.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

/**
 * Interface for identifying a component that will be excluded during
 * modal operations.  Implementing this interface will ensure that the
 * component willl still receive it's events.
 *
 * @version 1.2 12/19/03
 * @since 1.5
 * @author Joshua Outwater
 */
public interface ModalExclude {
}
