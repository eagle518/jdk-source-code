/*
 * @(#)UnsafeFloatFieldAccessorImpl.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Field;

class UnsafeFloatFieldAccessorImpl extends UnsafeFieldAccessorImpl {
    UnsafeFloatFieldAccessorImpl(Field field) {
        super(field);
    }

    public Object get(Object obj) throws IllegalArgumentException {
        return new Float(getFloat(obj));
    }

    public boolean getBoolean(Object obj) throws IllegalArgumentException {
        throw new IllegalArgumentException();
    }

    public byte getByte(Object obj) throws IllegalArgumentException {
        throw new IllegalArgumentException();
    }

    public char getChar(Object obj) throws IllegalArgumentException {
        throw new IllegalArgumentException();
    }

    public short getShort(Object obj) throws IllegalArgumentException {
        throw new IllegalArgumentException();
    }

    public int getInt(Object obj) throws IllegalArgumentException {
        throw new IllegalArgumentException();
    }

    public long getLong(Object obj) throws IllegalArgumentException {
        throw new IllegalArgumentException();
    }

    public float getFloat(Object obj) throws IllegalArgumentException {
        ensureObj(obj);
        return unsafe.getFloat(obj, fieldOffset);
    }

    public double getDouble(Object obj) throws IllegalArgumentException {
        return getFloat(obj);
    }

    public void set(Object obj, Object value)
        throws IllegalArgumentException, IllegalAccessException
    {
        ensureObj(obj);
        if (isFinal) {
            throw new IllegalAccessException("Field is final");
        }
        if (value == null) {
            throw new IllegalArgumentException();
        }
        if (value instanceof Byte) {
            unsafe.putFloat(obj, fieldOffset, ((Byte) value).byteValue());
            return;
        }
        if (value instanceof Short) {
            unsafe.putFloat(obj, fieldOffset, ((Short) value).shortValue());
            return;
        }
        if (value instanceof Character) {
            unsafe.putFloat(obj, fieldOffset, ((Character) value).charValue());
            return;
        }
        if (value instanceof Integer) {
            unsafe.putFloat(obj, fieldOffset, ((Integer) value).intValue());
            return;
        }
        if (value instanceof Long) {
            unsafe.putFloat(obj, fieldOffset, ((Long) value).longValue());
            return;
        }
        if (value instanceof Float) {
            unsafe.putFloat(obj, fieldOffset, ((Float) value).floatValue());
            return;
        }
        throw new IllegalArgumentException();
    }

    public void setBoolean(Object obj, boolean z)
        throws IllegalArgumentException, IllegalAccessException
    {
        throw new IllegalArgumentException();
    }

    public void setByte(Object obj, byte b)
        throws IllegalArgumentException, IllegalAccessException
    {
        setFloat(obj, b);
    }

    public void setChar(Object obj, char c)
        throws IllegalArgumentException, IllegalAccessException
    {
        setFloat(obj, c);
    }

    public void setShort(Object obj, short s)
        throws IllegalArgumentException, IllegalAccessException
    {
        setFloat(obj, s);
    }

    public void setInt(Object obj, int i)
        throws IllegalArgumentException, IllegalAccessException
    {
        setFloat(obj, i);
    }

    public void setLong(Object obj, long l)
        throws IllegalArgumentException, IllegalAccessException
    {
        setFloat(obj, l);
    }

    public void setFloat(Object obj, float f)
        throws IllegalArgumentException, IllegalAccessException
    {
        ensureObj(obj);
        if (isFinal) {
            throw new IllegalAccessException("Field is final");
        }
        unsafe.putFloat(obj, fieldOffset, f);
    }

    public void setDouble(Object obj, double d)
        throws IllegalArgumentException, IllegalAccessException
    {
        throw new IllegalArgumentException();
    }
}
