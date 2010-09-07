/*
 * @(#)Applet2IllegalArgumentException.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

/**
 * IllegalArgumentException implementation supporting localization.
 *
 * @version 	10/03/24
 * @author 	Arthur van Hoff
 */
public 
class Applet2IllegalArgumentException extends IllegalArgumentException {
    private String key = null;

    public Applet2IllegalArgumentException(String key) {
        super(key);
        this.key = key;

    }

    public String getLocalizedMessage() {
        return amh.getMessage(key);
    }

    private static Applet2MessageHandler amh = new Applet2MessageHandler("appletillegalargumentexception");
}
