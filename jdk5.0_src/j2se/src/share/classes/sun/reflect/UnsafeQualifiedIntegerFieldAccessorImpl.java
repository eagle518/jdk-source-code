/*
 * @(#)UnsafeQualifiedIntegerFieldAccessorImpl.java	1.1 04/05/10
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Field;

class UnsafeQualifiedIntegerFieldAccessorImpl
    extends UnsafeQualifiedFieldAccessorImpl
{
    UnsafeQualifiedIntegerFieldAccessorImpl(Field field, boolean isReadOnly) {
        super(field, isReadOnly);
    }

    public Object get(Object obj) throws IllegalArgumentException {
        return new Integer(getInt(obj));
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
        ensureObj(obj);
        return unsafe.getIntVolatile(obj, fieldOffset);
    }

    public long getLong(Object obj) throws IllegalArgumentException {
        return getInt(obj);
    }

    public float getFloat(Object obj) throws IllegalArgumentException {
        return getInt(obj);
    }

    public double getDouble(Object obj) throws IllegalArgumentException {
        return getInt(obj);
    }

    public void set(Object obj, Object value)
        throws IllegalArgumentException, IllegalAccessException
    {
        ensureObj(obj);
        if (isReadOnly) {
            throw new IllegalAccessException("Field is final");
        }
        if (value == null) {
            throw new IllegalArgumentException();
        }
        if (value instanceof Byte) {
            unsafe.putIntVolatile(obj, fieldOffset, ((Byte) value).byteValue());
            return;
        }
        if (value instanceof Short) {
            unsafe.putIntVolatile(obj, fieldOffset, ((Short) value).shortValue());
            return;
        }
        if (value instanceof Character) {
            unsafe.putIntVolatile(obj, fieldOffset, ((Character) value).charValue());
            return;
        }
        if (value instanceof Integer) {
            unsafe.putIntVolatile(obj, fieldOffset, ((Integer) value).intValue());
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
        setInt(obj, b);
    }

    public void setChar(Object obj, char c)
        throws IllegalArgumentException, IllegalAccessException
    {
        setInt(obj, c);
    }

    public void setShort(Object obj, short s)
        throws IllegalArgumentException, IllegalAccessException
    {
        setInt(obj, s);
    }

    public void setInt(Object obj, int i)
        throws IllegalArgumentException, IllegalAccessException
    {
        ensureObj(obj);
        if (isReadOnly) {
            throw new IllegalAccessException("Field is final");
        }
        unsafe.putIntVolatile(obj, fieldOffset, i);
    }

    public void setLong(Object obj, long l)
        throws IllegalArgumentException, IllegalAccessException
    {
        throw new IllegalArgumentException();
    }

    public void setFloat(Object obj, float f)
        throws IllegalArgumentException, IllegalAccessException
    {
        throw new IllegalArgumentException();
    }

    public void setDouble(Object obj, double d)
        throws IllegalArgumentException, IllegalAccessException
    {
        throw new IllegalArgumentException();
    }
}
