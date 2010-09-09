/**
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 */

package java.util;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Map.Entry;


public class TreeMap<K,V>
        extends AbstractMap<K,V>
        implements NavigableMap<K,V>, Cloneable, Serializable {

    private static final long serialVersionUID = 919286545866124006L;

    private transient int size;

    private final Comparator<? super K> comparator;

    private transient int modCount;

    private transient Node<K,V> root;

    private transient Set<Map.Entry<K,V>> entrySet;
    private transient KeySet<K> navigableKeySet;
    private transient NavigableMap<K,V> descendingMap;

    /**
     * Entry is an internal class which is used to hold the entries of a
     * TreeMap.
     *
     * also used to record key, value, and position
     */
    class Entry implements Map.Entry<K,V>, Cloneable {

        final int offset;
        final Node<K,V> node;
        final K key;

        Entry(Node<K,V> node, int offset) {
            this.node = node;
            this.offset = offset;
            key = node.keys[offset];
        }

        public Object clone() {
            try {
                return super.clone();
            } catch (CloneNotSupportedException e) {
                return null;
            }
        }

        public boolean equals(Object object) {
            if (this == object) {
                return true;
            }
            if (object instanceof Map.Entry) {
                Map.Entry<?,?> entry = (Map.Entry<?,?>) object;
                V value = getValue();
                return (key == null ? entry.getKey() == null :
                        key.equals(entry.getKey())) && (value == null ?
                        entry.getValue() == null : value.equals(entry.getValue()));
            }
            return false;
        }

        public K getKey() {
            return key;
        }

        public V getValue() {
            if (node.keys[offset] == key) {
                return node.values[offset];
            }
            if (containsKey(key)) {
                return get(key);
            }
            throw new IllegalStateException();
        }

        public int hashCode() {
            V value = getValue();
            return (key == null ? 0 : key.hashCode()) ^
                   (value == null ? 0 : value.hashCode());
        }

        public V setValue(V object) {
            if (node.keys[offset] == key) {
                V res = node.values[offset];
                node.values[offset] = object;
                return res;
            }
            if (containsKey(key)) {
                return put(key, object);
            }
            throw new IllegalStateException();
        }

        public String toString() {
            return key + "=" + getValue();
        }
    }

    static class Node<K,V> implements Cloneable {
        static final int NODE_SIZE = 64;
        Node<K,V> prev, next;
        Node<K,V> parent, left, right;
        V[] values;
        K[] keys;
        int leftIndex = 0;
        int rightIndex = -1;
        int size = 0;
        boolean color;  // true == black, false == red

        public Node() {
            keys = (K[]) new Object[NODE_SIZE];
            values = (V[]) new Object[NODE_SIZE];
        }

        Node<K,V> clone( Node<K,V> parent) throws CloneNotSupportedException {
            Node<K,V> clone = (Node<K,V>) super.clone();
            clone.keys = (K[]) new Object[NODE_SIZE];
            clone.values = (V[]) new Object[NODE_SIZE];
            System.arraycopy(keys, 0, clone.keys, 0, keys.length);
            System.arraycopy(values, 0, clone.values, 0, values.length);
            clone.leftIndex = leftIndex;
            clone.rightIndex = rightIndex;
            clone.parent = parent;
            if (left != null) {
                clone.left = left.clone(clone);
            }
            if (right != null) {
                clone.right = right.clone(clone);
            }
            clone.prev = null;
            clone.next = null;
            return clone;
        }
    }

    private static <T> Comparable<T> toComparable(T obj) {
        return (Comparable) obj;
    }

    static class AbstractMapIterator<K,V> {

        final TreeMap<K,V> backingMap;
        int expectedModCount;
        Node<K,V> node;
        Node<K,V> lastNode;
        int offset;
        int lastOffset;

        AbstractMapIterator(TreeMap<K,V> map,
                            Node<K,V> startNode,
                            int startOffset) {
            backingMap = map;
            expectedModCount = map.modCount;
            node = startNode;
            offset = startOffset;
        }

        AbstractMapIterator(TreeMap<K,V> map, Node<K,V> startNode) {
            this(map, startNode, startNode != null ?
                 startNode.rightIndex - startNode.leftIndex : 0);
        }

        AbstractMapIterator(TreeMap<K,V> map) {
            this(map, minimum(map.root));
        }

        public boolean hasNext() {
            return node != null;
        }

        final void makeNext() {
            if (expectedModCount != backingMap.modCount) {
                throw new ConcurrentModificationException();
            } else if (node == null) {
                throw new NoSuchElementException();
            }
            lastNode = node;
            lastOffset = offset;
            if (offset != 0) {
                offset--;
            } else {
                node = node.next;
                if (node != null) {
                    offset = node.rightIndex - node.leftIndex;
                }
            }
        }

        public void remove() {
            if (expectedModCount == backingMap.modCount) {
                if (lastNode != null) {
                    int index = lastNode.rightIndex - lastOffset;
                    backingMap.removeFromIterator(lastNode, index);
                    lastNode = null;
                    expectedModCount++;
                } else {
                    throw new IllegalStateException();
                }
            } else {
                throw new ConcurrentModificationException();
            }
        }
    }

    static class UnboundedEntryIterator<K,V>
            extends AbstractMapIterator<K,V>
            implements Iterator<Map.Entry<K,V>> {

        UnboundedEntryIterator(TreeMap<K,V> map,
                               Node<K,V> startNode,
                               int startOffset) {
            super(map, startNode, startOffset);
        }

        UnboundedEntryIterator(TreeMap<K,V> map) {
            super(map);
        }

        public Map.Entry<K,V> next() {
            makeNext();
            int index = lastNode.rightIndex - lastOffset;
            Map.Entry<K,V> e = backingMap.new Entry(lastNode, index);
            return exportEntry(e);
        }
    }

    static class UnboundedKeyIterator<K,V>
            extends AbstractMapIterator<K,V>
            implements Iterator<K> {

        UnboundedKeyIterator(TreeMap<K,V> map,
                             Node<K,V> startNode,
                             int startOffset) {
            super(map, startNode, startOffset);
        }

        UnboundedKeyIterator(TreeMap<K,V> map) {
            super(map);
        }

        public K next() {
            makeNext();
            return lastNode.keys[lastNode.rightIndex - lastOffset];
        }
    }

    static class UnboundedValueIterator<K,V>
            extends AbstractMapIterator<K,V>
            implements Iterator<V> {

        UnboundedValueIterator(TreeMap<K,V> map, Node<K,V> startNode, int startOffset) {
            super(map, startNode, startOffset);
        }

        UnboundedValueIterator(TreeMap<K,V> map) {
            super(map);
        }

        public V next() {
            makeNext();
            return lastNode.values[lastNode.rightIndex - lastOffset];
        }
    }

    static class BoundedMapIterator<K,V> extends AbstractMapIterator<K,V> {

        final Node<K,V> finalNode;
        final int finalOffset;

        BoundedMapIterator(Node<K,V> startNode, int startOffset,
                TreeMap<K,V> map, Node<K,V> finalNode, int finalOffset) {
            super(map, finalNode == null ? null : startNode, startOffset);
            this.finalNode = finalNode;
            this.finalOffset = finalOffset;
        }

        BoundedMapIterator(Node<K,V> startNode, TreeMap<K,V> map,
                           Node<K,V> finalNode, int finalOffset) {
            this(startNode, startNode != null ? startNode.rightIndex - startNode.leftIndex : 0,
                 map, finalNode, finalOffset);
        }

        BoundedMapIterator(Node<K,V> startNode, int startOffset,
                           TreeMap<K,V> map, Node<K,V> finalNode) {
            this(startNode, startOffset, map, finalNode,
                 finalNode.rightIndex - finalNode.leftIndex);
        }

        void makeBoundedNext() {
            makeNext();
            if (lastNode == finalNode && lastOffset == finalOffset) {
                node = null;
            }
        }
    }

    static class BoundedEntryIterator<K,V>
            extends BoundedMapIterator<K,V>
            implements Iterator<Map.Entry<K,V>> {

        public BoundedEntryIterator(Node<K,V> startNode, int startOffset,
                   TreeMap<K,V> map, Node<K,V> finalNode, int finalOffset) {
            super(startNode, startOffset, map, finalNode, finalOffset);
        }

        public Map.Entry<K,V> next() {
            makeBoundedNext();
            int index = lastNode.rightIndex - lastOffset;
            Map.Entry<K,V> e = backingMap.new Entry(lastNode, index);
            return exportEntry(e);
        }
    }

    static class BoundedKeyIterator<K,V>
            extends BoundedMapIterator<K,V>
            implements Iterator<K> {

        public BoundedKeyIterator(Node<K,V> startNode, int startOffset,
                TreeMap<K,V> map, Node<K,V> finalNode, int finalOffset) {
            super(startNode, startOffset, map, finalNode, finalOffset);
        }

        public K next() {
            makeBoundedNext();
            return lastNode.keys[lastNode.rightIndex - lastOffset];
        }
    }

    static class BoundedValueIterator<K,V>
            extends BoundedMapIterator<K,V>
            implements Iterator<V> {

        public BoundedValueIterator(Node<K,V> startNode, int startOffset,
                   TreeMap<K,V> map, Node<K,V> finalNode, int finalOffset) {
            super(startNode, startOffset, map, finalNode, finalOffset);
        }

        public V next() {
            makeBoundedNext();
            return lastNode.values[lastNode.rightIndex - lastOffset];
        }
    }

    static class DescendingMapIterator<K,V> extends AbstractMapIterator<K,V> {

        DescendingMapIterator(TreeMap<K,V> map, Node<K,V> startNode) {
            super(map, startNode);
        }

        DescendingMapIterator(TreeMap<K,V> map,
                              Node<K,V> startNode,
                              int startOffset) {
            super(map, startNode, startOffset);
        }

        final void makePrev() {
            if (expectedModCount != backingMap.modCount) {
                throw new ConcurrentModificationException();
            } else if (node == null) {
                throw new NoSuchElementException();
            }
            lastNode = node;
            lastOffset = offset;
            if (offset != 0) {
                offset--;
            } else {
                node = node.prev;
                if (node != null) {
                    offset = node.rightIndex - node.leftIndex;
                }
            }
        }

        final public void remove() {
            if (expectedModCount == backingMap.modCount) {
                if (lastNode != null) {
                    int index = lastNode.leftIndex + lastOffset;
                    backingMap.removeFromIterator(lastNode, index);
                    lastNode = null;
                    expectedModCount++;
                } else {
                    throw new IllegalStateException();
                }
            } else {
                throw new ConcurrentModificationException();
            }
        }
    }

    static class UnboundedDescendingKeyIterator<K,V>
            extends DescendingMapIterator<K,V>
            implements Iterator<K> {

        UnboundedDescendingKeyIterator(TreeMap<K,V> map) {
            super(map, TreeMap.maximum(map.root));
        }

        UnboundedDescendingKeyIterator(TreeMap<K,V> map,
                                       Node<K,V> startNode,
                                       int startOffset) {
            super(map, startNode, startOffset);
        }

        public K next() {
            makePrev();
            return lastNode.keys[lastNode.leftIndex + lastOffset];
        }
    }

    static class UnboundedDescendingEntryIterator<K,V>
            extends DescendingMapIterator<K,V>
            implements Iterator<Map.Entry<K,V>> {

        UnboundedDescendingEntryIterator(TreeMap<K,V> map) {
            super(map, TreeMap.maximum(map.root));
        }

        UnboundedDescendingEntryIterator(TreeMap<K,V> map,
                                         Node<K,V> startNode,
                                         int startOffset) {
            super(map, startNode, startOffset);
        }

        public Map.Entry<K,V> next() {
            makePrev();
            int index = lastNode.leftIndex + lastOffset;
            Map.Entry<K,V> e = backingMap.new Entry(lastNode, index);
            return exportEntry(e);
        }
    }

    static class BoundedDescendingMapIterator<K,V>
            extends DescendingMapIterator<K,V> {

        final Node<K,V> finalNode;
        final int finalOffset;

        BoundedDescendingMapIterator(TreeMap<K,V> map,
                                     Node<K,V> startNode, int startOffset,
                                     Node<K,V> finalNode, int finalOffset) {
            super(map, finalNode, finalOffset);
            this.finalNode = startNode;
            this.finalOffset = startOffset;

        }

        final void makeBoundedPrev() {
            makePrev();
            if (lastNode == finalNode && lastOffset == finalOffset) {
                node = null;
            }
        }
    }

    static class BoundedDescendingKeyIterator<K,V>
           extends BoundedDescendingMapIterator<K,V>
           implements Iterator<K> {

        public BoundedDescendingKeyIterator(Node<K,V> startNode,
                                        int startOffset, TreeMap<K,V> map,
                                        Node<K,V> finalNode, int finalOffset) {
            super(map, startNode, startOffset, finalNode, finalOffset);
        }

        public K next() {
            makeBoundedPrev();
            return lastNode.keys[lastNode.leftIndex + lastOffset];
        }
    }

    static class BoundedDescendingEntryIterator<K,V>
           extends BoundedDescendingMapIterator<K,V>
           implements Iterator<Map.Entry<K,V>> {

        public BoundedDescendingEntryIterator(Node<K,V> startNode,
                                        int startOffset, TreeMap<K,V> map,
                                        Node<K,V> finalNode, int finalOffset) {
            super(map, startNode, startOffset, finalNode, finalOffset);
        }

        public Map.Entry<K,V> next() {
            makeBoundedPrev();
            int index = lastNode.leftIndex + lastOffset;
            Map.Entry<K,V> e = backingMap.new Entry(lastNode, index);
            return exportEntry(e);
        }
    }

    static class BoundedDescendingValueIterator<K,V>
           extends BoundedDescendingMapIterator<K,V>
           implements Iterator<V> {

        public BoundedDescendingValueIterator(Node<K,V> startNode,
                                        int startOffset, TreeMap<K,V> map,
                                        Node<K,V> finalNode, int finalOffset) {
            super(map, startNode, startOffset, finalNode, finalOffset);
        }

        public V next() {
            makeBoundedPrev();
            return lastNode.values[lastNode.leftIndex + lastOffset];
        }
    }

    private class SubMap extends AbstractMap<K,V>
                               implements SortedMap<K,V>, java.io.Serializable {
        private static final long serialVersionUID = -6520786458950516097L;
        private boolean fromStart = false, toEnd = false;
        private K fromKey, toKey;
        private Object readResolve() {
            return new AscendingSubMap<K,V>(TreeMap.this,
                                            fromStart, fromKey, true,
                                            toEnd, toKey, false);
        }
        public Set<Map.Entry<K,V>> entrySet() { throw new InternalError(); }
        public K lastKey() { throw new InternalError(); }
        public K firstKey() { throw new InternalError(); }
        public SortedMap<K,V> subMap(K fromKey, K toKey) { throw new InternalError(); }
        public SortedMap<K,V> headMap(K toKey) { throw new InternalError(); }
        public SortedMap<K,V> tailMap(K fromKey) { throw new InternalError(); }
        public Comparator<? super K> comparator() { throw new InternalError(); }
    }

    // SubMaps

    static abstract class NavigableSubMap<K,V>
            extends AbstractMap<K,V>
            implements NavigableMap<K,V>, java.io.Serializable {

        /**
         * The backing map.
         */
        final TreeMap<K,V> m;

        /**
         * Endpoints are represented as triples (fromStart, lo,
         * loInclusive) and (toEnd, hi, hiInclusive). If fromStart is
         * true, then the low (absolute) bound is the start of the
         * backing map, and the other values are ignored. Otherwise,
         * if loInclusive is true, lo is the inclusive bound, else lo
         * is the exclusive bound. Similarly for the upper bound.
         */
        final K lo, hi;
        final boolean fromStart, toEnd;
        final boolean loInclusive, hiInclusive;

        // additional attributes for finding keys/values in their Nodes
        transient int loKeyModCount, hiKeyModCount;
        transient Node<K,V> firstKeyNode, lastKeyNode;
        transient int firstKeyIndex, lastKeyIndex;

        NavigableSubMap(TreeMap<K,V> m,
                        boolean fromStart, K lo, boolean loInclusive,
                        boolean toEnd,     K hi, boolean hiInclusive) {
            if (!fromStart && !toEnd) {
                if (m.compare(lo, hi) > 0)
                    throw new IllegalArgumentException("fromKey > toKey");
            } else {
                if (!fromStart) // type check
                    m.compare(lo, lo);
                if (!toEnd)
                    m.compare(hi, hi);
            }
            this.m = m;
            this.fromStart = fromStart;
            this.lo = lo;
            this.loInclusive = loInclusive;
            this.toEnd = toEnd;
            this.hi = hi;
            this.hiInclusive = hiInclusive;
            this.loKeyModCount = this.hiKeyModCount = -1;
        }

        // internal utilities

        final boolean tooLow(Object key) {
            if (!fromStart) {
                int c = m.compare(key, lo);
                if (c < 0 || (c == 0 && !loInclusive))
                    return true;
            }
            return false;
        }

        final boolean tooHigh(Object key) {
            if (!toEnd) {
                int c = m.compare(key, hi);
                if (c > 0 || (c == 0 && !hiInclusive))
                    return true;
            }
            return false;
        }

        final boolean inRange(Object key) {
            return !tooLow(key) && !tooHigh(key);
        }

        final boolean inClosedRange(Object key) {
            return (fromStart || m.compare(key, lo) >= 0)
                && (toEnd || m.compare(hi, key) >= 0);
        }

        final boolean inRange(Object key, boolean inclusive) {
            return inclusive ? inRange(key) : inClosedRange(key);
        }

        /*
         * Absolute versions of relation operations.
         * Subclasses map to these using like-named "sub"
         * versions that invert senses for descending maps
         */

        final Entry<K,V> absLowest() {
            Entry<K,V> e =
                (fromStart ?  m.getFirstEntry() :
                 (loInclusive ? m.getCeilingEntry(lo) :
                                m.getHigherEntry(lo)));
            return (e == null || tooHigh(e.getKey())) ? null : e;
        }

        final Entry<K,V> absHighest() {
            Entry<K,V> e =
                (toEnd ?  m.getLastEntry() :
                 (hiInclusive ?  m.getFloorEntry(hi) :
                                 m.getLowerEntry(hi)));
            return (e == null || tooLow(e.getKey())) ? null : e;
        }

        final Entry<K,V> absCeiling(K key) {
            if (tooLow(key))
                return absLowest();
            Entry<K,V> e = m.getCeilingEntry(key);
            return (e == null || tooHigh(e.getKey())) ? null : e;
        }

        final Entry<K,V> absHigher(K key) {
            if (tooLow(key))
                return absLowest();
            Entry<K,V> e = m.getHigherEntry(key);
            return (e == null || tooHigh(e.getKey())) ? null : e;
        }

        final Entry<K,V> absFloor(K key) {
            if (tooHigh(key))
                return absHighest();
            Entry<K,V> e = m.getFloorEntry(key);
            return (e == null || tooLow(e.getKey())) ? null : e;
        }

        final Entry<K,V> absLower(K key) {
            if (tooHigh(key))
                return absHighest();
            Entry<K,V> e = m.getLowerEntry(key);
            return (e == null || tooLow(e.getKey())) ? null : e;
        }

        // Abstract methods defined in ascending vs descending classes
        // These relay to the appropriate absolute versions

        abstract Entry<K,V> subLowest();
        abstract Entry<K,V> subHighest();
        abstract Entry<K,V> subCeiling(K key);
        abstract Entry<K,V> subHigher(K key);
        abstract Entry<K,V> subFloor(K key);
        abstract Entry<K,V> subLower(K key);

        /** Returns ascending iterator from the perspective of this submap */
        abstract Iterator<K> keyIterator();

        /** Returns descending iterator from the perspective of this submap */
        abstract Iterator<K> descendingKeyIterator();

        // public methods

        @Override
        public boolean isEmpty() {
            return (fromStart && toEnd) ? m.isEmpty() : this.size() == 0;
        }

        @Override
        public int size() {
            if (fromStart && toEnd) {
                return m.size();
            } // else compute size from restricted lo & hi keys range, and lo/hi inclusive
            Node<K,V> from, to;
            int fromIndex, toIndex;
            if (fromStart) {
                from = minimum(m.root);
                fromIndex = from == null ? 0 : from.leftIndex;
            } else {
                setFirstKey();
                from = firstKeyNode;
                fromIndex = firstKeyIndex;
            }
            if (from == null) {
                return 0;
            }
            if (toEnd) {
                to = maximum(m.root);
                toIndex = to == null ? 0 : to.rightIndex;
            } else {
                setLastKey();
                to = lastKeyNode;
                toIndex = lastKeyIndex;
            }
            if (to == null) {
                return 0;
            }
            if (from == to) {
                return toIndex - fromIndex + 1;
            }
            int sum = 0;
            while (from != to) {
                sum += (from.rightIndex - fromIndex + 1);
                from = from.next;
                fromIndex = from.leftIndex;
            }
            return sum + toIndex - fromIndex + 1;
        }

        @Override
        public final boolean containsKey(Object key) {
            return inRange(key) && m.containsKey(key);
        }

        @Override
        public final V put(K key, V value) {
            if (!inRange(key))
                throw new IllegalArgumentException("key out of range");
            return m.put(key, value);
        }

        @Override
        public final V get(Object key) {
            return !inRange(key)? null :  m.get(key);
        }

        @Override
        public final V remove(Object key) {
            return !inRange(key)? null  : m.remove(key);
        }

        public final Map.Entry<K,V> ceilingEntry(K key) {
            return exportEntry(subCeiling(key));
        }

        public final K ceilingKey(K key) {
            return keyOrNull(subCeiling(key));
        }

        public final Map.Entry<K,V> higherEntry(K key) {
            return exportEntry(subHigher(key));
        }

        public final K higherKey(K key) {
            return keyOrNull(subHigher(key));
        }

        public final Map.Entry<K,V> floorEntry(K key) {
            return exportEntry(subFloor(key));
        }

        public final K floorKey(K key) {
            return keyOrNull(subFloor(key));
        }

        public final Map.Entry<K,V> lowerEntry(K key) {
            return exportEntry(subLower(key));
        }

        public final K lowerKey(K key) {
            return keyOrNull(subLower(key));
        }

        public final K firstKey() {
            return key(subLowest());
        }

        public final K lastKey() {
            return key(subHighest());
        }

        public final Map.Entry<K,V> firstEntry() {
            return exportEntry(subLowest());
        }

        public final Map.Entry<K,V> lastEntry() {
            return exportEntry(subHighest());
        }

        public final Map.Entry<K,V> pollFirstEntry() {
            Map.Entry<K,V> e = subLowest();
            Map.Entry<K,V> result = exportEntry(e);
            if (e != null) {
                V value = m.remove(e.getKey());
            }
            return result;
        }

        public final Map.Entry<K,V> pollLastEntry() {
            Map.Entry<K,V> e = subHighest();
            Map.Entry<K,V> result = exportEntry(e);
            if (e != null) {
                V value = m.remove(e.getKey());
            }
            return result;
        }

        // Views
        transient NavigableMap<K,V> descendingMapView = null;
        transient EntrySetView entrySetView = null;
        transient KeySet<K> navigableKeySetView = null;

        public final NavigableSet<K> navigableKeySet() {
            KeySet<K> nksv = navigableKeySetView;
            return (nksv != null) ? nksv :
                (navigableKeySetView = new TreeMap.KeySet<K>((NavigableMap<K, Object>) this));
        }

        @Override
        public final Set<K> keySet() {
            return navigableKeySet();
        }

        public NavigableSet<K> descendingKeySet() {
            return descendingMap().navigableKeySet();
        }

        public final SortedMap<K,V> subMap(K fromKey, K toKey) {
            return subMap(fromKey, true, toKey, false);
        }

        public final SortedMap<K,V> headMap(K toKey) {
            return headMap(toKey, false);
        }

        public final SortedMap<K,V> tailMap(K fromKey) {
            return tailMap(fromKey, true);
        }

        protected void setFirstKey() {
            if (loKeyModCount == m.modCount) {
                return;
            }
            if (loInclusive) {
                setFirstKeyInclusive();
            } else {
                setFirstKeyNonInclusive();
            }
            loKeyModCount = m.modCount;
        }

        private void setFirstKeyNonInclusive() {
            Comparable<K> object = m.comparator == null ? toComparable(lo) : null;
            K key = lo;
            Node<K,V> node = m.root;
            Node<K,V> foundNode = null;
            int foundIndex = -1;
            TOP_LOOP:
            while (node != null) {
                K[] keys = node.keys;
                int leftIndex = node.leftIndex;
                int result = m.cmp(object, key, keys[leftIndex]);
                if (result < 0) {
                    foundNode = node;
                    foundIndex = leftIndex;
                    node = node.left;
                } else {
                    int rightIndex = node.rightIndex;
                    if (leftIndex != rightIndex) {
                        result = m.cmp(object, key, keys[rightIndex]);
                    }
                    if (result >= 0) {
                        node = node.right;
                    } else { /*search in node*/
                        foundNode = node;
                        foundIndex = rightIndex;
                        int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                        while (low <= high) {
                            mid = (low + high) >> 1;
                            result = m.cmp(object, key, keys[mid]);
                            if (result > 0) {
                                low = mid + 1;
                            } else if (result == 0) {
                                foundNode = node;
                                foundIndex = mid + 1;
                                break TOP_LOOP;
                            } else {
                                foundNode = node;
                                foundIndex = mid;
                                high = mid - 1;
                            }
                        }
                        break TOP_LOOP;
                    }
                }
            }
            if (foundNode != null && !checkUpperBound(foundNode.keys[foundIndex])) {
                foundNode = null;
            }
            firstKeyNode = foundNode;
            firstKeyIndex = foundIndex;
        }

        private void setFirstKeyInclusive() {
            Comparable<K> object = m.comparator == null ? toComparable(lo) : null;
            K key = lo;
            Node<K,V> node = m.root;
            Node<K,V> foundNode = null;
            int foundIndex = -1;
            TOP_LOOP:
            while (node != null) {
                K[] keys = node.keys;
                int leftIndex = node.leftIndex;
                int result = m.cmp(object, key, keys[leftIndex]);
                if (result < 0) {
                    foundNode = node;
                    foundIndex = leftIndex;
                    node = node.left;
                } else if (result == 0) {
                    foundNode = node;
                    foundIndex = leftIndex;
                    break;
                } else {
                    int rightIndex = node.rightIndex;
                    if (leftIndex != rightIndex) {
                        result = m.cmp(object, key, keys[rightIndex]);
                    }
                    if (result > 0) {
                        node = node.right;
                    } else if (result == 0) {
                        foundNode = node;
                        foundIndex = rightIndex;
                        break;
                    } else { /*search in node*/
                        foundNode = node;
                        foundIndex = rightIndex;
                        int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                        while (low <= high) {
                            mid = (low + high) >> 1;
                            result = m.cmp(object, key, keys[mid]);
                            if (result > 0) {
                                low = mid + 1;
                            } else if (result == 0) {
                                foundNode = node;
                                foundIndex = mid;
                                break TOP_LOOP;
                            } else {
                                foundNode = node;
                                foundIndex = mid;
                                high = mid - 1;
                            }
                        }
                        break TOP_LOOP;
                    }
                }
            }
            if (foundNode != null && !checkUpperBound(foundNode.keys[foundIndex])) {
                foundNode = null;
            }
            firstKeyNode = foundNode;
            firstKeyIndex = foundIndex;
        }

        protected void setLastKey() {
            if (hiKeyModCount == m.modCount) {
                return;
            }
            if (hiInclusive) {
                setLastKeyInclusive();
            } else {
                setLastKeyNonInclusive();
            }
            hiKeyModCount = m.modCount;
        }

        private void setLastKeyInclusive() {
           Comparable<K> object = m.comparator == null ? toComparable(hi) : null;
           K key = hi;
           Node<K,V> node = m.root;
           Node<K,V> foundNode = null;
           int foundIndex = -1;
           TOP_LOOP:
           while (node != null) {
               K[] keys = node.keys;
               int leftIndex = node.leftIndex;
               int result = m.cmp(object, key, keys[leftIndex]);
               if (result < 0) {
                   node = node.left;
               } else if (result == 0) {
                   foundNode = node;
                   foundIndex = leftIndex;
                   break;
               } else {
                   int rightIndex = node.rightIndex;
                   if (leftIndex != rightIndex) {
                       result = m.cmp(object, key, keys[rightIndex]);
                   }
                   if (result > 0) {
                       foundNode = node;
                       foundIndex = rightIndex;
                       node = node.right;
                   } else if (result == 0) {
                       foundNode = node;
                       foundIndex = rightIndex;
                       break;
                   } else { /*search in node*/
                       foundNode = node;
                       foundIndex = leftIndex;
                       int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                       while (low <= high) {
                           mid = (low + high) >> 1;
                           result = m.cmp(object, key, keys[mid]);
                           if (result > 0) {
                               foundNode = node;
                               foundIndex = mid;
                               low = mid + 1;
                           } else if (result == 0) {
                               foundNode = node;
                               foundIndex = mid;
                               break TOP_LOOP;
                           } else {
                               high = mid - 1;
                           }
                       }
                       break TOP_LOOP;
                   }
               }
           }
           if (foundNode != null && !checkLowerBound(foundNode.keys[foundIndex])) {
               foundNode = null;
           }
           lastKeyNode = foundNode;
           lastKeyIndex = foundIndex;
        }

        private void setLastKeyNonInclusive() {
            Comparable<K> object = m.comparator == null ? toComparable(hi) : null;
            K key = hi;
            Node<K,V> node = m.root;
            Node<K,V> foundNode = null;
            int foundIndex = -1;
            TOP_LOOP:
            while (node != null) {
                K[] keys = node.keys;
                int leftIndex = node.leftIndex;
                int result = m.cmp(object, key, keys[leftIndex]);
                if (result <= 0) {
                    node = node.left;
                } else {
                    int rightIndex = node.rightIndex;
                    if (leftIndex != rightIndex) {
                        result = m.cmp(object, key, keys[rightIndex]);
                    }
                    if (result > 0) {
                        foundNode = node;
                        foundIndex = rightIndex;
                        node = node.right;
                    } else if (result == 0) {
                        if (node.leftIndex == node.rightIndex) {
                            foundNode = node.prev;
                            if (foundNode != null) {
                                foundIndex = foundNode.rightIndex - 1;
                            }
                        } else {
                            foundNode = node;
                            foundIndex = rightIndex - 1;
                        }
                        break;
                    } else { /*search in node*/
                        foundNode = node;
                        foundIndex = leftIndex;
                        int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                        while (low <= high) {
                            mid = (low + high) >> 1;
                            result = m.cmp(object, key, keys[mid]);
                            if (result > 0) {
                                foundNode = node;
                                foundIndex = mid;
                                low = mid + 1;
                            } else if (result == 0) {
                                foundNode = node;
                                foundIndex = mid - 1;
                                break TOP_LOOP;
                            } else {
                                high = mid - 1;
                            }
                        }
                        break TOP_LOOP;
                    }
                }
            }
            if (foundNode != null && !checkLowerBound(foundNode.keys[foundIndex])) {
                foundNode = null;
            }
            lastKeyNode = foundNode;
            lastKeyIndex = foundIndex;
        }

        private boolean checkLowerBound(K key) {
            if (!fromStart) {  // has a restricted starting key
                Comparator<? super K> cmp = m.comparator;
                if (cmp != null) {
                    if (loInclusive) return (cmp.compare(key, lo) >= 0);
                    return (cmp.compare(key, lo) > 0);
                }
                if (loInclusive) return (toComparable(key).compareTo(lo) >= 0);
                return (toComparable(key).compareTo(lo) > 0);
            } // else no restricted starting key
            return true;
        }

        private boolean checkUpperBound(K key) {
            if (!toEnd) { // has a restricted ending key
                Comparator<? super K> cmp = m.comparator;
                if (cmp != null) {
                    if (hiInclusive) return (cmp.compare(key, hi) <= 0);
                    return (cmp.compare(key, hi) < 0);
                }
                if (hiInclusive) return (toComparable(key).compareTo(hi) <= 0);
                return (toComparable(key).compareTo(hi) < 0);
            } // else no restricted ending key
            return true;
        }

        // View classes

        abstract class EntrySetView extends AbstractSet<Map.Entry<K,V>> {
            private transient int esvSize = -1, esvSizeModCount;

            public int size() {
                if (fromStart && toEnd)
                    return m.size();
                if (esvSize == -1 || esvSizeModCount != m.modCount) {
                    esvSizeModCount = m.modCount;
                    esvSize = 0;
                    Iterator i = iterator();
                    while (i.hasNext()) {
                        esvSize++;
                        i.next();
                    }
                }
                return esvSize;
            }

            @Override
            public boolean isEmpty() {
                Map.Entry<K,V> n = absLowest();
                return n == null || tooHigh(n.getKey());
            }

            @Override
            public boolean contains(Object o) {
                if (!(o instanceof Map.Entry))
                    return false;
                @SuppressWarnings("unchecked")
                Map.Entry<K,V> entry = (Map.Entry<K,V>) o;
                K key = entry.getKey();
                if (!inRange(key))
                    return false;
                Map.Entry mEntry = m.getEntry(key);
                return mEntry != null &&
                    valEquals(mEntry.getValue(), entry.getValue());
            }

            @Override
            public boolean remove(Object o) {
                if (!(o instanceof Map.Entry))
                    return false;
                @SuppressWarnings("unchecked")
                Map.Entry<K,V> entry = (Map.Entry<K,V>) o;
                K key = entry.getKey();
                if (!inRange(key))
                    return false;
                Map.Entry<K,V> mEntry = m.getEntry(key);
                if (mEntry!=null && valEquals(mEntry.getValue(),entry.getValue())){
                    V value = m.remove(mEntry.getKey());
                    return true;
                }
                return false;
            }
        }

        private void readObject(ObjectInputStream stream) throws IOException,
                ClassNotFoundException {
            stream.defaultReadObject();
            loKeyModCount = -1;
            hiKeyModCount = -1;
        }
    }

    // AscendingSubMap

    static final class AscendingSubMap<K,V> extends NavigableSubMap<K,V> {
        private static final long serialVersionUID = 912986545866124060L;
        AscendingSubMap(TreeMap<K,V> m,
                        boolean fromStart, K lo, boolean loInclusive,
                        boolean toEnd,     K hi, boolean hiInclusive) {
            super(m, fromStart, lo, loInclusive, toEnd, hi, hiInclusive);
        }

        public Comparator<? super K> comparator() {
            return m.comparator();
        }

        public NavigableMap<K,V> subMap(K fromKey, boolean fromInclusive,
                                        K toKey,   boolean toInclusive) {
            if (!inRange(fromKey, fromInclusive))
                throw new IllegalArgumentException("fromKey out of range");
            if (!inRange(toKey, toInclusive))
                throw new IllegalArgumentException("toKey out of range");
            return new AscendingSubMap<K,V>(m,
                                            false, fromKey, fromInclusive,
                                            false, toKey,   toInclusive);
        }

        public NavigableMap<K,V> headMap(K toKey, boolean inclusive) {
            if (!inRange(toKey, inclusive))
                throw new IllegalArgumentException("toKey out of range");
            return new AscendingSubMap<K,V>(m,
                                            fromStart, lo,    loInclusive,
                                            false,     toKey, inclusive);
        }

        public NavigableMap<K,V> tailMap(K fromKey, boolean inclusive){
            if (!inRange(fromKey, inclusive))
                throw new IllegalArgumentException("fromKey out of range");
            return new AscendingSubMap<K,V>(m,
                                            false, fromKey, inclusive,
                                            toEnd, hi,      hiInclusive);
        }

        public NavigableMap<K,V> descendingMap() {
            NavigableMap<K,V> mv = descendingMapView;
            return (mv != null) ? mv :
                (descendingMapView =
                 new DescendingSubMap<K,V>(m,
                                           fromStart, lo, loInclusive,
                                           toEnd,     hi, hiInclusive));
        }

        Iterator<K> keyIterator() {
            Node<K,V> fromNode;
            int fromIndex;
            if (fromStart) { // start at backing map's absolute lowest key
                fromNode = minimum(m.root);
                fromIndex = (fromNode != null ? fromNode.leftIndex : 0);
            } else {  // start at sub map's restricted lowest key
                setFirstKey();
                fromNode = firstKeyNode;
                fromIndex = firstKeyIndex;
            }
            if (toEnd) { // end at backing map's absolute highest key
                return new UnboundedKeyIterator<K,V>(m, fromNode,
                        fromNode == null ? 0 : fromNode.rightIndex - fromIndex);
            } // else end at sub map's restricted highest key
            setLastKey();
            Node<K,V> toNode = lastKeyNode;
            int toIndex = lastKeyIndex;
            return new BoundedKeyIterator<K,V>(fromNode,
                    fromNode == null ? 0 : fromNode.rightIndex - fromIndex, m, toNode,
                    toNode == null ? 0 : toNode.rightIndex - toIndex);
        }

        Iterator<K> descendingKeyIterator() {
            Node<K,V> fromNode, toNode;
            int fromIndex, toIndex;
            // find "to" Node and "to" Key's index
            if (toEnd) { // start at backing map's absolute highest key
                toNode = maximum(m.root);
                toIndex = (toNode == null ? 0 : toNode.rightIndex);
            } else { // start at sub map's restricted highest key
                setLastKey();
                toNode = lastKeyNode;
                toIndex = lastKeyIndex;
            }
            int toOffset = (toNode == null ? 0 : toIndex - toNode.leftIndex);
            // find from Node and from Key's index
            if (fromStart) { // end at backing map's absolute lowest key
                return new UnboundedDescendingKeyIterator<K,V>(m, toNode, toOffset);
            }
            // else end at sub map's restricted lowest key
            setFirstKey();
            fromNode = firstKeyNode;
            if (fromNode == null) toNode = null; // empty Iterator case
            fromIndex = firstKeyIndex;
            int fromOffset = (fromNode == null ? 0 : fromIndex - fromNode.leftIndex);
            Iterator<K> itr =
                    new BoundedDescendingKeyIterator<K,V>(fromNode, fromOffset,
                                                           m, toNode, toOffset);
            return itr;
        }

        final class AscendingEntrySetView extends EntrySetView {
            public Iterator<Map.Entry<K,V>> iterator() {
                Node<K,V> fromNode;
                int fromIndex;
                if (fromStart) { // start at backing map's absolute lowest key
                    fromNode = minimum(m.root);
                    fromIndex = (fromNode != null ? fromNode.leftIndex : 0);
                } else {  // start at sub map's restricted lowest key
                    setFirstKey();
                    fromNode = firstKeyNode;
                    fromIndex = firstKeyIndex;
                }
                if (toEnd) { // end at backing map's absolute highest key
                    return new UnboundedEntryIterator<K,V>(m, fromNode,
                            fromNode == null ? 0 : fromNode.rightIndex - fromIndex);
                } // end at sub map's restricted highest key
                setLastKey();
                Node<K,V> toNode = lastKeyNode;
                int toIndex = lastKeyIndex;
                return new BoundedEntryIterator<K,V>(fromNode,
                        fromNode == null ? 0 : fromNode.rightIndex - fromIndex, m, toNode,
                        toNode == null ? 0 : toNode.rightIndex - toIndex);
            }
        }

        @Override
        public Set<Map.Entry<K,V>> entrySet() {
            EntrySetView es = entrySetView;
            return (es != null) ? es : (entrySetView = new AscendingEntrySetView());
        }

        Map.Entry<K,V> subLowest()       { return absLowest(); }
        Map.Entry<K,V> subHighest()      { return absHighest(); }
        Map.Entry<K,V> subCeiling(K key) { return absCeiling(key); }
        Map.Entry<K,V> subHigher(K key)  { return absHigher(key); }
        Map.Entry<K,V> subFloor(K key)   { return absFloor(key); }
        Map.Entry<K,V> subLower(K key)   { return absLower(key); }
    }

    // DescendingSubMap

    static final class DescendingSubMap<K,V> extends NavigableSubMap<K,V> {
        private static final long serialVersionUID = 912986545866120460L;
        private final Comparator<? super K> reverseComparator;

        DescendingSubMap(TreeMap<K,V> m,
                        boolean fromStart, K lo, boolean loInclusive,
                        boolean toEnd,     K hi, boolean hiInclusive) {
            super(m, fromStart, lo, loInclusive, toEnd, hi, hiInclusive);
            reverseComparator = Collections.reverseOrder(m.comparator);
        }

        public Comparator<? super K> comparator() {
            return reverseComparator;
        }

        public NavigableMap<K,V> subMap(K fromKey, boolean fromInclusive,
                                        K toKey,   boolean toInclusive) {
            if (!inRange(fromKey, fromInclusive))
                throw new IllegalArgumentException("fromKey out of range");
            if (!inRange(toKey, toInclusive))
                throw new IllegalArgumentException("toKey out of range");
            return new DescendingSubMap<K,V>(m,
                                             false, toKey,   toInclusive,
                                             false, fromKey, fromInclusive);
        }

        public NavigableMap<K,V> headMap(K toKey, boolean inclusive) {
            if (!inRange(toKey, inclusive))
                throw new IllegalArgumentException("toKey out of range");
            return new DescendingSubMap<K,V>(m,
                                             false, toKey, inclusive,
                                             toEnd, hi,    hiInclusive);
        }

        public NavigableMap<K,V> tailMap(K fromKey, boolean inclusive){
            if (!inRange(fromKey, inclusive))
                throw new IllegalArgumentException("fromKey out of range");
            return new DescendingSubMap<K,V>(m,
                                             fromStart, lo, loInclusive,
                                             false, fromKey, inclusive);
        }

        public NavigableMap<K,V> descendingMap() {
            NavigableMap<K,V> mv = descendingMapView;
            return (mv != null) ? mv :
                (descendingMapView =
                 new AscendingSubMap<K,V>(m,
                                          fromStart, lo, loInclusive,
                                          toEnd,     hi, hiInclusive));
        }

        Iterator<K> keyIterator() {
            Node<K,V> fromNode, toNode;
            int fromIndex, toIndex;
            // find to Node and to Key's index
            if (toEnd) { // start at backing map's absolute highest key
                toNode = maximum(m.root);
                toIndex = (toNode == null ? 0 : toNode.rightIndex);
            } else { // start at sub map's restricted highest key
                setLastKey();
                toNode = lastKeyNode;
                toIndex = lastKeyIndex;
            }
            int toOffset = (toNode == null ? 0 : toIndex - toNode.leftIndex);
            // find from Node and from Key's index
            if (fromStart) { // end at backing map's absolute lowest key
                return new UnboundedDescendingKeyIterator<K,V>(m, toNode, toOffset);
            }
            // else end at sub map's restricted lowest key
            setFirstKey();
            fromNode = firstKeyNode;
            if (fromNode == null) toNode = null; // empty Iterator case
            fromIndex = firstKeyIndex;
            int fromOffset = (fromNode == null ? 0 : fromIndex - fromNode.leftIndex);
            Iterator<K> itr =
                    new BoundedDescendingKeyIterator<K,V>(fromNode, fromOffset,
                                                          m, toNode, toOffset);
            return itr;
        }

        Iterator<K> descendingKeyIterator() {
            Node<K,V> fromNode;
            int fromIndex;
            if (fromStart) { // start at backing map's absolute lowest key
                fromNode = minimum(m.root);
                fromIndex = (fromNode != null ? fromNode.leftIndex : 0);
            } else {  // start at sub map's restricted lowest key
                setFirstKey();
                fromNode = firstKeyNode;
                fromIndex = firstKeyIndex;
            }
            if (toEnd) { // end at backing map's absolute highest key
                return new UnboundedKeyIterator<K,V>(m, fromNode,
                        fromNode == null ? 0 : fromNode.rightIndex - fromIndex);
            } // else end at sub map's restricted highest key
            setLastKey();
            Node<K,V> toNode = lastKeyNode;
            int toIndex = lastKeyIndex;
            return new BoundedKeyIterator<K,V>(fromNode,
                    fromNode == null ? 0 : fromNode.rightIndex - fromIndex, m, toNode,
                    toNode == null ? 0 : toNode.rightIndex - toIndex);
        }

        final class DescendingEntrySetView extends EntrySetView {
            public Iterator<Map.Entry<K,V>> iterator() {
                Node<K,V> fromNode, toNode;
                int fromIndex, toIndex;
                // find to Node and to Key's index
                if (toEnd) { // start at backing map's absolute highest key
                    toNode = maximum(m.root);
                    toIndex = (toNode == null ? 0 : toNode.rightIndex);
                } else { // start at sub map's restricted highest key
                    setLastKey();
                    toNode = lastKeyNode;
                    toIndex = lastKeyIndex;
                }
                int toOffset = (toNode == null ? 0 : toIndex - toNode.leftIndex);
                // find from Node and from Key's index
                if (fromStart) { // end at backing map's absolute lowest key
                    return new UnboundedDescendingEntryIterator<K,V>(m, toNode, toOffset);
                }
                // else end at sub map's restricted lowest key
                setFirstKey();
                fromNode = firstKeyNode;
                if (fromNode == null) {
                    toNode = null; // empty Iterator case
                }
                fromIndex = firstKeyIndex;
                int fromOffset = (fromNode == null ? 0 : fromIndex - fromNode.leftIndex);
                Iterator<Map.Entry<K,V>> itr =
                        new BoundedDescendingEntryIterator<K,V>(fromNode, fromOffset,
                                                                m, toNode, toOffset);
                return itr;
            }
        }

        @Override
        public Set<Map.Entry<K,V>> entrySet() {
            EntrySetView es = entrySetView;
            return (es != null) ? es : (entrySetView = new DescendingEntrySetView());
        }

        Map.Entry<K,V> subLowest()       { return absHighest(); }
        Map.Entry<K,V> subHighest()      { return absLowest(); }
        Map.Entry<K,V> subCeiling(K key) { return absFloor(key); }
        Map.Entry<K,V> subHigher(K key)  { return absLower(key); }
        Map.Entry<K,V> subFloor(K key)   { return absCeiling(key); }
        Map.Entry<K,V> subLower(K key)   { return absHigher(key); }
    }

    /*
     * Unlike Values and EntrySet, the KeySet class is static,
     * delegating to a NavigableMap to allow use by SubMaps, which
     * outweighs the ugliness of needing type-tests for the following
     * Iterator methods that are defined appropriately in main versus
     * submap classes.
     */

    Iterator<K> keyIterator() {
        return new UnboundedKeyIterator<K,V>(this);
    }

    Iterator<K> descendingKeyIterator() {
        return new UnboundedDescendingKeyIterator<K,V>(this);
    }

    static final class KeySet<E>
            extends AbstractSet<E>
            implements NavigableSet<E> {
       private final NavigableMap<E,Object> m;

        KeySet(NavigableMap<E,Object> map) {
            m = map;
        }

        public Iterator<E> iterator() {
            if (m instanceof TreeMap) {
                return ((TreeMap<E,Object>)m).keyIterator();
            } else {
                return (((TreeMap.NavigableSubMap<E,Object>) m).keyIterator());
            }
        }

        public Iterator<E> descendingIterator() {
            if (m instanceof TreeMap)
                return ((TreeMap<E,Object>)m).descendingKeyIterator();
            else
                return (((TreeMap.NavigableSubMap<E,Object>) m).descendingKeyIterator());
        }

        public int size() { return m.size(); }
        @Override
        public boolean isEmpty() { return m.isEmpty(); }
        @Override
        public boolean contains(Object o) { return m.containsKey(o); }
        @Override
        public void clear() { m.clear(); }
        public E lower(E e) { return m.lowerKey(e); }
        public E floor(E e) { return m.floorKey(e); }
        public E ceiling(E e) { return m.ceilingKey(e); }
        public E higher(E e) { return m.higherKey(e); }
        public E first() { return m.firstKey(); }
        public E last() { return m.lastKey(); }
        public Comparator<? super E> comparator() { return m.comparator(); }
        public E pollFirst() {
            Map.Entry<E,Object> e = m.pollFirstEntry();
            return e == null? null : e.getKey();
        }
        public E pollLast() {
            Map.Entry<E,Object> e = m.pollLastEntry();
            return e == null? null : e.getKey();
        }
        @Override
        public boolean remove(Object o) {
            int oldSize = size();
            m.remove(o);
            return size() != oldSize;
        }
        public NavigableSet<E> subSet(E fromElement, boolean fromInclusive,
                                      E toElement,   boolean toInclusive) {
            return new TreeSet<E>(m.subMap(fromElement, fromInclusive,
                                           toElement,   toInclusive));
        }
        public NavigableSet<E> headSet(E toElement, boolean inclusive) {
            return new TreeSet<E>(m.headMap(toElement, inclusive));
        }
        public NavigableSet<E> tailSet(E fromElement, boolean inclusive) {
            return new TreeSet<E>(m.tailMap(fromElement, inclusive));
        }
        public SortedSet<E> subSet(E fromElement, E toElement) {
            return subSet(fromElement, true, toElement, false);
        }
        public SortedSet<E> headSet(E toElement) {
            return headSet(toElement, false);
        }
        public SortedSet<E> tailSet(E fromElement) {
            return tailSet(fromElement, true);
        }
        @SuppressWarnings("unchecked")
        public NavigableSet<E> descendingSet() {
            return new TreeSet(m.descendingMap());
        }
    }

    public TreeMap() {
        comparator = null;
    }


    public TreeMap(Comparator<? super K> comparator) {
        this.comparator = comparator;
    }

    public TreeMap(Map<? extends K, ? extends V> m) {
        comparator = null;
        putAll(m);
    }

    public TreeMap(SortedMap<K, ? extends V> m) {
        this(m.comparator());
        Node<K,V> lastNode = null;
        Iterator<? extends Map.Entry<K, ? extends V>> it = m.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<K, ? extends V> entry = it.next();
            lastNode = addToLast(lastNode, entry.getKey(), entry.getValue());
        }
    }

    // Add key / value to the last node
    private Node<K,V> addToLast(Node<K,V> last, K key, V value) {
        if (last == null) {
            root = last = createNode(key, value);
            size = 1;
        } else if (last.size == Node.NODE_SIZE) {
            Node<K,V> newNode = createNode(key, value);
            attachToRight(last, newNode);
            balance(newNode);
            size++;
            last = newNode;
        } else {
            appendFromRight(last, key, value);
            size++;
        }
        return last;
    }

    @Override
    public void clear() {
        root = null;
        size = 0;
        modCount++;
    }

    @Override
    public Object clone() {
        try {
            TreeMap<K,V> clone = (TreeMap<K,V>) super.clone();
            clone.entrySet = null;
            if (root != null) {
                clone.root = root.clone(null);
                // restore prev/next chain
                Node<K,V> node = minimum(clone.root);
                while (true) {
                    Node<K,V> nxt = successor(node);
                    if (nxt == null) {
                        break;
                    }
                    nxt.prev = node;
                    node.next = nxt;
                    node = nxt;
                }
            }
            return clone;
        } catch (CloneNotSupportedException e) {
            return null;
        }
    }

    // Return the successor Node to  node.
    static private <K,V> Node<K,V> successor(Node<K,V> node) {
        if (node.right != null) {
            return minimum(node.right);
        }
        Node<K,V> n = node.parent;
        while (n != null && node == n.right) {
            node = n;
            n = n.parent;
        }
        return n;
    }

    public Comparator<? super K> comparator() {
        return comparator;
    }

    @Override
    public boolean containsKey(Object key) {
        Comparable<K> object = comparator == null ? toComparable((K) key) : null;
        K keyK = (K) key;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            int result = cmp(object, keyK, keys[leftIndex]);
            if (result < 0) {
                node = node.left;
            } else if (result == 0) {
                return true;
            } else {
                int rightIndex = node.rightIndex;
                if (leftIndex != rightIndex) {
                    result = cmp(object, keyK, keys[rightIndex]);
                }
                if (result > 0) {
                    node = node.right;
                } else if (result == 0) {
                    return true;
                } else { /* search in node */
                    int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                    while (low <= high) {
                        mid = (low + high) >> 1;
                        result = cmp(object, keyK, keys[mid]);
                        if (result > 0) {
                            low = mid + 1;
                        } else if (result == 0) {
                            return true;
                        } else {
                            high = mid - 1;
                        }
                    }
                    return false;
                }
            }
        }
        return false;
    }

    @Override
    public boolean containsValue(Object value) {
        if (root == null) {
            return false;
        }
        Node<K,V> node = minimum(root);
        if (value != null) {
            while (node != null) {
                int to = node.rightIndex;
                V[] tmpValues = node.values;
                for (int i = node.leftIndex; i <= to; i++) {
                    if (value.equals(tmpValues[i])) {
                        return true;
                    }
                }
                node = node.next;
            }
        } else {
            while (node != null) {
                int to = node.rightIndex;
                V[] tmpValues = node.values;
                for (int i = node.leftIndex; i <= to; i++) {
                    if (tmpValues[i] == null) {
                        return true;
                    }
                }
                node = node.next;
            }
        }
        return false;
    }

    @Override
    public Set<Map.Entry<K,V>> entrySet() {
        if (entrySet == null) {
            entrySet = new AbstractSet<Map.Entry<K,V>>() {

                @Override
                public int size() {
                    return TreeMap.this.size;
                }

                @Override
                public void clear() {
                    TreeMap.this.clear();
                }

                @SuppressWarnings("unchecked")
                @Override
                public boolean contains(Object object) {
                    if (object instanceof Map.Entry) {
                        Map.Entry<K,V> entry = (Map.Entry<K,V>) object;
                        K key = entry.getKey();
                        Object v1 = TreeMap.this.get(key), v2 = entry.getValue();
                        return v1 == null ? (v2 == null && TreeMap.this.containsKey(key)) : v1.equals(v2);
                    }
                    return false;
                }

                @Override
                public boolean remove(Object object) {
                    if (contains(object)) {
                        @SuppressWarnings("unchecked")
                        Map.Entry<K,V> entry = (Map.Entry<K,V>) object;
                        K key = entry.getKey();
                        TreeMap.this.remove(key);
                        return true;
                    }
                    return false;
                }

                @Override
                public Iterator<Map.Entry<K,V>> iterator() {
                    return new UnboundedEntryIterator<K,V>(TreeMap.this);
                }
            };
        }
        return entrySet;
    }

    public K firstKey() {
        if (root != null) {
            Node<K,V> node = minimum(root);
            return node.keys[node.leftIndex];
        }
        throw new NoSuchElementException();
    }

    @Override
    public V get(Object key) {
        Comparable<K> object = comparator == null ? toComparable((K) key) : null;
        K keyK = (K) key;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            int result = cmp(object, keyK, keys[leftIndex]);
            if (result < 0) {
                node = node.left;
            } else if (result == 0) {
                return node.values[leftIndex];
            } else {
                int rightIndex = node.rightIndex;
                if (leftIndex != rightIndex) {
                    result = cmp(object, keyK, keys[rightIndex]);
                }
                if (result > 0) {
                    node = node.right;
                } else if (result == 0) {
                    return node.values[rightIndex];
                } else { /* search in node */
                    int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                    while (low <= high) {
                        mid = (low + high) >> 1;
                        result = cmp(object, keyK, keys[mid]);
                        if (result > 0) {
                            low = mid + 1;
                        } else if (result == 0) {
                            return node.values[mid];
                        } else {
                            high = mid - 1;
                        }
                    }
                    return null;
                }
            }
        }
        return null;
    }

    final Map.Entry<K,V> getEntry(Object key) {
        Comparable<K> object = comparator == null ? toComparable((K) key) : null;
        K keyK = (K) key;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            int result = cmp(object, keyK, keys[leftIndex]);
            if (result < 0) {
                node = node.left;
            } else if (result == 0) {
                return new TreeMap.Entry(node, leftIndex);
            } else {
                int rightIndex = node.rightIndex;
                if (leftIndex != rightIndex) {
                    result = cmp(object, keyK, keys[rightIndex]);
                }
                if (result > 0) {
                    node = node.right;
                } else if (result == 0) {
                    return new TreeMap.Entry(node, rightIndex);
                } else { /*search in node*/
                    int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                    while (low <= high) {
                        mid = (low + high) >> 1;
                        result = cmp(object, keyK, keys[mid]);
                        if (result > 0) {
                            low = mid + 1;
                        } else if (result == 0) {
                            return new TreeMap.Entry(node, mid);
                        } else {
                            high = mid - 1;
                        }
                    }
                    return null;
                }
            }
        }
        return null;
    }

    private int cmp(Comparable<K> object, K key1, K key2) {
        return object != null ? object.compareTo(key2) : comparator.compare(key1, key2);
    }

    public SortedMap<K,V> headMap(K toKey) {
        SortedMap<K,V> sortedMap = headMap(toKey, false);
        return sortedMap;
    }

    public NavigableMap<K,V> headMap(K toKey, boolean inclusive) {
        NavigableMap<K,V> nm = new AscendingSubMap<K,V>(this,
                                                       true,  null,  true,
                                                       false, toKey, inclusive);
        return nm;
    }

    @Override
    public Set<K> keySet() {
        return navigableKeySet();
    }

    public K lastKey() {
        if (root != null) {
            Node<K,V> node = maximum(root);
            return node.keys[node.rightIndex];
        }
        throw new NoSuchElementException();
    }

    // Find and return the Node holding the mininum Entry.key
    // in the sub-tree below this node, or null if node is null.
    private static <K,V> Node<K,V> minimum(Node<K,V> node) {
        if (node == null) {
            return null;
        }
        while (node.left != null) {
            node = node.left;
        }
        return node;
    }

    // Find and return the Node holding the maximum Entry.key
    // in the tree with Entries greater than this node.
    private static <K,V> Node<K,V> maximum(Node<K,V> node) {
        if (node == null) {
            return null;
        }
        while (node.right != null) {
            node = node.right;
        }
        return node;
    }

    @Override
    public V put(K key, V value) {
        if (root == null) {
            root = createNode(key, value);
            size = 1;
            modCount++;
            return null;
        }
        Comparable<K> object = comparator == null ? toComparable(key) : null;
        K keyK = key;
        Node<K,V> node = root;
        Node<K,V> prevNode = null;
        int result = 0;
        while (node != null) {
            prevNode = node;
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            result = cmp(object, keyK, keys[leftIndex]);
            if (result < 0) {
                node = node.left;
            } else if (result == 0) {
                V res = node.values[leftIndex];
                node.values[leftIndex] = value;
                return res;
            } else {
                int rightIndex = node.rightIndex;
                if (leftIndex != rightIndex) {
                    result = cmp(object, keyK, keys[rightIndex]);
                }
                if (result > 0) {
                    node = node.right;
                } else if (result == 0) {
                    V res = node.values[rightIndex];
                    node.values[rightIndex] = value;
                    return res;
                } else { /*search in node*/
                    int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                    while (low <= high) {
                        mid = (low + high) >> 1;
                        result = cmp(object, keyK, keys[mid]);
                        if (result > 0) {
                            low = mid + 1;
                        } else if (result == 0) {
                            V res = node.values[mid];
                            node.values[mid] = value;
                            return res;
                        } else {
                            high = mid - 1;
                        }
                    }
                    result = low;
                    break;
                }
            }
        } /* while */

        size++;
        modCount++;
        if (node == null) {
            if (prevNode == null) {
                // case of empty Tree
                root = createNode(key, value);
            } else if (prevNode.size < Node.NODE_SIZE) {
                // there is a place for insert
                if (result < 0) {
                    appendFromLeft(prevNode, key, value);
                } else {
                    appendFromRight(prevNode, key, value);
                }
            } else {
                // create and link
                Node<K,V> newNode = createNode(key, value);
                if (result < 0) {
                    attachToLeft(prevNode, newNode);
                } else {
                    attachToRight(prevNode, newNode);
                }
                balance(newNode);
            }
        } else {
            // insert into node.
            // result - index where it should be inserted.
            if (node.size < Node.NODE_SIZE) { // insert and ok
                int leftIndex = node.leftIndex;
                int rightIndex = node.rightIndex;
                if (leftIndex == 0 || ((rightIndex != Node.NODE_SIZE - 1) && (rightIndex - result <= result - leftIndex))) {
                    int rightIndexPlus1 = rightIndex + 1;
                    System.arraycopy(node.keys, result, node.keys, result + 1, rightIndexPlus1 - result);
                    System.arraycopy(node.values, result, node.values, result + 1, rightIndexPlus1 - result);
                    node.rightIndex = rightIndexPlus1;
                    node.keys[result] = key;
                    node.values[result] = value;
                } else {
                    int leftIndexMinus1 = leftIndex - 1;
                    System.arraycopy(node.keys, leftIndex, node.keys, leftIndexMinus1, result - leftIndex);
                    System.arraycopy(node.values, leftIndex, node.values, leftIndexMinus1, result - leftIndex);
                    node.leftIndex = leftIndexMinus1;
                    node.keys[result - 1] = key;
                    node.values[result - 1] = value;
                }
                node.size++;
            } else {
                // there are no place here
                // insert and push old pair
                Node<K,V> previous = node.prev;
                Node<K,V> nextNode = node.next;
                boolean removeFromStart;
                boolean attachFromLeft = false;
                Node<K,V> attachHere = null;
                if (previous == null) {
                    if (nextNode != null && nextNode.size < Node.NODE_SIZE) {
                        // move last pair to next
                        removeFromStart = false;
                    } else {
                        // next node doesn't exist or full
                        // left==null
                        // drop first pair to new node from left
                        removeFromStart = true;
                        attachFromLeft = true;
                        attachHere = node;
                    }
                } else if (nextNode == null) {
                    if (previous.size < Node.NODE_SIZE) {
                        // move first pair to prev
                        removeFromStart = true;
                    } else {
                        // right == null;
                        // drop last pair to new node from right
                        removeFromStart = false;
                        attachFromLeft = false;
                        attachHere = node;
                    }
                } else {
                    if (previous.size < Node.NODE_SIZE) {
                        if (nextNode.size < Node.NODE_SIZE) {
                            // choose prev or next for moving
                            removeFromStart = previous.size < nextNode.size;
                        } else {
                            // move first pair to prev
                            removeFromStart = true;
                        }
                    } else {
                        if (nextNode.size < Node.NODE_SIZE) {
                            // move last pair to next
                            removeFromStart = false;
                        } else {
                            // prev & next are full
                            // if node.right!=null then node.next.left==null
                            // if node.left!=null then node.prev.right==null
                            if (node.right == null) {
                                attachHere = node;
                                attachFromLeft = false;
                                removeFromStart = false;
                            } else {
                                attachHere = nextNode;
                                attachFromLeft = true;
                                removeFromStart = false;
                            }
                        }
                    }
                }
                K movedKey;
                V movedValue;
                if (removeFromStart) {
                    // node.leftIndex == 0
                    movedKey = node.keys[0];
                    movedValue = node.values[0];
                    int resMunus1 = result - 1;
                    System.arraycopy(node.keys, 1, node.keys, 0, resMunus1);
                    System.arraycopy(node.values, 1, node.values, 0, resMunus1);
                    node.keys[resMunus1] = key;
                    node.values[resMunus1] = value;
                } else {
                    // node.rightIndex == Node.NODE_SIZE - 1
                    movedKey = node.keys[Node.NODE_SIZE - 1];
                    movedValue = node.values[Node.NODE_SIZE - 1];
                    System.arraycopy(node.keys, result, node.keys, result + 1, Node.NODE_SIZE - 1 - result);
                    System.arraycopy(node.values, result, node.values, result + 1, Node.NODE_SIZE - 1 - result);
                    node.keys[result] = key;
                    node.values[result] = value;
                }
                if (attachHere == null) {
                    if (removeFromStart) {
                        appendFromRight(previous, movedKey, movedValue);
                    } else {
                        appendFromLeft(nextNode, movedKey, movedValue);
                    }
                } else {
                    Node<K,V> newNode = createNode(movedKey, movedValue);
                    if (attachFromLeft) {
                        attachToLeft(attachHere, newNode);
                    } else {
                        attachToRight(attachHere, newNode);
                    }
                    balance(newNode);
                }
            }
        }
        return null;
    }

    private void appendFromLeft(Node<K,V> node, K key, V value) {
        if (node.leftIndex == 0) {
            int newRight = node.rightIndex + 1;
            System.arraycopy(node.keys, 0, node.keys, 1, newRight);
            System.arraycopy(node.values, 0, node.values, 1, newRight);
            node.rightIndex = newRight;
        } else {
            node.leftIndex--;
        }
        node.size++;
        node.keys[node.leftIndex] = key;
        node.values[node.leftIndex] = value;
    }

    private void attachToLeft(Node<K,V> node, Node<K,V> newNode) {
        newNode.parent = node;
        // node.left == null, so attach newNode here
        node.left = newNode;
        Node<K,V> predecessor = node.prev;
        newNode.prev = predecessor;
        newNode.next = node;
        if (predecessor != null) {
            predecessor.next = newNode;
        }
        node.prev = newNode;
    }

    /**
     * Add key / value pair into node.
     * Existing free room in the node should be checked before calling
     * this method.
     */
    private void appendFromRight(Node<K,V> node, K key, V value) {
        if (node.rightIndex == Node.NODE_SIZE - 1) {
            int leftIndex = node.leftIndex;
            int leftIndexMinus1 = leftIndex - 1;
            System.arraycopy(node.keys, leftIndex, node.keys, leftIndexMinus1, Node.NODE_SIZE - leftIndex);
            System.arraycopy(node.values, leftIndex, node.values, leftIndexMinus1, Node.NODE_SIZE - leftIndex);
            node.leftIndex = leftIndexMinus1;
        } else {
            node.rightIndex++;
        }
        node.size++;
        node.keys[node.rightIndex] = key;
        node.values[node.rightIndex] = value;
    }

    private void attachToRight(Node<K,V> node, Node<K,V> newNode) {
        newNode.parent = node;
        // - node.right==null - attach here
        node.right = newNode;
        newNode.prev = node;
        Node<K,V> successor = node.next;
        newNode.next = successor;
        if (successor != null) {
            successor.prev = newNode;
        }
        node.next = newNode;
    }

    private Node<K,V> createNode(K key, V value) {
        Node<K,V> node = new Node<K,V>();
        node.keys[0] = key;
        node.values[0] = value;
        node.leftIndex = 0;
        node.rightIndex = 0;
        node.size = 1;
        return node;
    }

    private void balance(Node<K,V> node) {
        Node<K,V> y;
        node.color = true;
        while (node != root && node.parent.color) {
            if (node.parent == node.parent.parent.left) {
                y = node.parent.parent.right;
                if (y != null && y.color) {
                    node.parent.color = false;
                    y.color = false;
                    node.parent.parent.color = true;
                    node = node.parent.parent;
                } else {
                    if (node == node.parent.right) {
                        node = node.parent;
                        leftRotate(node);
                    }
                    node.parent.color = false;
                    node.parent.parent.color = true;
                    rightRotate(node.parent.parent);
                }
            } else {
                y = node.parent.parent.left;
                if (y != null && y.color) {
                    node.parent.color = false;
                    y.color = false;
                    node.parent.parent.color = true;
                    node = node.parent.parent;
                } else {
                    if (node == node.parent.left) {
                        node = node.parent;
                        rightRotate(node);
                    }
                    node.parent.color = false;
                    node.parent.parent.color = true;
                    leftRotate(node.parent.parent);
                }
            }
        }
        root.color = false;
    }

    private void rightRotate(Node<K,V> node) {
        Node<K,V> n = node.left;
        node.left = n.right;
        if (n.right != null) {
            n.right.parent = node;
        }
        n.parent = node.parent;
        if (node.parent == null) {
            root = n;
        } else {
            if (node == node.parent.right) {
                node.parent.right = n;
            } else {
                node.parent.left = n;
            }
        }
        n.right = node;
        node.parent = n;
    }

    private void leftRotate(Node<K,V> node) {
        Node<K,V> n = node.right;
        node.right = n.left;
        if (n.left != null) {
            n.left.parent = node;
        }
        n.parent = node.parent;
        if (node.parent == null) {
            root = n;
        } else {
            if (node == node.parent.left) {
                node.parent.left = n;
            } else {
                node.parent.right = n;
            }
        }
        n.left = node;
        node.parent = n;
    }

    @Override
    public void putAll(Map<? extends K, ? extends V> map) {
        int mapSize = map.size();
        if (size==0 && mapSize!=0 && map instanceof SortedMap) {
            Comparator c = ((SortedMap)map).comparator();
            if (c == comparator || (c != null && c.equals(comparator))) {
                ++modCount;
                try {
                    buildFromSorted(mapSize,map.entrySet().iterator(),null,null);
                } catch (java.io.IOException cannotHappen) {
                } catch (ClassNotFoundException cannotHappen) {
                }
                return;
            }
        }
        super.putAll(map);
    }

    @Override
    public V remove(Object key) {
        if (size == 0) {
            return null;
        }
        Comparable<K> object = comparator == null ? toComparable((K) key) : null;
        K keyK = (K) key;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            int result = cmp(object, keyK, keys[leftIndex]);
            if (result < 0) {
                node = node.left;
            } else if (result == 0) {
                V value = node.values[leftIndex];
                removeLeftmost(node);
                return value;
            } else {
                int rightIndex = node.rightIndex;
                if (leftIndex != rightIndex) {
                    result = cmp(object, keyK, keys[rightIndex]);
                }
                if (result > 0) {
                    node = node.right;
                } else if (result == 0) {
                    V value = node.values[rightIndex];
                    removeRightmost(node);
                    return value;
                } else { /*search in node*/
                    int low = leftIndex + 1, mid = 0, high = rightIndex - 1;
                    while (low <= high) {
                        mid = (low + high) >> 1;
                        result = cmp(object, keyK, keys[mid]);
                        if (result > 0) {
                            low = mid + 1;
                        } else if (result == 0) {
                            V value = node.values[mid];
                            removeMiddleElement(node, mid);
                            return value;
                        } else {
                            high = mid - 1;
                        }
                    }
                    return null;
                }
            }
        }
        return null;
    }

    private void removeLeftmost(Node<K,V> node) {
        int index = node.leftIndex;
        if (node.size == 1) {
            deleteNode(node);
        } else if (node.prev != null && (Node.NODE_SIZE - 1 - node.prev.rightIndex) > node.size) {
            // move all to prev node and kill it
            Node<K,V> prev = node.prev;
            int tmpSize = node.rightIndex - index;
            System.arraycopy(node.keys, index + 1, prev.keys, prev.rightIndex + 1, tmpSize);
            System.arraycopy(node.values, index + 1, prev.values, prev.rightIndex + 1, tmpSize);
            prev.rightIndex += tmpSize;
            prev.size += tmpSize;
            deleteNode(node);
        } else if (node.next != null && (node.next.leftIndex) > node.size) {
            // move all to next node and kill it
            Node<K,V> next = node.next;
            int tmpSize = node.rightIndex - index;
            int nextNewLeft = next.leftIndex - tmpSize;
            next.leftIndex = nextNewLeft;
            System.arraycopy(node.keys, index + 1, next.keys, nextNewLeft, tmpSize);
            System.arraycopy(node.values, index + 1, next.values, nextNewLeft, tmpSize);
            next.size += tmpSize;
            deleteNode(node);
        } else {
            node.keys[index] = null;
            node.values[index] = null;
            node.leftIndex++;
            node.size--;
            Node<K,V> prev = node.prev;
            if (prev != null && prev.size == 1) {
                node.size++;
                node.leftIndex--;
                node.keys[node.leftIndex] = prev.keys[prev.leftIndex];
                node.values[node.leftIndex] = prev.values[prev.leftIndex];
                deleteNode(prev);
            }
        }
        modCount++;
        size--;
    }

    private void removeRightmost(Node<K,V> node) {
        int index = node.rightIndex;
        if (node.size == 1) {
            deleteNode(node);
        } else if (node.prev != null && (Node.NODE_SIZE - 1 - node.prev.rightIndex) > node.size) {
            // move all to prev node and kill it
            Node<K,V> prev = node.prev;
            int leftIndex = node.leftIndex;
            int tmpSize = index - leftIndex;
            System.arraycopy(node.keys, leftIndex, prev.keys, prev.rightIndex + 1, tmpSize);
            System.arraycopy(node.values, leftIndex, prev.values, prev.rightIndex + 1, tmpSize);
            prev.rightIndex += tmpSize;
            prev.size += tmpSize;
            deleteNode(node);
        } else if (node.next != null && (node.next.leftIndex) > node.size) {
            // move all to next node and kill it
            Node<K,V> next = node.next;
            int leftIndex = node.leftIndex;
            int tmpSize = index - leftIndex;
            int nextNewLeft = next.leftIndex - tmpSize;
            next.leftIndex = nextNewLeft;
            System.arraycopy(node.keys, leftIndex, next.keys, nextNewLeft, tmpSize);
            System.arraycopy(node.values, leftIndex, next.values, nextNewLeft, tmpSize);
            next.size += tmpSize;
            deleteNode(node);
        } else {
            node.keys[index] = null;
            node.values[index] = null;
            node.rightIndex--;
            node.size--;
            Node<K,V> next = node.next;
            if (next != null && next.size == 1) {
                node.size++;
                node.rightIndex++;
                node.keys[node.rightIndex] = next.keys[next.leftIndex];
                node.values[node.rightIndex] = next.values[next.leftIndex];
                deleteNode(next);
            }
        }
        modCount++;
        size--;
    }

    private void removeMiddleElement(Node<K,V> node, int index) {
        // this function is called iff index if some middle element;
        // so node.leftIndex < index < node.rightIndex
        // condition above assume that node.size > 1
        if (node.prev != null && (Node.NODE_SIZE - 1 - node.prev.rightIndex) > node.size) {
            // move all to prev node and kill it
            Node<K,V> prev = node.prev;
            int leftIndex = node.leftIndex;
            int tmpSize = index - leftIndex;
            System.arraycopy(node.keys, leftIndex, prev.keys, prev.rightIndex + 1, tmpSize);
            System.arraycopy(node.values, leftIndex, prev.values, prev.rightIndex + 1, tmpSize);
            prev.rightIndex += tmpSize;
            tmpSize = node.rightIndex - index;
            System.arraycopy(node.keys, index + 1, prev.keys, prev.rightIndex + 1, tmpSize);
            System.arraycopy(node.values, index + 1, prev.values, prev.rightIndex + 1, tmpSize);
            prev.rightIndex += tmpSize;
            prev.size += (node.size - 1);
            deleteNode(node);
        } else if (node.next != null && (node.next.leftIndex) > node.size) {
            // move all to next node and kill it
            Node<K,V> next = node.next;
            int leftIndex = node.leftIndex;
            int nextNewLeft = next.leftIndex - node.size + 1;
            next.leftIndex = nextNewLeft;
            int tmpSize = index - leftIndex;
            System.arraycopy(node.keys, leftIndex, next.keys, nextNewLeft, tmpSize);
            System.arraycopy(node.values, leftIndex, next.values, nextNewLeft, tmpSize);
            nextNewLeft += tmpSize;
            tmpSize = node.rightIndex - index;
            System.arraycopy(node.keys, index + 1, next.keys, nextNewLeft, tmpSize);
            System.arraycopy(node.values, index + 1, next.values, nextNewLeft, tmpSize);
            next.size += (node.size - 1);
            deleteNode(node);
        } else {
            int moveFromRight = node.rightIndex - index;
            int leftIndex = node.leftIndex;
            int moveFromLeft = index - leftIndex;
            if (moveFromRight <= moveFromLeft) {
                System.arraycopy(node.keys, index + 1, node.keys, index, moveFromRight);
                System.arraycopy(node.values, index + 1, node.values, index, moveFromRight);
                Node<K,V> next = node.next;
                if (next != null && next.size == 1) {
                    node.keys[node.rightIndex] = next.keys[next.leftIndex];
                    node.values[node.rightIndex] = next.values[next.leftIndex];
                    deleteNode(next);
                } else {
                    node.keys[node.rightIndex] = null;
                    node.values[node.rightIndex] = null;
                    node.rightIndex--;
                    node.size--;
                }
            } else {
                System.arraycopy(node.keys, leftIndex, node.keys, leftIndex + 1, moveFromLeft);
                System.arraycopy(node.values, leftIndex, node.values, leftIndex + 1, moveFromLeft);
                Node<K,V> prev = node.prev;
                if (prev != null && prev.size == 1) {
                    node.keys[leftIndex] = prev.keys[prev.leftIndex];
                    node.values[leftIndex] = prev.values[prev.leftIndex];
                    deleteNode(prev);
                } else {
                    node.keys[leftIndex] = null;
                    node.values[leftIndex] = null;
                    node.leftIndex++;
                    node.size--;
                }
            }
        }
        modCount++;
        size--;
    }

    private void removeFromIterator(Node<K,V> node, int index) {
        if (node.size == 1) {
            // it is safe to delete the whole node here.
            // iterator already moved to the next node;
            deleteNode(node);
        } else {
            int leftIndex = node.leftIndex;
            if (index == leftIndex) {
                Node<K,V> prev = node.prev;
                if (prev != null && prev.size == 1) {
                    node.keys[leftIndex] = prev.keys[prev.leftIndex];
                    node.values[leftIndex] = prev.values[prev.leftIndex];
                    deleteNode(prev);
                } else {
                    node.keys[leftIndex] = null;
                    node.values[leftIndex] = null;
                    node.leftIndex++;
                    node.size--;
                }
            } else if (index == node.rightIndex) {
                node.keys[index] = null;
                node.values[index] = null;
                node.rightIndex--;
                node.size--;
            } else {
                int moveFromRight = node.rightIndex - index;
                int moveFromLeft = index - leftIndex;
                if (moveFromRight <= moveFromLeft) {
                    System.arraycopy(node.keys, index + 1, node.keys, index, moveFromRight);
                    System.arraycopy(node.values, index + 1, node.values, index, moveFromRight);
                    node.keys[node.rightIndex] = null;
                    node.values[node.rightIndex] = null;
                    node.rightIndex--;
                    node.size--;
                } else {
                    System.arraycopy(node.keys, leftIndex, node.keys, leftIndex + 1, moveFromLeft);
                    System.arraycopy(node.values, leftIndex, node.values, leftIndex + 1, moveFromLeft);
                    node.keys[leftIndex] = null;
                    node.values[leftIndex] = null;
                    node.leftIndex++;
                    node.size--;
                }
            }
        }
        modCount++;
        size--;
    }

    private void deleteNode(Node<K,V> node) {
        if (node.right == null) {
            if (node.left != null) {
                attachToParent(node, node.left);
            } else {
                attachNullToParent(node);
            }
            fixNextChain(node);
        } else if (node.left == null) { // node.right != null
            attachToParent(node, node.right);
            fixNextChain(node);
        } else {
            // Here node.left!=null && node.right!=null
            // node.next should replace node in tree
            // node.next!=null by tree logic.
            // node.next.left==null by tree logic.
            // node.next.right may be null or non-null
            Node<K,V> toMoveUp = node.next;
            fixNextChain(node);
            if (toMoveUp.right == null) {
                attachNullToParent(toMoveUp);
            } else {
                attachToParent(toMoveUp, toMoveUp.right);
            }
            // Here toMoveUp is ready to replace node
            toMoveUp.left = node.left;
            if (node.left != null) {
                node.left.parent = toMoveUp;
            }
            toMoveUp.right = node.right;
            if (node.right != null) {
                node.right.parent = toMoveUp;
            }
            attachToParentNoFixup(node, toMoveUp);
            toMoveUp.color = node.color;
        }
    }

    private void attachToParentNoFixup(Node<K,V> toDelete, Node<K,V> toConnect) {
        // assert toConnect!=null
        Node<K,V> parent = toDelete.parent;
        toConnect.parent = parent;
        if (parent == null) {
            root = toConnect;
        } else if (toDelete == parent.left) {
            parent.left = toConnect;
        } else {
            parent.right = toConnect;
        }
    }

    private void attachToParent(Node<K,V> toDelete, Node<K,V> toConnect) {
        // assert toConnect!=null
        attachToParentNoFixup(toDelete, toConnect);
        if (!toDelete.color) {
            fixup(toConnect);
        }
    }

    private void attachNullToParent(Node<K,V> toDelete) {
        Node<K,V> parent = toDelete.parent;
        if (parent == null) {
            root = null;
        } else {
            if (toDelete == parent.left) {
                parent.left = null;
            } else {
                parent.right = null;
            }
            if (!toDelete.color) {
                fixup(parent);
            }
        }
    }

    private void fixNextChain(Node<K,V> node) {
        if (node.prev != null) {
            node.prev.next = node.next;
        }
        if (node.next != null) {
            node.next.prev = node.prev;
        }
    }

    private void fixup(Node<K,V> node) {
        Node<K,V> n;
        while (node != root && !node.color) {
            if (node == node.parent.left) {
                n = node.parent.right;
                if (n == null) {
                    node = node.parent;
                    continue;
                }
                if (n.color) {
                    n.color = false;
                    node.parent.color = true;
                    leftRotate(node.parent);
                    n = node.parent.right;
                    if (n == null) {
                        node = node.parent;
                        continue;
                    }
                }
                if ((n.left == null || !n.left.color) && (n.right == null || !n.right.color)) {
                    n.color = true;
                    node = node.parent;
                } else {
                    if (n.right == null || !n.right.color) {
                        n.left.color = false;
                        n.color = true;
                        rightRotate(n);
                        n = node.parent.right;
                    }
                    n.color = node.parent.color;
                    node.parent.color = false;
                    n.right.color = false;
                    leftRotate(node.parent);
                    node = root;
                }
            } else {
                n = node.parent.left;
                if (n == null) {
                    node = node.parent;
                    continue;
                }
                if (n.color) {
                    n.color = false;
                    node.parent.color = true;
                    rightRotate(node.parent);
                    n = node.parent.left;
                    if (n == null) {
                        node = node.parent;
                        continue;
                    }
                }
                if ((n.left == null || !n.left.color) && (n.right == null || !n.right.color)) {
                    n.color = true;
                    node = node.parent;
                } else {
                    if (n.left == null || !n.left.color) {
                        n.right.color = false;
                        n.color = true;
                        leftRotate(n);
                        n = node.parent.left;
                    }
                    n.color = node.parent.color;
                    node.parent.color = false;
                    n.left.color = false;
                    rightRotate(node.parent);
                    node = root;
                }
            }
        }
        node.color = false;
    }

    @Override
    public int size() {
        return size;
    }

    public SortedMap<K,V> subMap(K fromKey, K toKey) {
        SortedMap<K,V> sortedMap = subMap(fromKey, true, toKey, false);
        return sortedMap;
    }

    public NavigableMap<K,V> subMap(K fromKey, boolean fromInclusive,
                                    K toKey, boolean toInclusive) {
        NavigableMap<K,V> nm = new AscendingSubMap<K,V>(this,
                                                        false, fromKey, fromInclusive,
                                                        false, toKey,   toInclusive);
        return nm;
    }

    public SortedMap<K,V> tailMap(K fromKey) {
        SortedMap<K,V> sm = tailMap(fromKey, true);
        return sm;
    }

    public NavigableMap<K,V> tailMap(K fromKey, boolean inclusive) {
        NavigableMap<K,V> nm = new AscendingSubMap<K,V>(this,
                                                        false, fromKey, inclusive,
                                                        true,  null,    true);
        return nm;
    }

    @Override
    public Collection<V> values() {
        if (super.values == null) {
            super.values = new AbstractCollection<V>() {
                @Override
                public boolean contains(Object object) {
                    return TreeMap.this.containsValue(object);
                }

                @Override
                public int size() {
                    return TreeMap.this.size;
                }

                @Override
                public void clear() {
                    TreeMap.this.clear();
                }

                @Override
                public Iterator<V> iterator() {
                    return new UnboundedValueIterator<K, V>(TreeMap.this);
                }
            };
        }
        return super.values;
    }

    // NavigableMap API methods

    public Map.Entry <K,V> firstEntry() {
        Map.Entry<K,V> e = getFirstEntry();
        return exportEntry(e);
    }

    final Map.Entry <K,V> getFirstEntry() {
        if (root != null) {
            Node<K,V> node = minimum(root);
            return new Entry(node, node.leftIndex);
        }
        return null;
    }

    public Map.Entry <K,V> lastEntry() {
        Map.Entry<K,V> e = getLastEntry();
        return exportEntry(e);
    }

    final Map.Entry<K,V> getLastEntry() {
        if (root != null) {
            Node<K,V> node = maximum(root);
            return new Entry(node, node.rightIndex);
        }
        return null;
    }

    public Map.Entry<K,V> pollFirstEntry() {
        Map.Entry<K,V> e = getFirstEntry();
        Map.Entry<K,V> result = exportEntry(e);
        if (e != null)
            remove(e.getKey());
        return result;
    }

    public Map.Entry<K,V> pollLastEntry() {
        Map.Entry<K,V> e = getLastEntry();
        Map.Entry<K,V> result = exportEntry(e);
        if (e != null)
            remove(e.getKey());
        return result;
    }

    public Map.Entry<K,V> lowerEntry(K key) {
        Map.Entry<K,V> e = getLowerEntry(key);
        return exportEntry(e);
    }

    final Map.Entry<K,V> getLowerEntry(K key) {
        Comparable<K> object = comparator == null ? toComparable(key) : null;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int rightIndex = node.rightIndex;
            int result = cmp(object, key, keys[rightIndex]);
            if (result > 0) {
                // key > current node's largest key
                if (node.right != null) {
                    node = node.right;
                } else {
                    // current node's largest key is the greatest key less than key
                    return new Entry(node, rightIndex);
                }
            } else { //if (result < 0) {
                // key <= current node's largest key
                // check node's smallest key
                int leftIndex = node.leftIndex;
                result = cmp(object, key, keys[leftIndex]);
                if (result > 0) {
                    // key > current node's smallest key and
                    // key <= current node's largest key
                    // Find greatest key less than key in this node
                    for (int i = rightIndex; i >= leftIndex; i--) {
                        result = cmp(object, key, keys[i]);
                        if (result > 0) {
                            // found lower Entry
                            return new Entry(node, i);
                        }
                    }
                } else { //if (result < 0) {
                    // key <= current node's smallest key and
                    // key <= current node's largest key
                    if (node.left != null) {
                        node = node.left;
                    } else {
                        // traverse back up the tree
                        Node<K,V> parent = node.parent;
                        Node<K,V> child = node;
                        while (parent != null && child == parent.left) {
                            child = parent;
                            parent = parent.parent;
                        }
                        node = parent;
                        if (node != null) {
                            // Find greatest key less than key in this node
                            keys = node.keys;
                            leftIndex = node.leftIndex;
                            rightIndex = node.rightIndex;
                            for (int i = rightIndex; i >= leftIndex; i--) {
                                result = cmp(object, key, keys[i]);
                                if (result > 0) {
                                    // found lower Entry
                                    return new Entry(node, i);
                                }
                            }
                        }
                        // else, key not found, return null
                    }
                }
            }
        }
        // no greatest key found in tree less than or equal to given key
        return null;
    }

    public K lowerKey(K key) {
        Map.Entry<K,V> e = getLowerEntry(key);
        return keyOrNull(e);
    }

    public Map.Entry<K,V> floorEntry(K key) {
        Map.Entry<K,V> e = getFloorEntry(key);
        return exportEntry(e);
    }

    final Map.Entry<K,V> getFloorEntry(K key) {
        Comparable<K> object = comparator == null ? toComparable(key) : null;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int rightIndex = node.rightIndex;
            int result = cmp(object, key, keys[rightIndex]);
            if (result > 0) {
                // key > current node's largest key
                if (node.right != null) {
                    node = node.right;
                } else {
                    // current node's largest key is the greatest key less
                    // than or equal to key
                    return new Entry(node, rightIndex);
                }
            } else if (result < 0) {
                // key < current node's largest key
                // check node's smallest key
                int leftIndex = node.leftIndex;
                result = cmp(object, key, keys[leftIndex]);
                if (result > 0) {
                    // key > current node's smallest key and
                    // key < current node's largest key
                    // Find greatest key less than or equal to key in this node
                    for (int i = rightIndex; i >= leftIndex; i--) {
                        result = cmp(object, key, keys[i]);
                        if (result >= 0) {
                            // found floor Entry
                            return new Entry(node, i);
                        }
                    }
                } else if (result < 0) {
                    // key < current node's smallest key and
                    // key < current node's largest key
                    if (node.left != null) {
                        node = node.left;
                    } else {
                        // traverse back up the tree
                        Node<K,V> parent = node.parent;
                        Node<K,V> child = node;
                        while (parent != null && child == parent.left) {
                            child = parent;
                            parent = parent.parent;
                        }
                        node = parent;
                        if (node != null) {
                            // Find greatest key less than or equal to key in this node
                            keys = node.keys;
                            leftIndex = node.leftIndex;
                            rightIndex = node.rightIndex;
                            for (int i = rightIndex; i >= leftIndex; i--) {
                                result = cmp(object, key, keys[i]);
                                if (result >= 0) {
                                    // found floor Entry
                                    return new Entry(node, i);
                                }
                            }
                        }
                        // else, key not found, return null
                    }
                } else {
                    // key == current node's smallest key
                    return new Entry(node, leftIndex);
                }
            } else {
                // key == current node's largest key
                return new Entry(node, rightIndex);
            }
        }
        // no greatest key found in tree less than or equal to given key
        return null;
    }

    public K floorKey(K key) {
        Map.Entry<K,V> e = getFloorEntry(key);
        return keyOrNull(e);
    }

    public Map.Entry<K,V> ceilingEntry(K key) {
        Map.Entry<K,V> e = getCeilingEntry(key);
        return exportEntry(e);
    }

    final Map.Entry<K,V> getCeilingEntry(K key) {
        Comparable<K> object = comparator == null ? toComparable(key) : null;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            int result = cmp(object, key, keys[leftIndex]);
            if (result < 0) {
                // key < current node's smallest key
                if (node.left != null) {
                    node = node.left;
                } else {
                    // current node's smallest key is the least key greater
                    // than or equal to key
                    return new Entry(node, leftIndex);
                }
            } else if (result > 0) {
                // key > current node's smallest key
                // check node's largest key
                int rightIndex = node.rightIndex;
                result = cmp(object, key, keys[rightIndex]);
                if (result < 0) {
                    // key > current node's smallest key and
                    // key < current node's largest key
                    // Find least key greater than or equal to key in this node
                    for (int i = leftIndex; i <= rightIndex; i++) {
                        result = cmp(object, key, keys[i]);
                        if (result <= 0) {
                            // found ceiling Entry
                            return new Entry(node, i);
                        }
                    }
                } else if (result > 0) {
                    // key > current node's smallest key and
                    // key > current node's greatest key
                    if (node.right != null) {
                        node = node.right;
                    } else {
                        // traverse back up the tree
                        Node<K,V> parent = node.parent;
                        Node<K,V> child = node;
                        while (parent != null && child == parent.right) {
                            child = parent;
                            parent = parent.parent;
                        }
                        node = parent;
                        if (node != null) {
                            // Find least key greater than or equal to key in this node
                            leftIndex = node.leftIndex;
                            rightIndex = node.rightIndex;
                            keys = node.keys;
                            for (int i = leftIndex; i <= rightIndex; i++) {
                                result = cmp(object, key, keys[i]);
                                if (result <= 0) {
                                    // found ceiling Entry
                                    return new Entry(node, i);
                                }
                            }
                        }
                        // else, key not found, return null
                    }
                } else {
                    // key == current node's largest key
                    return new Entry(node, rightIndex);
                }
            } else {
                // key == current node's smallest key
                return new Entry(node, leftIndex);
            }
        }
        // no key found in tree greater than or equal to key
        return null;
    }

    public K ceilingKey(K key) {
        Map.Entry<K,V> e = getCeilingEntry(key);
        return keyOrNull(e);
    }

    public Map.Entry<K,V> higherEntry(K key) {
        Map.Entry<K,V> e = getHigherEntry(key);
        return exportEntry(e);
    }

    final Map.Entry<K,V> getHigherEntry(K key) {
        Comparable<K> object = comparator == null ? toComparable(key) : null;
        Node<K,V> node = root;
        while (node != null) {
            K[] keys = node.keys;
            int leftIndex = node.leftIndex;
            int result = cmp(object, key, keys[leftIndex]);
            if (result < 0) {
                // key < current node's smallest key
                if (node.left != null) {
                    node = node.left;
                } else {
                    // current node's smallest key is the least key greater
                    // than key
                    return new Entry(node, leftIndex);
                }
            } else {
                // key >= current node's smallest key
                // check node's largest key
                int rightIndex = node.rightIndex;
                result = cmp(object, key, keys[rightIndex]);
                if (result < 0) {
                    // key > current node's smallest key and
                    // key < current node's largest key
                    // Find least key greater than key in this node
                    for (int i = leftIndex; i <= rightIndex; i++) {
                        result = cmp(object, key, keys[i]);
                        if (result < 0) {
                            // found higher Entry
                            return new Entry(node, i);
                        }
                    }
                } else {
                    // key >= current node's smallest key and
                    // key >= current node's greatest key
                    if (node.right != null) {
                        node = node.right;
                    } else {
                        // traverse back up the tree
                        Node<K,V> parent = node.parent;
                        Node<K,V> child = node;
                        while (parent != null && child == parent.right) {
                            child = parent;
                            parent = parent.parent;
                        }
                        node = parent;
                        if (node != null) {
                            // Find least key greater than key in this node
                            keys = node.keys;
                            leftIndex = node.leftIndex;
                            rightIndex = node.rightIndex;
                            for (int i = leftIndex; i <= rightIndex; i++) {
                                result = cmp(object, key, keys[i]);
                                if (result < 0) {
                                    // found higher Entry
                                    return new Entry(node, i);
                                }
                            }
                        }
                        // else, key not found, return null
                    }
                }
            }
        }
        // no key found in tree greater than key
        return null;
    }

    public K higherKey(K key) {
        Map.Entry<K,V> e = getHigherEntry(key);
        return keyOrNull(e);
    }

    public NavigableSet<K> navigableKeySet() {
        KeySet<K> nks = navigableKeySet;
        return (nks != null) ? nks : (navigableKeySet = new KeySet(this));
    }

    public NavigableMap<K,V> descendingMap() {
        NavigableMap<K,V> km = descendingMap;
        return (km != null) ? km :
            (descendingMap = new DescendingSubMap<K,V>(this,
                                                       true, null, true,
                                                       true, null, true));
    }

    public NavigableSet<K> descendingKeySet() {
        NavigableSet<K> descendingKeySet = descendingMap().navigableKeySet();
        return descendingKeySet;
    }

    // utilities

    static <K,V> Map.Entry<K,V> exportEntry(Map.Entry<K,V> e) {
        return e == null ? null :
            new AbstractMap.SimpleImmutableEntry<K,V>(e);
    }

    static <K,V> K keyOrNull(Map.Entry<K,V> e) {
        return e == null ? null : e.getKey();
    }

    static <K> K key(Map.Entry<K,?> e) {
        if (e == null)
            throw new NoSuchElementException();
        return e.getKey();
    }

    final int compare(Object k1, Object k2) {
        return comparator==null ? ((Comparable<? super K>)k1).compareTo((K)k2)
            : comparator.compare((K)k1, (K)k2);
    }

    final static boolean valEquals(Object o1, Object o2) {
        return (o1==null ? o2==null : o1.equals(o2));
    }

    private void writeObject(ObjectOutputStream stream) throws IOException {
        stream.defaultWriteObject();
        stream.writeInt(size);
        if (size > 0) {
            Node<K,V> node = minimum(root);
            while (node != null) {
                int to = node.rightIndex;
                for (int i = node.leftIndex; i <= to; i++) {
                    stream.writeObject(node.keys[i]);
                    stream.writeObject(node.values[i]);
                }
                node = node.next;
            }
        }
    }

    private void readObject(ObjectInputStream stream) throws IOException,
            ClassNotFoundException {
        stream.defaultReadObject();
        int tmpSize = stream.readInt();
        buildFromSorted(tmpSize, null, stream, null);
    }

    /** Intended to be called only from TreeSet.readObject */
    void readTreeSet(int size, java.io.ObjectInputStream s, V defaultVal)
        throws java.io.IOException, ClassNotFoundException {
        buildFromSorted(size, null, s, defaultVal);
    }

    /** Intended to be called only from TreeSet.addAll */
    void addAllForTreeSet(SortedSet<? extends K> set, V defaultVal) {
        try {
            buildFromSorted(set.size(), set.iterator(), null, defaultVal);
        } catch (java.io.IOException cannotHappen) {
        } catch (ClassNotFoundException cannotHappen) {
        }
    }

    /**
     * Tree building algorithm from sorted data.  Can accept keys
     * and/or values from iterator or stream. This leads to too many
     * parameters, but seems better than alternatives.  The four formats
     * that this method accepts are:
     *
     *    1) An iterator of Map.Entries.  (it != null, defaultVal == null).
     *    2) An iterator of keys.         (it != null, defaultVal != null).
     *    3) A stream of alternating serialized keys and values.
     *                                   (it == null, defaultVal == null).
     *    4) A stream of serialized keys. (it == null, defaultVal != null).
     *
     * It is assumed that the comparator of the TreeMap is already set prior
     * to calling this method.
     *
     * @param size the number of keys (or key-value pairs) to be read from
     *        the iterator or stream
     * @param it If non-null, new entries are created from entries
     *        or keys read from this iterator.
     * @param str If non-null, new entries are created from keys and
     *        possibly values read from this stream in serialized form.
     *        Exactly one of it and str should be non-null.
     * @param defaultVal if non-null, this default value is used for
     *        each value in the map.  If null, each value is read from
     *        iterator or stream, as described above.
     * @throws IOException propagated from stream reads. This cannot
     *         occur if str is null.
     * @throws ClassNotFoundException propagated from readObject.
     *         This cannot occur if str is null.
     */
    private void buildFromSorted(int theSize, Iterator itr,
                                 java.io.ObjectInputStream stream,
                                 V defaultVal)
                           throws  java.io.IOException, ClassNotFoundException {
        Node<K,V> lastNode = null;
        for (int i = 0; i < theSize; i++) {
            // extract key and/or value from iterator or stream
            K key;
            V value;
            if (itr != null) {  // use iterator
                if (defaultVal==null) {
                    Map.Entry<K,V> entry = (Map.Entry<K,V>)itr.next();
                    key = entry.getKey();
                    value = entry.getValue();
                } else {
                    key = (K)itr.next();
                    value = defaultVal;
                }
            } else { // use stream
                key = (K) stream.readObject();
                value = (defaultVal != null ? defaultVal : (V) stream.readObject());
            }
            lastNode = addToLast(lastNode, key, value);
        }
    }
}
