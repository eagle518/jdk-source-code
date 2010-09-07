/*
 * @(#)ArgumentHelper.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.liveconnect;

import java.io.*;
import sun.plugin2.message.*;

/** This class helps in transport of objects and values between
    processes. It is used for both JavaScript-to-Java calls and
    Java-to-JavaScript calls. <P>

    The following kinds of objects can be sent over the wire:

    <UL>
    <LI> Null references
    <LI> Any primitive type boxing object (Boolean, Byte, Integer,
         etc.)
    <LI> Instances of java.lang.String
    <LI> Instances of sun.plugin2.liveconnect.BrowserSideObject
         representing JavaScript objects on the browser side
    <LI> Instances of RemoteJavaObject corresponding to Java objects
         exposed to the web browser
    </UL>
*/

public final class ArgumentHelper {
    private ArgumentHelper() {}

    /** Indicates whether a given object is a box for a primitive
        value or a String, in which case it requires no conversion
        when travelling between JVMs. */
    public static boolean isPrimitiveOrString(Object obj) {
        if (obj instanceof String)
            return true;
        return isPrimitive(obj);
    }

    /** Indicates whether a given object is a box for a primitive
        value, in which case it requires no conversion when travelling
        between JVMs. */
    public static boolean isPrimitive(Object obj) {
        if (obj instanceof Boolean ||
            obj instanceof Byte ||
            obj instanceof Character ||
            obj instanceof Short ||
            obj instanceof Integer ||
            obj instanceof Long ||
            obj instanceof Float ||
            obj instanceof Double)
            return true;
        return false;
    }

    /** Validates an outgoing argument array, ensuring that all of the
        elements contained within obey the serialization restrictions
        described in the class Javadoc above. This should be called
        before calling {@link #write write} and in fact before
        starting the serialization process to avoid potentially
        corrupting the data stream by throwing an exception during
        serialization. */
    public static void validate(Object[] argArray) throws IllegalArgumentException {
        if (argArray != null) {
            for (int i = 0; i < argArray.length; i++) {
                validate(argArray[i]);
            }
        }
    }

    // Tags for objects we write
    private static final int NULL_TAG                = 0;
    private static final int STRING_TAG              = 1;
    private static final int BOOLEAN_TAG             = 2;
    private static final int BYTE_TAG                = 3;
    private static final int CHARACTER_TAG           = 4;
    private static final int SHORT_TAG               = 5;
    private static final int INTEGER_TAG             = 6;
    private static final int LONG_TAG                = 7;
    private static final int FLOAT_TAG               = 8;
    private static final int DOUBLE_TAG              = 9;
    private static final int BROWSER_SIDE_OBJECT_TAG = 10;
    private static final int REMOTE_JAVA_OBJECT_TAG  = 11;

    /** Serializes one individual object to the stream according to
        the serialization rules of this class. */
    public static void writeObject(Serializer ser, Object obj) throws IOException {
        if (obj == null) {
            ser.writeByte((byte) NULL_TAG);
            return;
        }

        if (obj instanceof String) {
            ser.writeByte((byte) STRING_TAG);
            ser.writeUTF((String) obj);
        } else if (obj instanceof Boolean) {
            ser.writeByte((byte) BOOLEAN_TAG);
            ser.writeBoolean(((Boolean) obj).booleanValue());
        } else if (obj instanceof Byte) {
            ser.writeByte((byte) BYTE_TAG);
            ser.writeByte(((Byte) obj).byteValue());
        } else if (obj instanceof Character) {
            ser.writeByte((byte) CHARACTER_TAG);
            ser.writeChar(((Character) obj).charValue());
        } else if (obj instanceof Short) {
            ser.writeByte((byte) SHORT_TAG);
            ser.writeShort(((Short) obj).shortValue());
        } else if (obj instanceof Integer) {
            ser.writeByte((byte) INTEGER_TAG);
            ser.writeInt(((Integer) obj).intValue());
        } else if (obj instanceof Long) {
            ser.writeByte((byte) LONG_TAG);
            ser.writeLong(((Long) obj).longValue());
        } else if (obj instanceof Float) {
            ser.writeByte((byte) FLOAT_TAG);
            ser.writeFloat(((Float) obj).floatValue());
        } else if (obj instanceof Double) {
            ser.writeByte((byte) DOUBLE_TAG);
            ser.writeDouble(((Double) obj).doubleValue());
        } else if (obj instanceof BrowserSideObject) {
            ser.writeByte((byte) BROWSER_SIDE_OBJECT_TAG);
            ser.writeLong(((BrowserSideObject) obj).getNativeObjectReference());
        } else if (obj instanceof RemoteJavaObject) {
            ser.writeByte((byte) REMOTE_JAVA_OBJECT_TAG);
            RemoteJavaObject.write(ser, (RemoteJavaObject) obj);
        } else {
            throw new RuntimeException("Can't serialize objects of type " + obj.getClass().getName());
        }
    }

    /** Deserializes one individual object from the stream according
        to the serialization rules of this class. */
    public static Object readObject(Serializer ser) throws IOException {
        int tag = ser.readByte() & 0xFF;
        switch (tag) {
            case NULL_TAG:                return null;
            case STRING_TAG:              return ser.readUTF();
            case BOOLEAN_TAG:             return (ser.readBoolean() ? Boolean.TRUE : Boolean.FALSE);
            case BYTE_TAG:                return new Byte(ser.readByte());
            case CHARACTER_TAG:           return new Character(ser.readChar());
            case SHORT_TAG:               return new Short(ser.readShort());
            case INTEGER_TAG:             return new Integer(ser.readInt());
            case LONG_TAG:                return new Long(ser.readLong());
            case FLOAT_TAG:               return new Float(ser.readFloat());
            case DOUBLE_TAG:              return new Double(ser.readDouble());
            case BROWSER_SIDE_OBJECT_TAG: return new BrowserSideObject(ser.readLong());
            case REMOTE_JAVA_OBJECT_TAG:  return RemoteJavaObject.read(ser);
            default: throw new RuntimeException("Unexpected object tag " + tag);
        }
    }

    /** Serializes an outgoing argument array according to the
        serialization rules of this class. */
    public static void writeObjectArray(Serializer ser, Object[] argumentArray) throws IOException {
        if (argumentArray == null) {
            ser.writeBoolean(false);
            return;
        }

        ser.writeBoolean(true);
        ser.writeInt(argumentArray.length);
        for (int i = 0; i < argumentArray.length; i++) {
            writeObject(ser, argumentArray[i]);
        }
    }

    /** Deserializes an incoming argument array according to the
        serialization rules of this class. */
    public static Object[] readObjectArray(Serializer ser) throws IOException {
        if (!ser.readBoolean())
            return null;

        int length = ser.readInt();
        Object[] res = new Object[length];
        for (int i = 0; i < length; i++) {
            res[i] = readObject(ser);
        }
        return res;
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private static void validate(Object obj) throws IllegalArgumentException {
        if (obj == null)
            return;
        if (isPrimitiveOrString(obj))
            return;
        if (obj instanceof BrowserSideObject)
            return;
        if (obj instanceof RemoteJavaObject)
            return;
        throw new IllegalArgumentException("Can't pass instances of class " +
                                           obj.getClass().getName() + " between JVMs");
    }
}
