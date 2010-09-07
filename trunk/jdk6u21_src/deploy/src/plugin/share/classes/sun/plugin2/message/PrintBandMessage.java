/*
 * @(#)PrintBandMessage.java	1.4 10/03/24 12:01:47
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.nio.ByteBuffer;
import sun.plugin2.main.client.PrintBandDescriptor;


/** A request to print an applet. */

public class PrintBandMessage extends AppletMessage {
    public static final int ID = PluginMessages.PRINT_BAND;

    // The device context handle of the applet to print
    private long hdc;

    //
    private byte[] data;     // DIB data as byte array - For writing by VM side 
    private ByteBuffer buf;  // DIB data as ByteBuffer - For reading by browser
    private int offset;      // Image offset
    private int sx;          // Source Coordinates
    private int sy;  
    private int swidth;
    private int sheight;
    private int dx;          // Destination Coordinates
    private int dy;
    private int dwidth;
    private int dheight;

    /** For deserialization purposes */
    public PrintBandMessage(Conversation c) {
        super(ID, c);
    }

    public PrintBandMessage(Conversation c, int appletID, long hdc, byte[] data, int offset,
                            int sx, int sy, int swidth, int sheight,
                            int dx, int dy, int dwidth, int dheight) {
        super(ID, c, appletID);
        this.hdc = hdc;
        this.data = data;
        this.offset = offset;
        this.sx = sx;
        this.sy = sy;
        this.swidth = swidth;
        this.sheight = sheight;
        this.dx = dx;
        this.dy = dy;
        this.dwidth = dwidth;
        this.dheight = dheight;
    }

    public long getHDC() {
        return hdc;
    }

    public byte[] getData() {
        return data;
    }

    // To be used on the browser side
    public ByteBuffer getDataAsByteBuffer() {
        return buf;
    }

    public int getOffset() {
        return offset;
    }

    public int getSrcX() {
        return sx;
    }

    public int getSrcY() {
        return sy;
    }

    public int getSrcWidth() {
        return swidth;
    }

    public int getSrcHeight() {
        return sheight;
    }

    public int getDestX() {
        return dx;
    }

    public int getDestY() {
        return dy;
    }

    public int getDestWidth() {
        return dwidth;
    }

    public int getDestHeight() {
        return dheight;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeLong(hdc);
        ser.writeByteArray(data);
        ser.writeInt(offset);
        ser.writeInt(sx);
        ser.writeInt(sy);
        ser.writeInt(swidth);
        ser.writeInt(sheight);
        ser.writeInt(dx);
        ser.writeInt(dy);
        ser.writeInt(dwidth);
        ser.writeInt(dheight);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        hdc = ser.readLong();
        buf = ser.readByteBuffer();
        offset = ser.readInt();
        sx = ser.readInt();
        sy = ser.readInt();
        swidth = ser.readInt();
        sheight = ser.readInt();
        dx = ser.readInt();
        dy = ser.readInt();
        dwidth = ser.readInt();
        dheight = ser.readInt();
    }
}
