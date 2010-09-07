/*
 * @(#)Serializer.java	1.4 07/11/20
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.nio.ByteBuffer;

/** Small abstraction layer to admit multiple kinds of writing
    Messages on the wire. */
public interface Serializer {
    public void writeBoolean(boolean v) throws IOException;
    public void writeByte(byte b)     throws IOException;
    public void writeShort(short s)   throws IOException;
    public void writeChar(char c)     throws IOException;
    public void writeInt(int i)       throws IOException;
    public void writeLong(long l)     throws IOException;
    public void writeFloat(float f)   throws IOException;
    public void writeDouble(double d) throws IOException;
    public void writeByteArray(byte[] b) throws IOException;
    public void writeByteArray(byte[] b, int offset, int length) throws IOException;
    public void writeCharArray(char[] c) throws IOException;
    public void writeCharArray(char[] c, int offset, int length) throws IOException;
    public void writeIntegerArray(Integer[] ii) throws IOException;
    public void writeUTF(String s)    throws IOException;
    public void writeUTFArray(String[] ss) throws IOException;

    // Force written data to be seen by the other side
    public void flush() throws IOException;

    public boolean    readBoolean()    throws IOException;
    public byte       readByte()       throws IOException;
    public short      readShort()      throws IOException;
    public char       readChar()       throws IOException;
    public int        readInt()        throws IOException;
    public long       readLong()       throws IOException;
    public float      readFloat()      throws IOException;
    public double     readDouble()     throws IOException;
    public ByteBuffer readByteBuffer() throws IOException;
    public byte[]     readByteArray()  throws IOException;
    public char[]     readCharArray()  throws IOException;
    public Integer[]  readIntegerArray() throws IOException;
    public String     readUTF()        throws IOException;
    public String[] readUTFArray()  throws IOException;

    public Conversation readConversation() throws IOException;
    public void         writeConversation(Conversation conversation) throws IOException;
}
