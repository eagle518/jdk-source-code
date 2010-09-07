/*
 * @(#)OperaPreferences.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.lang.Object;
import java.lang.String;
import java.lang.StringBuffer;
import java.util.ArrayList;
import java.util.ConcurrentModificationException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.NoSuchElementException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


/**
 Provides support for the Opera web browser.  This class can be used to
 determine if Opera is installed, and if it is, insure that Opera is setup
 to handle the application/x-java-jnlp-file mime type.

 @author Michael W. Romanchuk
 @version 0.1, 05/22/03
 @since 1.5
 */
public class OperaPreferences extends Object
{
    /**
     Loads the preferences with the contents of the stream.

     @param istream  the stream to load preferences from.
     */
    public void load(InputStream istream) throws IOException
    {
        InputStreamReader ireader = new InputStreamReader(istream, OPERA_ENCODING);
        BufferedReader    in      = new BufferedReader(ireader, DEFAULT_SIZE);
        String            section = "";    // first section always unnamed

        for (String line = in.readLine(); line != null; /* unused */)
        {
            if (line.length() > 0)
            {
                if (line.charAt(0) == OPEN_BRACKET)
                {
                    // this is a new section; the name is between the [ ] brackets
                    section = line.substring(1, line.indexOf(CLOSE_BRACKET));
                }
                else
                {
                    String key   = null;
                    String value = null;
                    int    index = line.indexOf(SEPARATOR);

                    if (index >= 0)
                    {
                        key   = line.substring(0, index);
                        value = line.substring(index + 1);
                    }
                    else
                    {
                        // this key has no value
                        key = line;
                    }

                    put(section, key, value);
                }
            }
            // no else required; ignore blank lines

            line = in.readLine();
        }
    }

    /**
     Stores the preferences to the stream.

     @param ostream  the stream to store preferences in.
     */
    public void store(OutputStream ostream) throws IOException
    {
        OutputStreamWriter owriter = new OutputStreamWriter(ostream, OPERA_ENCODING);
        PrintWriter        out     = new PrintWriter(owriter, true);

        out.println(toString());
    }

    /**
     Checks if <code>this</code> Opera preferences contains the given section.

     @param section  the name of the section to check for.

     @return <code>true</code> if <code>this</code> Opera preferences contains
             the given section; <code>false</code> otherwise.
     */
    public boolean containsSection(String section)
    {
        return (indexOf(section) >= 0);
    }

    /**
     Checks if <code>this</code> Opera preferences contains the given key in
     the given section.

     @param section  the name of the section to check in.
     @param key      the name of the key to check for.

     @return <code>true</code> if <code>this</code> Opera preferences contains
             the given key in the given section; <code>false</code> otherwise.
     */
    public boolean containsKey(String section, String key)
    {
        int i = indexOf(section);

        return ((i < 0) ? false :
                          ((PreferenceSection) sections.get(i)).contains(key));
    }

    /**
     Gets the value of the given key in the given section.

     @param section  the name of the section containing <code>key</code>.
     @param key      the name of the key to get the value of.

     @return the value of the given key in the given section, or <code>null</code>
             if the key isn't found.
     */
    public String get(String section, String key)
    {
        int i = indexOf(section);

        PreferenceSection.PreferenceEntry entry = ((i < 0) ? null :
                                                             ((PreferenceSection) sections.get(i))
                                                                                          .get(key));

        return ((entry == null) ? null : entry.getValue());
    }

    /**
     Puts the value into the given key in the given section.

     @param section  the name of the section containing <code>key</code>.
     @param key      the name of the key to put the value in.
     @param value    the value to put.

     @return the previous value for the given key in the given section, or
             <code>null</code> if the key didn't previously exist.
     */
    public String put(String section, String key, String value)
    {
        int               i  = indexOf(section);
        PreferenceSection ps = null;

        if (i < 0)
        {
            // the section needs to be added first
            ps = new PreferenceSection(section);
            sections.add(ps);
        }
        else
        {
            ps = (PreferenceSection) sections.get(i);
        }

        return (ps.put(key, value));
    }

