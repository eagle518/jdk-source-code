/*
 * @(#)DOMAction.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.dom;

 
public interface DOMAction
{
    /**
     * When an object implementing interface DOMAction is passed
     * to DOMService.invokeAndWait() or DOMService.invokeLater(),
     * run method is called in the DOM access dispatch thread.
     *
     * accessor is used for the DOMAction to access the entry point of 
     * the browser's DOM, if necessary.
     *
     * @param accessor DOMAccessor
     */
    public Object run(DOMAccessor accessor);
}            
