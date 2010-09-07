/*
 * @(#)XAtomList.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.util.*;

/**
 * Helper class to ease the work with the lists of atoms.
 */
class XAtomList {
    Set atoms = new HashSet();

    /**
     * Creates empty list.
     */
    public XAtomList() {
    }

    /**
     * Creates instance of XAtomList and initializes it with
     * the contents pointer by <code>data</code>.
     * Uses default display to initialize atoms.
     */
    public XAtomList(long data, int count) {
        init(data, count);
    }
    private void init(long data, int count) {
        for (int i = 0; i < count; i++) {
            add(new XAtom(XToolkit.getDisplay(), XAtom.getAtom(data+count*XAtom.getAtomSize())));
        }
    }

    /**
     * Creates instance of XAtomList and initializes it with
     * the arrays of atoms. Array can contain null atoms.
     */
    public XAtomList(XAtom[] atoms) {
        init(atoms);
    }
    private void init(XAtom[] atoms) {
        for (int i = 0; i < atoms.length; i++) {
            add(atoms[i]);
        }
    }

    /**
     * Returns contents of the list as array of atoms.
     */
    public XAtom[] getAtoms() {
        XAtom[] res = new XAtom[size()];
        Iterator iter = atoms.iterator();
        int i = 0;
        while (iter.hasNext()) {
            res[i++] = (XAtom)iter.next();
        }
        return res;
    }

    /**
     * Returns contents of the list as pointer to native data
     * The size of the native data is size of the list multiplied by
     * size of the Atom type on the platform. Caller is responsible for
     * freeing the data by Unsafe.freeMemory when it is no longer needed.
     */
    public long getAtomsData() {
        return XAtom.toData(getAtoms());
    }
    
    /**
     * Returns true if this list contains the atom <code>atom</code>
     */
    public boolean contains(XAtom atom) {
        return atoms.contains(atom);
    }

    /**
     * Add atom to the list. Does nothing if list already contains this atom.
     */
    public void add(XAtom atom) {
        atoms.add(atom);
    }

    /**
     * Removes atom from the list. Does nothing if arrays doesn't conaint this atom.
     */
    public void remove(XAtom atom) {
        atoms.remove(atom);
    }


    /**
     * Returns size of the list
     */
    public int size() {
        return atoms.size();
    }

    /**
     * Returns a subset of a list which is intersection of this set and set build by mapping <code>mask</code> in
     * <code>mapping</code>.
     */
    public XAtomList subset(int mask, Map mapping) {
        XAtomList res = new XAtomList();
        Iterator iter = mapping.keySet().iterator();
        while (iter.hasNext()) {
            Integer bits = (Integer)iter.next();
            if ( (mask & bits.intValue()) == bits.intValue() ) {
                XAtom atom = (XAtom)mapping.get(bits);
                if (contains(atom)) {
                    res.add(atom);
                }
            }
        }
        return res;
    }

    /**
     * Returns iterator for items.
     */
    public Iterator iterator() {
        return atoms.iterator();
    }

    /**
     * Merges without duplicates all the atoms from another list
     */
    public void addAll(XAtomList atoms) {
        Iterator iter = atoms.iterator();
        while(iter.hasNext()) {
            add((XAtom)iter.next());
        }
    }

    public String toString() {
        StringBuffer buf = new StringBuffer();
        buf.append("[");
        Iterator iter = atoms.iterator();
        while (iter.hasNext()) {
            buf.append(iter.next().toString());
            if (iter.hasNext()) {
                buf.append(", ");
            }
        }
        buf.append("]");
        return buf.toString();
    }
}