    /**
     Removes the given section from <code>this</code> Opera preferences.

     @param section  the name of the section to remove.

     @return the preference section that was removed, or <code>null</code> if
             no section with the given name exists.
     */
    public PreferenceSection remove(String section)
    {
        int i = indexOf(section);

        return ((i < 0) ? null : (PreferenceSection) sections.remove(i));
    }

    /**
     Removes the given key in the given section from <code>this</code> Opera
     preferences.

     @param section  the name of the section containing <code>key</code>.
     @param key      the name of the key to remove.

     @return the value associated with the key that was removed, or
             <code>null</code> if the section or key weren't found.
     */
    public String remove(String section, String key)
    {
        int i = indexOf(section);

        return ((i < 0) ? null :
                          ((PreferenceSection) sections.get(i)).remove(key));
    }

    /**
     Gets an iterator over the entries in the given section.  The entries are
     guaranteed to be in the same order they were added to the section.
     <br />
     The entries are of type <code>OperaPreferences.PreferenceSection.PreferenceEntry</code>.

     @param section  the name of the section to iterate over.

     @return an iterator over the entries in the given section.
     */
    public Iterator iterator(String section)
    {
        int i = indexOf(section);

        return ((i < 0) ? (new PreferenceSection(section).iterator()) :
                          ((PreferenceSection) sections.get(i)).iterator());
    }

    /**
     Gets an iterator over the sections associated with these preferences.  The
     entries are guaranteed to be in the same order they were added.
     <br />
     The entries are of type <code>String</code>.

     @return an iterator over the sections associated with these preferences.
     */
    public Iterator iterator()
    {
        return (new OperaPreferencesIterator());
    }

    /**
     Compares the specified object with <code>this</code> for equality.  Returns
     true if the given object is also an Opera preferences containing the same
     number of sections, with the same section names, and the same section
     contents.

     @param o  the object to test for equality with <code>this</code>
               entry.
     @return <code>true</code> if the two objects are equal;
             <code>false</code> otherwise.
     */
    public boolean equals(Object o)
    {
        boolean result = false;

        if (o instanceof OperaPreferences)
        {
            OperaPreferences rhs = (OperaPreferences) o;
            Iterator         i   = sections.listIterator();
            Iterator         j   = rhs.sections.listIterator();

            while (true)
            {
                if (i.hasNext() && j.hasNext())
                {
                    PreferenceSection s1 = (PreferenceSection) i.next();
                    PreferenceSection s2 = (PreferenceSection) j.next();

                    if (s1.equals(s2) == false)
                    {
                        // section mismatch; the sections aren't equal
                        // (result already false)
                        break;
                    }
                    // no else required; keep checking
                }
                else if ((i.hasNext() == false) &&
                         (j.hasNext() == false))
                {
                    // the two prefernces have the same number of sections and
                    // are equal
                    result = true;
                    break;
                }
                else
                {
                    // the two prefernces have a different number of sections
                    // (result already false)
                    break;
                }
            }
        }
        // no else required; not even same type

        return (result);
    }

    // inherits documentation from java.lang.Object class
    public int hashCode()
    {
        return (sections.hashCode());
    }

    // inherits documentation from java.lang.Object class
    public String toString()
    {
        StringBuffer buffer = new StringBuffer();

        for (Iterator i = sections.listIterator(); i.hasNext(); /* unused */)
        {
            PreferenceSection ps = (PreferenceSection) i.next();

            buffer.append(ps);
        }

        return (buffer.toString());
    }

    /**
     Construct a new <code>OperaPreferences</code>.
     */
    public OperaPreferences()
    {
        sections = new ArrayList(DEFAULT_SECTION_COUNT);
    }

