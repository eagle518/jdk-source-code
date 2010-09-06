/*
 * @(#)ValueHelper.java	1.16 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.org.omg.CORBA.portable;

import org.omg.CORBA.TypeCode;
import org.omg.CORBA.portable.BoxedValueHelper;

/**
 * An interface that is implemented by valuetype helper classes.
 * This interface appeared in CORBA 2.3 drafts but was removed from
 * the published CORBA 2.3 specification.
 * <P>
 * @deprecated Deprecated by CORBA 2.3.
 */
@Deprecated
public interface ValueHelper extends BoxedValueHelper {
    Class get_class();
    String[] get_truncatable_base_ids();
    TypeCode get_type();
}

