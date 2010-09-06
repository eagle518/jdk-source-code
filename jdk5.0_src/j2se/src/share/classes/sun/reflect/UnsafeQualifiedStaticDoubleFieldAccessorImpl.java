/*
 * @(#)UnsafeQualifiedStaticDoubleFieldAccessorImpl.java	1.1 04/05/10
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Field;

class UnsafeQualifiedStaticDoubleFieldAccessorImpl
    extends UnsafeQualifiedStaticFieldAccessorImpl
{
    UnsafeQualifiedStaticDoubleFieldAccessorImpl(Field field, boolean isReadOnly) {
        super(field, isReadOnly);
    }

    public Object get(Object obj) throws IllegalArgumentException {
        return new Double(getDouble(obj));
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
        throw new IllegalArgumentException();
    }

    public double getDouble(Object obj) throws IllegalArgumentException {
        return unsafe.getDoubleVolatile(base, fieldOffset);
    }

    public void set(Object obj, Object value)
        throws IllegalArgumentException, IllegalAccessException
    {
        if (isReadOnly) {
            throw new IllegalAccessException("Field is final");
        }
        if (value == null) {
            throw new IllegalArgumentException();
        }
        if (value instanceof Byte) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Byte) value).byteValue());
            return;
        }
        if (value instanceof Short) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Short) value).shortValue());
            return;
        }
        if (value instanceof Character) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Character) value).charValue());
            return;
        }
        if (value instanceof Integer) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Integer) value).intValue());
            return;
        }
        if (value instanceof Long) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Long) value).longValue());
            return;
        }
        if (value instanceof Float) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Float) value).floatValue());
            return;
        }
        if (value instanceof Double) {
            unsafe.putDoubleVolatile(base, fieldOffset, ((Double) value).doubleValue());
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
        setDouble(obj, b);
    }

    public void setChar(Object obj, char c)
        throws IllegalArgumentException, IllegalAccessException
    {
        setDouble(obj, c);
    }

    public void setShort(Object obj, short s)
        throws IllegalArgumentException, IllegalAccessException
    {
        setDouble(obj, s);
    }

    public void setInt(Object obj, int i)
        throws IllegalArgumentException, IllegalAccessException
    {
        setDouble(obj, i);
    }

    public void setLong(Object obj, long l)
        throws IllegalArgumentException, IllegalAccessException
    {
        setDouble(obj, l);
    }

    public void setFloat(Object obj, float f)
        throws IllegalArgumentException, IllegalAccessException
    {
        setDouble(obj, f);
    }

    public void setDouble(Object obj, double d)
        throws IllegalArgumentException, IllegalAccessException
    {
        if (isReadOnly) {
            throw new IllegalAccessException("Field is final");
        }
        unsafe.putDoubleVolatile(base, fieldOffset, d);
    }
}