    /**
     Finds the index of the given section.

     @param section  the name of the section to find.

     @return the index of the given section if it is in the list; otherwise
             <code>-1</code>.
     */
    private int indexOf(String section)
    {
        // An Opera preference file doesn't ever contain very many sections, so
        // this brute-force search isn't really that expensive.
        int count  =  0;
        int result = -1;

        for (Iterator i = sections.listIterator(); i.hasNext(); count++)
        {
            PreferenceSection ps = (PreferenceSection) i.next();

            if ((ps != null) && ps.getName().equalsIgnoreCase(section))
            {
                result = count;
                break;
            }
            // no else required; keep searching
        }

        return (result);
    }

    /**
     This class is used for storing the key/value pairs for an Opera
     preferences file, version 2.0.
     <br />
     Preference entries are stored in a <code>HashMap</code> for fast searching.
     However each entry is also part of a doubly linked list, since it is
     important to maintain the original order when storing the preferences.
     <br />
     Opera drops any duplicate keys after the first one in a section.  So,
     if a duplicate key is entered, it is not put into the section.  To
     change the value of a preference entry, first use {@link #get(String) get}
     for the key, then call {@link PreferenceEntry#setValue(String) setValue}
     on the result.
     */
    private class PreferenceSection extends Object
    {
        /**
         Gets the section's name.

         @return the section's name.
         */
        public String getName()
        {
            return (name);
        }

        /**
         Checks to see if <code>this</code> preference section contains the
         given key.

         @param key  the key to check for.

         @return <code>true</code> if <code>this</code> preference section
                 contains the key; <code>false</code> otherwise
         */
        public boolean contains(String key)
        {
            return (entries.containsKey(key));
        }

        /**
         Puts an entry into <code>this</code> preference section.
         <br />
         Preference entries are stored in a <code>HashMap</code> for fast
         searching.  However each entry is also part of a doubly linked list,
         since it is important to maintain the original order when storing
         the preferences.
         <br />
         Opera drops any duplicate keys after the first one in a section.  So,
         if a duplicate key is entered, it is not put into the section.  Instead
         the existing entry value is cahnged.

         @param key    the key for the new entry.
         @param value  the value of the entry to put in <code>this</code>
                       section.

         @return <code>null</code> if the key doesn't already exist in
                 <code>this</code> section.  Otherwise the existing entry is
                 changed, and the old value for the key is returned.
         */
        public String put(String key, String value)
        {
            PreferenceEntry entry  = (PreferenceEntry) entries.get(key);
            String          result = null;

            if (entry == null)
            {
                entry = new PreferenceEntry(key, value);
                if (end == null)
                {
                    // this is the first entry
                    start = entry;
                    end   = entry;
                }
                else
                {
                    end.add(entry);
                    end = entry;
                }

                entries.put(entry.getKey(), entry);
                modified++;
            }
            else
            {
                // Opera drops any duplicate keys after the first one in a
                // section, so instead update the current entry value and
                // return the previous value.
                result = entry.getValue();
                entry.setValue(value);
            }

            return (result);
        }

        /**
         Gets the entry for the given <code>key</code> associated with
         <code>this</code> preference section.

         @param key the key for the entry to get.

         @return the entry associated with the given <code>key</code> if it was
                 found in <code>this</code> section.  Otherwise it returns
                 <code>null</code>.
         */
        public PreferenceEntry get(String key)
        {
            return ((PreferenceEntry) entries.get(key));
        }

        /**
         Removes an entry from <code>this</code> preference section.

         @param key  the key for the entry to remove.

         @return the value previously associated with the <code>key</code> if
                 it was found in <code>this</code> section.  Otherwise it
                 returns <code>null</code>.
         */
        public String remove(String key)
        {
            PreferenceEntry  entry  = (PreferenceEntry) entries.get(key);
            String           result = null;

            if (entry != null)
            {
                result = entry.getValue();
                removeEntry(entry);
            }
            // no else required; key not stored

            return (result);
        }

