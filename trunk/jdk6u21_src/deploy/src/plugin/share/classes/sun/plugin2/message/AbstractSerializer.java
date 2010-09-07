/*
 * @(#)AbstractSerializer.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.nio.ByteBuffer;

/** Provides basic, and possibly suboptimal, implementations for
    several of the methods in the {@link Serializer Serializer}
    interface. */

public abstract class AbstractSerializer implements Serializer {
    public void writeBoolean(boolean v) throws IOException {
        writeByte(v ? ((byte) 1) : ((byte) 0));
    }

    public abstract void writeByte(byte b) throws IOException;

    public void writeShort(short s) throws IOException {
        writeByte((byte) (s >>> 8));
        writeByte((byte) s);
    }

    public void writeChar(char c) throws IOException {
        writeShort((short) c);
    }

    public void writeInt(int i) throws IOException {
        writeByte((byte) (i >>> 24));
        writeByte((byte) (i >>> 16));
        writeByte((byte) (i >>> 8));
        writeByte((byte) i);
    }

    public void writeLong(long l) throws IOException {
        writeByte((byte) (l >>> 56));
        writeByte((byte) (l >>> 48));
        writeByte((byte) (l >>> 40));
        writeByte((byte) (l >>> 32));
        writeByte((byte) (l >>> 24));
        writeByte((byte) (l >>> 16));
        writeByte((byte) (l >>> 8));
        writeByte((byte) l);
    }

    public void writeFloat(float f) throws IOException {
        writeInt(Float.floatToRawIntBits(f));
    }

    public void writeDouble(double d) throws IOException {
        writeLong(Double.doubleToRawLongBits(d));
    }

    public void writeByteArray(byte[] b) throws IOException {
        writeByteArray(b, 0, (b == null ? 0 : b.length));
    }

    public void writeByteArray(byte[] b, int offset, int length) throws IOException {
        if (b == null) {
            writeBoolean(false);
            return;
        }

        writeBoolean(true);
        writeInt(length);
        for (int i = 0; i < length; i++) {
            writeByte(b[offset + i]);
        }
    }

    public void writeCharArray(char[] c) throws IOException {
        writeCharArray(c, 0, (c == null ? 0 : c.length));
    }

    public void writeCharArray(char[] c, int offset, int length) throws IOException {
        if (c == null) {
            writeBoolean(false);
            return;
        }

        writeBoolean(true);
        writeInt(length);
        for (int i = 0; i < length; i++) {
            writeChar(c[offset + i]);
        }
    }

    public void writeIntegerArray(Integer[] ii) throws IOException {
        if (ii == null || ii.length==0) {
            writeBoolean(false);
            return;
        }

        writeBoolean(true);
        writeInt(ii.length);
        for (int i = 0; i < ii.length; i++) {
            writeInt(ii[i].intValue());
        }
    }

    public void writeUTF(String s) throws IOException {
        if (s == null) {
            writeBoolean(false);
            return;
        }

        writeBoolean(true);
        int len = s.length();
        writeInt(len);
        for (int i = 0; i < len; i++) {
            writeChar(s.charAt(i));
        }
    }

    public void writeUTFArray(String[] ss) throws IOException {
        if (ss == null) {
            writeBoolean(false);
            return;
        }

        writeBoolean(true);
        writeInt(ss.length);
        for (int i = 0; i < ss.length; i++) {
            writeUTF(ss[i]);
        }
    }

    public void writeConversation(Conversation conversation) throws IOException {
        if (conversation == null) {
            writeBoolean(false);
            return;
        }

        writeBoolean(true);
        conversation.writeFields(this);
    }

    public abstract void flush() throws IOException;

    public boolean readBoolean() throws IOException {
        return ((readByte() != 0) ? true : false);
    }

    public abstract byte readByte() throws IOException;

    public short readShort() throws IOException {
        short s = (short) (  ((readByte() & 0xFF) << 8)
                           | (readByte() & 0xFF));
        return s;
    }

    public char readChar() throws IOException {
        return (char) readShort();
    }

    public int readInt() throws IOException {
        int i = readByte() & 0xFF;
        i = (i << 8) | (readByte() & 0xFF);
        i = (i << 8) | (readByte() & 0xFF);
        i = (i << 8) | (readByte() & 0xFF);
        return i;
    }

    public long readLong() throws IOException {
        long l = readByte() & 0xFF;
        l = (l << 8) | (readByte() & 0xFF);
        l = (l << 8) | (readByte() & 0xFF);
        l = (l << 8) | (readByte() & 0xFF);
        l = (l << 8) | (readByte() & 0xFF);
        l = (l << 8) | (readByte() & 0xFF);
        l = (l << 8) | (readByte() & 0xFF);
        l = (l << 8) | (readByte() & 0xFF);
        return l;
    }

    public float readFloat() throws IOException {
        return Float.intBitsToFloat(readInt());
    }

    public double readDouble() throws IOException {
        return Double.longBitsToDouble(readLong());
    }

    public ByteBuffer readByteBuffer() throws IOException {
        if (!readBoolean())
            return null;
        int length = readInt();
        ByteBuffer res = ByteBuffer.allocateDirect(length);
        for (int i = 0; i < length; i++) {
            res.put(readByte());
        }
        res.rewind();
        return res;
    }

    public byte[] readByteArray() throws IOException {
        if (!readBoolean())
            return null;

        int length = readInt();
        byte[] res = new byte[length];
        int offset = 0;
        for (int i = 0; i < length; i++) {
            res[i] = readByte();
        }
        return res;
    }

    public char[] readCharArray() throws IOException {
        if (!readBoolean())
            return null;

        int length = readInt();
        char[] res = new char[length];
        for (int i = 0; i < length; i++) {
            res[i] = readChar();
        }
        return res;
    }
    
    public Integer[] readIntegerArray()   throws IOException
    {
        if (!readBoolean())
            return null;

        int length = readInt();
        Integer[] res = new Integer[length];
        for (int i = 0; i < length; i++) {
            res[i] = new Integer(readInt());
        }
        return res;
    }

    public String readUTF() throws IOException {
        if (!readBoolean())
            return null;

        int len = readInt();
        StringBuffer buf = new StringBuffer(len);
        for (int i = 0; i < len; i++) {
            buf.append(readChar());
        }
        return buf.toString();
    }

    public String[] readUTFArray() throws IOException {
        if (!readBoolean())
            return null;

        int num = readInt();
        String[] res = new String[num];
        for (int i = 0; i < num; i++) {
            res[i] = readUTF();
        }
        return res;
    }

    public Conversation readConversation() throws IOException {
        if (!readBoolean()) {
            return null;
        }

        Conversation conversation = new Conversation();
        conversation.readFields(this);
        return conversation;
    }
}
