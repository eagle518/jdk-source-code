/*
 * @(#)BrowserType.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

/** Enumeration indicating which browser type is in use. Given the
    current deployment code base, this is unfortunately needed in
    order to make browser certificate store access work. See {@link
    sun.plugin2.main.client.ServiceDelegate ServiceDelegate}. */

public final class BrowserType {
    private BrowserType() {}

    /** Indicates a "default" browser type with no special
        functionality for certificate store access. */
    public static final int DEFAULT = 1;

    /** Indicates the Internet Explorer browser (on Windows). */
    public static final int INTERNET_EXPLORER = 2;

    /** Indicates the Mozilla family of browsers. */
    public static final int MOZILLA = 3;

    /** Indicates the Safari browser on Mac OS X (under the assumption
        that Safari on Windows might work slightly differently). */
    public static final int SAFARI_MACOSX = 4;
}