        /**
         Gets an iterator over the entries in <code>this</code> preference
         section.  This iterator is guaranteed to return entries in the same
         order they were loaded in.
         <br />
         This iterator is <i>fail-fast</i>: if the preference section is
         structurally modified at any time after the iterator is created, in
         any way except through the iterator's own remove method, the iterator
         will throw a <code>ConcurrentModificationException</code>.

         @return an iterator over the entries in <code>this</code> preference
                 section.

         @see OperaPreferences#load(InputStream)
         @see PreferenceEntryIterator
         @see java.util.ConcurrentModificationException
         */
        public Iterator iterator()
        {
            return (new PreferenceEntryIterator(start));
        }

        /**
         Compares the specified object with <code>this</code> entry for
         equality.  Returns true if the given object is also a preference
         section and the two sections represent the same key/value pair entries.
         <br />
         More formally, two entries e1 and e2 represent the same key/value
         pair if:
         <pre>
             ((e1.getKey() == null) ?
                  e2.getKey() == null : e1.getKey().equals(e2.getKey())) &&
             ((e1.getValue() == null) ?
                  e2.getValue() == null : e1.getValue().equals(e2.getValue()))
         </pre>

         @param o  the object to test for equality with <code>this</code>
                   entry.
         @return <code>true</code> if the two objects are equal;
                 <code>false</code> otherwise.
         */
        public boolean equals(Object o)
        {
            boolean result = false;

            if (o instanceof PreferenceSection)
            {
                PreferenceSection rhs = (PreferenceSection) o;

                if ((name == rhs.name) ||
                    ((name != null) && (name.equals(rhs.name))))
                {
                    Iterator i = iterator();
                    Iterator j = rhs.iterator();

                    while (true)
                    {
                        if (i.hasNext() && j.hasNext())
                        {
                            PreferenceEntry e1 = (PreferenceEntry) i.next();
                            PreferenceEntry e2 = (PreferenceEntry) j.next();

                            if (e1.equals(e2) == false)
                            {
                                // entry mismatch; the sections aren't equal
                                // (result already false)
                                break;
                            }
                            // no else required; keep checking
                        }
                        else if ((i.hasNext() == false) &&
                                 (j.hasNext() == false))
                        {
                            // the two sections have the same number of entries
                            // and are equal
                            result = true;
                            break;
                        }
                        else
                        {
                            // the two sections have a different number of
                            // entries (result already false)
                            break;
                        }
                    }
                }
                // no else required; different names
            }
            // no else required; not even same type

            return (result);
        }

        // inherits documentation from java.lang.Object class
        public int hashCode()
        {
            return (entries.hashCode());
        }

        // inherits documentation from java.lang.Object class
        public String toString()
        {
            StringBuffer buffer = new StringBuffer(entries.size() * 80);

            if ((name != null) && (name.length() > 0))
            {
                buffer.append(OPEN_BRACKET)
                      .append(name)
                      .append(CLOSE_BRACKET)
                      .append(System.getProperty("line.separator"));
            }
            // no else required; nothing to print

            for (Iterator i = iterator(); i.hasNext(); /* blank */ )
            {
                PreferenceEntry entry = (PreferenceEntry) i.next();

                buffer.append(entry).append(System.getProperty("line.separator"));
            }

            buffer.append(System.getProperty("line.separator"));

            return (buffer.toString());
        }

        /**
         Construct a new <code>PreferenceSection</code>.

         @param name the the name of the preference section.
         */
        public PreferenceSection(String name)
        {
            this.name     = name;
            this.entries  = new HashMap();
            this.modified = 0;
            this.start    = null;
            this.end      = null;
        }

        /**
         Removes an entry from the preference section, while maintaining the
         status of any modifications to the section.  This is used to provide
         fail-fast support for the iterator.

         @param the entry to remove.
         */
        private void removeEntry(PreferenceEntry entry)
        {
            if (entry == start)
            {
                start = entry.getNext();
            }
            // no else required

            if (entry == end)
            {
                end = entry.getPrevious();
            }
            // no else required

            entry.remove();
            entries.remove(entry.getKey());
            modified++;
        }

