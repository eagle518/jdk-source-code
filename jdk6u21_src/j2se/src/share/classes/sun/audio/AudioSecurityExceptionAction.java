/*
 * @(#)AudioSecurityExceptionAction.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

public interface AudioSecurityExceptionAction {
    Object run() throws Exception;
}
