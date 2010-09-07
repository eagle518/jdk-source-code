/*
 * @(#)GeneralEntity.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.xml;

import java.lang.Object;
import java.lang.NullPointerException;
import java.lang.String;


/**
 This class represents an XML general entity.
 <br />
 A true XML parser allows an entity to be defined within a Document Type
 Definition (DTD, which may be internal or external to the XML document being
 parsed.  For purposes of simplicity and smaller download sizes, JaWS uses a
 simplified XML parser, which ignores the contents of any DTD, and can basically
 only parse a JNLP document.
 <br />
 This class is used to represent the few predefined general entities required
 from any XML parser.
 */
public class GeneralEntity extends Object
{
    /**
     Gets the name assigned to <code>this</code> general entity.

     @return The name assigned to <code>this</code> general entity.
     */
    public String getName()
    {
        return (_name);
    }

    /**
     Gets the replacement value assigned to <code>this</code> general entity.

     @return The replacement value assigned to <code>this</code> general entity.
     */
    public String getValue()
    {
        return (_value);
    }

    /**
     Checks if the given object is equal to <code>this</code> general entity.
     <br />
     An object is considered equal to a general entity if it is the same object
     as <code>this</code> general entity (i.e. identity).  An object is also
     considered equal to a general entity if both objects are general entities,
     and they have the same name and value.  Finally, a string is considered
     equal to a general entity if it represents the same name or value of the
     general entity.

     @return <code>true</code> if the object represents <code>this</code> general
     entity; <code>false</code> otherwise.
     */
    public boolean equals(Object obj)
    {
        boolean result = false;

        if (this == obj)
        {
            // these are the same objects (identitiy)
            result = true;
        }
        else if (obj instanceof GeneralEntity)
        {
            // if these are both general entities, then they must have the
            // same name and value
            GeneralEntity lhs = (GeneralEntity) obj;

            result = (_name.equals(lhs._name) && _value.equals(lhs._value));
        }
        else if (obj instanceof String)
        {
            // if the left hand side is a string, then consider it equal if
            // is the same as either the name or the value.
            String lhs = (String) obj;

            result = (_name.equals(lhs) || _value.equals(lhs));
        }
        // no else required; can't be equal

        return (result);
    }

    /**
     Gets a hash code value for the object.  This method returns the hash value
     for the name assigned to <code>this</code> general entity.

     @return A hash code value for the object.
     */
    public int hashCode()
    {
        return (_name.hashCode());
    }

    /**
     Construct a general entity with the given name and replacement value.

     @param name   the name for the general entity.
     @param value  the replacement value for the general entity.
     */
    public GeneralEntity(String name, String value)
    {
        if (name == null)
        {
            throw (new NullPointerException(BAD_NAME));
        }
        else if (value == null)
        {
            throw (new NullPointerException(BAD_VALUE));
        }
        // no else required;

        _name  = name;
        _value = value;
    }

    private static final String BAD_NAME  = "A general entity cannot must have a name!";
    private static final String BAD_VALUE = "A general entity cannot must have a substitution value!";

    private final String _name;
    private final String _value;
}