        /**
         This class is used to store key/value pairs for a preference section.
         <br />
         Preference entries are stored in a <code>HashMap</code> for fast
         searching.  However each entry is also part of a doubly linked list,
         since it is important to maintain the original order when storing the
         preferences.  The <code>HashMap</code> containing the entries is
         maintained by the <code>PreferenceSection</code> class, but
         <code>this</code> class maintains the doubly linked list.
         */
        private class PreferenceEntry extends Object
        {
            /**
             Gets the key associated with <code>this</code> entry.

             @return the key associated with <code>this</code> entry.
             */
            public String getKey()
            {
                return (key);
            }

            /**
             Gets the value associated with <code>this</code> entry.

             @return the value associated with <code>this</code> entry.
             */
            public String getValue()
            {
                return (value);
            }

            /**
             Sets the value associated with <code>this</code> entry.

             @param value the value to set.
             */
            public void setValue(String value)
            {
                this.value = value;
            }

            /**
             Adds the given <code>entry</code> to the doubly linked list after
             <code>this</code> entry.
             <br />
             If <code>this</code> entry isn't at the end of the list, the list
             is traversed to ensure that all adds are only made to the end.

             @param entry  the entry to add.
             */
            public void add(PreferenceEntry entry)
            {
                if (next != null)
                {
                    // only add to the end
                    next.add(entry);
                }
                else
                {
                    next           = entry;
                    entry.previous = this;
                }
            }

            /**
             Removes <code>this</code> entry from the doubly linked list.
             */
            public void remove()
            {
                if (previous != null)
                {
                    previous.next = next;
                }
                // no else required

                if (next != null)
                {
                    next.previous = previous;
                }
                // no else required

                this.previous = null;
                this.next     = null;
            }

            /**
             Gets the preference entry that is previous to <code>this</code>
             entry in the doubly linked list.

             @return the entry previous to <code>this</code> one.
             */
            public PreferenceEntry getPrevious()
            {
                return (previous);
            }

            /**
             Gets the preference entry that is next (after) to <code>this</code>
             entry in the doubly linked list.

             @return the entry next to <code>this</code> one.
             */
            public PreferenceEntry getNext()
            {
                return (next);
            }

            /**
             Compares the specified object with <code>this</code> entry for
             equality.  Returns true if the given object is also an map entry
             and the two entries represent the same key/value pair.
             <br />
             More formally, two entries e1 and e2 represent the same key/value
             pair if:
             <pre>
                 ((e1.getKey() == null) ?
                      e2.getKey() == null : e1.getKey().equals(e2.getKey())) &&
                 ((e1.getValue() == null) ?
                      e2.getValue() == null : e1.getValue().equals(e2.getValue()))
             </pre>

             @param o  the object to test for equality with <code>this</code>
                       entry.
             @return <code>true</code> if the two objects are equal;
                     <code>false</code> otherwise.
             */
            public boolean equals(Object o)
            {
                boolean result = false;

                if (o instanceof PreferenceEntry)
                {
                    PreferenceEntry rhs = (PreferenceEntry) o;
                    String          k1  = getKey();
                    String          k2  = rhs.getKey();

                    if ((k1 == k2) ||
                        ((k1 != null) && (k1.equals(k2))))
                    {
                        String v1 = getValue();
                        String v2 = rhs.getValue();

                        if ((v1 == v2) ||
                            ((v1 != null) && (v1.equals(v2))))
                        {
                            result = true;
                        }
                        // no else required; different values
                    }
                    // no else required; different keys
                }
                // no else required; not even same type

                return (result);
            }

            // inherits documentation from java.lang.Object class
            public int hashCode()
            {
                return ((key == null) ? 0 : key.hashCode());
            }

            // inherits documentation from java.lang.Object class
            public String toString()
            {
                StringBuffer buffer = new StringBuffer(((key == null) ? 0 : key.length()) +
                                                       ((value == null) ? 0 : value.length()) + 1);

                if ((key != null) && (value != null))
                {
                    // only put the separator in the returned string if there
                    // is both a key and a value
                    buffer.append(key).append(SEPARATOR).append(value);
                }
                else if (key != null)
                {
                    buffer.append(key);
                }
                else if (value != null)
                {
                    buffer.append(value);
                }
                // no else required; return the empty string

                return (buffer.toString());
            }

