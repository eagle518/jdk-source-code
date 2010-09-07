/*
 * @(#)PrintAppletMessage.java	1.2 07/12/05 12:39:36
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** A request to print an applet. */

public class PrintAppletMessage extends AppletMessage {
    public static final int ID = PluginMessages.PRINT_APPLET;

    // The device context handle of the applet to print
    private long hdc;

    //
    private boolean isPrinterDC;

    private int x;
    private int y;
    private int width;
    private int height;

    /** For deserialization purposes */
    public PrintAppletMessage(Conversation c) {
        super(ID, c);
    }

    public PrintAppletMessage(Conversation c, int appletID, long hdc, boolean isPrinterDC,
                              int x, int y, int width, int height) {
        super(ID, c, appletID);
        this.hdc = hdc;
        this.isPrinterDC = isPrinterDC;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    public long getHDC() {
        return hdc;
    }

    public boolean getIsPrinterDC() {
        return isPrinterDC;
    }

    public int getX() {
        return x;
    }
    
    public int getY() {
        return y;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeLong(hdc);
        ser.writeBoolean(isPrinterDC);
        ser.writeInt(x);
        ser.writeInt(y);
        ser.writeInt(width);
        ser.writeInt(height);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        hdc = ser.readLong();
        isPrinterDC = ser.readBoolean();
        x = ser.readInt();
        y = ser.readInt();
        width = ser.readInt();
        height = ser.readInt();
    }
}
