/*
 * @(#)Modifier.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench;

public interface Modifier {
    public Modifier.Iterator getIterator(TestEnvironment env);

    public void modifyTest(TestEnvironment env, Object val);

    public void restoreTest(TestEnvironment env, Object val);

    public String getTreeName();

    public String getAbbreviatedModifierDescription(Object val);

    public String getModifierValueName(Object val);

    public static interface Iterator {
	public boolean hasNext();

	public Object next();
    }

    public static interface Filter {
	public boolean isCompatible(Object val);
    }
}