            /**
             Constructs a <code>PreferenceEntry</code> for the given key/value
             pair.
             */
            public PreferenceEntry(String key, String value)
            {
                this.key      = key;
                this.value    = value;
                this.previous = null;
                this.next     = null;
            }

            /** The key for this entry */
            private final String key;

            /** The value for this entry */
            private String value;

            /**
             The previous entry in the doubly linked list or <code>null</code>.
             */
            private PreferenceEntry previous;

            /**
             The next entry in the doubly linked list or <code>null</code>.
             */
            private PreferenceEntry next;
        }

        /**
         This class provides an iterator for traversing the entries of a
         preference section in the order they were loaded.
         <br />
         This iterator is <i>fail-fast</i>: if the preference section is
         structurally modified at any time after the iterator is created, in
         any way except through the iterator's own remove method, the iterator
         will throw a <code>ConcurrentModificationException</code>.  Thus, in
         the face of concurrent modification, the iterator fails quickly and
         cleanly, rather than risking arbitrary, non-deterministic behavior at
         an undetermined time in the future.
         <br />
         Note that the fail-fast behavior of an iterator cannot be guaranteed
         as it is, generally speaking, impossible to make any hard guarantees
         in the presence of unsynchronized concurrent modification. Fail-fast
         iterators throw <code>ConcurrentModificationException</code> on a
         best-effort basis. Therefore, it would be wrong to write a program
         that depended on this exception for its correctness: <i>the fail-fast
         behavior of iterators should be used only to detect bugs</i>.

         @see OperaPreferences#load(InputStream)
         @see PreferenceSection#iterator()
         @see java.util.ConcurrentModificationException
         */
        private class PreferenceEntryIterator implements Iterator
        {
            // inherits documentation from java.util.Iterator interface
            public boolean hasNext()
            {
                return (next != null);
            }

            // inherits documentation from java.util.Iterator interface
            public Object next()
            {
                if (modified != expectedModified)
                {
                    throw (new ConcurrentModificationException());
                }
                else if (next == null)
                {
                    throw (new NoSuchElementException());
                }
                // no else required; pre-conditions met

                current = next;
                next    = next.getNext();

                return (current);
            }

            // inherits documentation from java.util.Iterator interface
            public void remove()
            {
                if (current == null)
                {
                    throw (new IllegalStateException());
                }
                else if (modified != expectedModified)
                {
                    throw (new ConcurrentModificationException());
                }
                // no else required; pre-conditions met

                removeEntry(current);
                current          = null;
                expectedModified = modified;
            }

            /**
             Constructs a <code>PreferenceEntryIterator</code> that will iterate
             forward through the entries starting from the given entry.
             */
            public PreferenceEntryIterator(PreferenceEntry start)
            {
                next             = start;
                current          = null;
                expectedModified = modified;
            }

            /** The next entry if their is one. */
            private PreferenceEntry next;

            /**
             The current entry if one has been traversed by {@link #getNext() getNext}.
             */
            private PreferenceEntry current;

            /**
             The number of times <code>this</code> iterator expects the
             <code>PreferenceSection</code> it is iterating over has been
             structurally modified.
             <br />
             Modification using <code>this</code> iterators remove method are
             allowed.  However, if any modification to the preference section
             are made outside the iterators methods a
             <code>ConcurrentModificationException</code> is thrown.
             */
            private int expectedModified;
        }

        /**
         The name associated with <code>this</code> <code>PreferenceSection</code>.
         */
        private String name;

        /**
         A <code>HashMap</code> containing the entries for <code>this</code>
         <code>PreferenceSection</code>.
         <br />
         Entries are also stored as part of a doubly linked list.  This
         structure allows both fast searching for a known key, and the ability
         to maintain the original order of preference section entries.
         */
        private HashMap entries;

        /**
         The number of times <code>this</code> <code>PreferenceSection</code>
         has been structurally modified.  Structural modifications are those
         that change the number of entries in the <code>PreferenceSection</code>.
         <br />
         This field is used to make iterators fail-fast.

         @see java.util.ConcurrentModificationException
        */
        private volatile int modified;

        /**
         A start of the preference entries for <code>this</code>
         <code>PreferenceSection</code>.
         <br />
         Entries are also stored in both a <code>HashMap</code>, and as part of
         a doubly linked list.  This structure allows both fast searching for a
         known key, and the ability to maintain the original order of preference
         section entries.
         */
        private PreferenceEntry start;

        /**
         A end of the preference entries for <code>this</code>
         <code>PreferenceSection</code>.
         <br />
         Entries are also stored in both a <code>HashMap</code>, and as part of
         a doubly linked list.  This structure allows both fast searching for a
         known key, and the ability to maintain the original order of preference
         section entries.
         */
        private PreferenceEntry end;
    }

    /**
     This class provides an iterator for traversing the sections of an
     <code>OperaPreferences</code> in the order they were loaded.
     <br />
     This iterator is <i>fail-fast</i>: if the preferences are structurally
     modified at any time after the iterator is created, in any way except
     through the iterator's own remove method, the iterator will throw a
     <code>ConcurrentModificationException</code>.  Thus, in the face of
     concurrent modification, the iterator fails quickly and cleanly, rather
     than risking arbitrary, non-deterministic behavior at an undetermined time
     in the future.
     <br />
     Note that the fail-fast behavior of an iterator cannot be guaranteed as it
     is, generally speaking, impossible to make any hard guarantees in the
     presence of unsynchronized concurrent modification. Fail-fast iterators
     throw <code>ConcurrentModificationException</code> on a best-effort basis.
     Therefore, it would be wrong to write a program that depended on this
     exception for its correctness: <i>the fail-fast behavior of iterators
     should be used only to detect bugs</i>.

     @see OperaPreferences#load(InputStream)
     @see OperaPreferences#iterator()
     @see java.util.ConcurrentModificationException
     */
    private class OperaPreferencesIterator implements Iterator
    {
        // inherits documentation from java.util.Iterator interface
        public boolean hasNext()
        {
            return (i.hasNext());
        }

        /**
         Gets the next element in the iteration.  The elements returned are the
         string names of each section contained in the peferences associated
         with <code>this</code> iterator.

         @return the next name element in the iterator, as a string.
         */
        public Object next()
        {
            return (((PreferenceSection) i.next()).getName());
        }

        // inherits documentation from java.util.Iterator interface
        public void remove()
        {
            i.remove();
        }

        /**
         Constructs an <code>OperaPreferencesIterator</code> that will iterate
         forward through the preference sections in the order they were loaded.
         <br />
         This class acts as a wrapper around the iterator returned by the
         <code>List</code> used to store each prefernce section.
         */
        public OperaPreferencesIterator()
        {
            i = sections.listIterator();
        }

        /**
         This is an iterator returned by the <code>List</code> used to store
         each prefernce section.
         */
        private Iterator i;
    }

    /** The encoding used by Opera preference files. */
    private static final String OPERA_ENCODING = "UTF-8";

    /**
     The character used as the opening bracket delimiting the start of a section
     name.
     */
    private static final char OPEN_BRACKET  = '[';

    /**
     The character used as the closing bracket delimiting the end of a section
     name.
     */
    private static final char CLOSE_BRACKET = ']';

    /**
     The character used to delimit the end of a key name and the start of the
     value.
     */
    private static final char SEPARATOR     = '=';

    /** The default size of an Opera preference file. */
    private static final int DEFAULT_SIZE = 16384;

    /** The default number of sections in an Opera preference file. */
    private static final int DEFAULT_SECTION_COUNT = 20;

    /**
     A list container for storing preference sections in the order they are
     loaded from file.
     */
    private ArrayList sections = null;
}
