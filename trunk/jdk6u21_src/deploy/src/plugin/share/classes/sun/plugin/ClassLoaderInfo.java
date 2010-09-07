/*
 * @(#)ClassLoaderInfo.java	1.51 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/**
 * This class keps track of information about active applet class loaders.
 * The classloaders are identified by their codebase URL.
 *
 * We keep around a pool of recently used class loaders.
 *
 * @author Graham Hamilton
 */

import java.io.PrintStream;
import java.util.Collection;
import java.util.Iterator;
import java.util.HashMap;
import java.util.ArrayList;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.SoftReference;
import java.net.URL;
import sun.applet.AppletClassLoader;
import sun.applet.AppletPanel;
import sun.plugin.security.PluginClassLoader;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

public class ClassLoaderInfo 
{
    private URL codebase;
    private String key;
    private int references;
    private HashMap jars;
    private boolean locked;
    private boolean isCachable = true;
    private static boolean initialized;

    // "infos" is a list of ClassLoaderInfos that are available for use.
    private static HashMap infos = new HashMap();

    // "zombies" is a list of ClassLoaderInfos that currently have
    // a reference count of zero.  We keep up to zombieLimit of these
    // guys available for resurrection.  The least recently used is
    // at the front, the more recently used at the end.
    // All entries in zombies are also in infos.
    private static int zombieLimit = 0;
    private static ArrayList zombies = new ArrayList();

    // Soft reference to the actual ClassLoader corresponding to
    // this ClassLoaderInfo
    private LoaderReference loaderRef = null;

    // Reference queue for tracking soft references
    private static ReferenceQueue refQueue = new ReferenceQueue();


    private static synchronized void initialize() {
	if (initialized) 
	    return;
	
	initialized = true;

	reset();
    }


    /** 
     * Reset setting.
     */
    public static synchronized void reset() 
    {
	initialized = true;

	zombieLimit = 0;

	String value = (String) java.security.AccessController.doPrivileged(
			    new sun.security.action.GetPropertyAction("javaplugin.classloader.cache.enabled"));

	if (value == null || value.equals("true"))
	{
	    // Set zombie cache sizes
	    zombieLimit = ((Integer) java.security.AccessController.doPrivileged(
		    new sun.security.action.GetIntegerAction("javaplugin.classloader.cache.sizes", 4))).intValue();
	}

	// For XP/IE, we limit the size to 4 max. to
	// reduce the chance of running out of memory.
	if (zombieLimit > 4)
	    zombieLimit = 4;
    }


    /**
     * Clear ClassLoader cache.
     */
    public static synchronized void clearClassLoaderCache()
    {
	// Iterate through the zombie list - this represents 
	// all the stopped applets that have been cached.
	// Cleanup all classloaders and its information.
	//
	Iterator iter = zombies.iterator();
	
	while (iter.hasNext())
	{
	    ClassLoaderInfo victim = (ClassLoaderInfo) iter.next();

	    if (victim != null)
	    {	
	        infos.remove(victim.key);

		// Clear the soft reference to the ClassLoader
		victim.clearLoaderRef();
	    }
	}

	zombies.clear();


	// Iterate through the infos list - this represents all 
	// the running applets and stopped applets in the system.
	// Tag each of them as not-cache.
	//
	Collection collection = infos.values();

	if (collection != null)
	{
	    iter = collection.iterator();

	    while (iter.hasNext())
	    {
		ClassLoaderInfo target = null;
		ArrayList list = (ArrayList)iter.next();
		if(list != null) {
		    Iterator listIter = list.iterator();
		    while(listIter.hasNext()) {
			target = (ClassLoaderInfo)listIter.next();
			if (target != null) {
			    target.isCachable = false;
			}
		    }
		}
	    }
	} 

	// Flush all existing classloaders
	AppletPanel.flushClassLoaders();
    }

    /**
     * Dump ClassLoader cache.
     */
    public static synchronized void dumpClassLoaderCache(PrintStream ps)
    {
	StringBuffer buffer = new StringBuffer();
	
	buffer.append("Dump classloader list ...\n");

	Collection collection = infos.values();

	if (collection != null)
	{
	    Iterator iter = collection.iterator();

	    while (iter.hasNext())
	    {
		ClassLoaderInfo info;
		ArrayList list = (ArrayList)iter.next();
		if(list != null) {
		    Iterator listIter = list.iterator();
		    while(listIter.hasNext()) {
			info = (ClassLoaderInfo)listIter.next();

			// Don't display this loader if its reference has been
			// cleared
			if ((info != null) &&
			    (info.loaderRef != null) &&
			    (info.loaderRef.get() != null))
			{
			    boolean isZombie = zombies.contains(info);

			    buffer.append("    codebase=" + info.codebase);
			    buffer.append(", key=" + info.key);
			    buffer.append(", zombie=" + isZombie);
			    buffer.append(", cache=" + info.isCachable);
			    buffer.append(", refcount=" + info.references);
			    buffer.append(", info=" + info);
			    buffer.append("\n");
			}
		    }
		}
	    }
	}

	buffer.append("Done.");

	ps.println(buffer.toString());
    }


    /**
     * remove the classloader for the specified URL
     *
     */
    public static synchronized void markNotCachable(URL codebase, String key) 
    {
	assert checkListsValidity() == true;

	ClassLoaderInfo cli = getUsableClassLoaderInfo(key);
	if(cli != null) {
	    //mark the classloader not cachable
	    cli.isCachable = false;

	    //make sure that classloader is not in the zombie list
	    if(zombies.remove(cli) == true) {
		//remove it from the infos list aswell, otherwise
		//this ClassLoaderInfo remains orphaned.
		removeClassLoaderInfo(cli);
	    }

	    //Flush the classloader so that new one can be created
	    AppletPanel.flushClassLoader(key);
	}

	assert checkListsValidity() == true;
    }


    private static synchronized void removeClassLoaderInfo(ClassLoaderInfo cli) {
	ArrayList list = (ArrayList)infos.get(cli.key);
	if(list != null) {
	    list.remove(cli);
	    if(list.size() == 0) {
		infos.remove(cli.key);
	    }
	}

	// Clear the soft reference to the classloader
	cli.clearLoaderRef();
    }

    private static synchronized void addClassLoaderInfo(ClassLoaderInfo cli) {
	ArrayList list = (ArrayList)infos.get(cli.key);
	if(list == null) {
	    list = new ArrayList();
	    list.add(cli);
	    infos.put(cli.key, list);
	}
	else {
	    list.add(cli);
	}
    }

    private static synchronized ClassLoaderInfo getUsableClassLoaderInfo(String key) {
	ArrayList list = (ArrayList)infos.get(key);
	if(list != null) {
	    Iterator listIter = list.iterator();
	    ClassLoaderInfo cli = null;
	    while(listIter.hasNext()) {
		cli = (ClassLoaderInfo)listIter.next();
		if(cli.isCachable == true) {
		    return cli;
		}
	    }
	}

	return null;
    }





    /**
     * Find ClassLoaderInfo for the given AppletPanel.
     *
     * If we don't have any active information, a new ClassLoaderInfo is
     * created with an initial reference count of zero.
     */
    public static synchronized ClassLoaderInfo find(URL codebase, String key) 
    {
	assert checkListsValidity() == true;

	initialize();

	if (codebase == null)
	    return null;

	ClassLoaderInfo cli = getUsableClassLoaderInfo(key);
	if(cli != null) {
	    // We have seen this classloader before, so the ClassLoader is 
	    // currently in use.
	    // We make sure this loader isn't on the zombies list.
	    //
	    zombies.remove(cli);
	}
	else {
	    // Usable ClassLoaderInfo was not found, create new one
	    cli = new ClassLoaderInfo(codebase, key);
	    addClassLoaderInfo(cli);
	} 

	assert checkListsValidity() == true;
	return cli;
    }

    /**
     * Add a retaining reference.
     */
    public synchronized void addReference() 
    {
	references++;
        Trace.msgPrintln("classloaderinfo.referencing", 
			  new Object[]{this, String.valueOf(references)},
                          TraceLevel.BASIC);
    }

    /**
     * Remove a retaining reference.  If there are no references left
     * then we put it on the zombies list.
     *
     * @return references count
     */
    synchronized int removeReference() 
    {
	references--;

        Trace.msgPrintln("classloaderinfo.releasing", 
			 new Object[]{this, String.valueOf(references)},
                         TraceLevel.BASIC);

	if (references < 0) 
	{
	    throw new Error("negative ref count???");
	}
	if (references == 0) 
	{
	    addZombie(this);
	}

        return references;
    }

    /**
     *  Add the given ClassLoaderInfo to the zomboies list.
     * If there are too many zombies we get rid of some.
     */
    private static synchronized void addZombie(ClassLoaderInfo cli) 
    {
		assert checkListsValidity() == true;

		// Flush the strong reference to the ClassLoader in the
		// AppletPanel. This allows the system to free up
		// the zombie classloader should memory become scarce.
		AppletPanel.flushClassLoader(cli.key);

		// There is no point in caching if the cache size is zero
		if (zombieLimit == 0 || !cli.isCachable)
			{
				// If the classloader info is tagged as not-cache, 
				// user have cleared the classloader cache, so 
				// we will not cache the classloader.
				removeClassLoaderInfo(cli);
			} 
		else
			{
	    
				// Classloader should be cached.
				Trace.msgPrintln("classloaderinfo.caching", new Object[] {cli}, TraceLevel.BASIC);

				// If exceptionStatus flag ever set in the life of the previous ClassLoader due
				// to Exception during Applet.stop() and Applet.destroy(),
				// clear the ClassLoader's soft referrent.  Else, just clean any
				// assertion status to allow fresh start next time.
				AppletClassLoader acl = (AppletClassLoader) cli.loaderRef.get();
				if (acl != null) {
					if (acl.getExceptionStatus()) {
						cli.clearLoaderRef();
					} 
				}

				// Add the ClassLoaderInfo to the end of the zombies list.
				zombies.add(cli);

				// Clean up our zombie list so we will get an accurate zombie count
				cleanupZombies();

				Trace.msgPrintln("classloaderinfo.cachesize", new Object[] {new Integer(zombies.size())}, TraceLevel.BASIC);


				// If there are too many zombies, kill unused classloader.
				if (zombies.size() > zombieLimit) 
					{    
						ClassLoaderInfo victim = (ClassLoaderInfo)zombies.get(0);

						Trace.msgPrintln("classloaderinfo.num", new Object[]{String.valueOf(zombieLimit), victim}, TraceLevel.BASIC);
	    		    
						// Flesh all references to classloader
						zombies.remove(0);
						removeClassLoaderInfo(victim);

						// Clear the soft reference to the classloader
						victim.clearLoaderRef();
					}
			}

		assert checkListsValidity() == true;
    }

    /**
     * Get the Classloader corresponding to this ClassLoaderInfo.
     * If not such ClassLoader is currently in existence, a new
     * one is created.
     */
    public synchronized AppletClassLoader getLoader()
    {
	AppletClassLoader loader = null;

	// Check for an existing classloader
	if (loaderRef != null)
	{
	    loader = (AppletClassLoader) loaderRef.get();
	}

	if (loader == null)
	{		
            // There is no class loader for this codebase.
	    // Either we haven't created one yet, or the
	    // one we created previously was garbage collected.
	    // Create a new one now.
            loader = (PluginClassLoader)java.security.AccessController.doPrivileged(
		new java.security.PrivilegedAction() { 
                   public Object run() { 
                       return new PluginClassLoader(codebase); 
                   } 
            }); 

	    loaderRef = new LoaderReference(loader);
	    jars.clear();
	    localJarsLoaded = false;
	}

	return loader;
    }

    /**
     * Get the classloader and AddReference to ClassLoaderInfo
     * Do it in one synchronized block to avoid issues that the ClassLoaderInfo
     * is in Zombie list but its references count > 0
     *
     * And get or create ThreadGroup and AppContext of the loader
     *
     * @return AppletClassLoader
     */
    AppletClassLoader grabClassLoader()
    {
        AppletClassLoader loader = null;

        synchronized(this) {
            loader = getLoader();
            addReference();
        }

        loader.getThreadGroup();

        return loader;
    }


    /**
     * Clear the soft reference to the ClassLoader so it can be
     * garbage collected more easily.
     */
    private synchronized void clearLoaderRef()
    {
	if (loaderRef != null)
	{   
	    loaderRef.clear();
	    loaderRef = null;
	}
    }

    /**
     * Clean up any zombie class loaders that have been freed by 
     * the system.
     */
    private static synchronized void cleanupZombies()
    {
	LoaderReference ref = (LoaderReference) refQueue.poll();
	
	while (ref != null)
	{
	    String key = ref.getKey();

	    ArrayList list = (ArrayList) infos.get(key);

	    if (list != null)
	    {
		ArrayList newlist = (ArrayList) list.clone();
		Iterator listIter = newlist.iterator();

		while (listIter.hasNext()) 
		{
		    ClassLoaderInfo victim = (ClassLoaderInfo) listIter.next();

		    if (victim.loaderRef == ref && zombies.contains(victim))
		    {
			list.remove(victim);
			zombies.remove(victim);
		    }
		}
	    }

	    ref = (LoaderReference) refQueue.poll();
	}
    }


    private ClassLoaderInfo(URL codebase, String key) {
	references = 0;
	this.codebase = codebase;
	this.key = key;
	jars = new HashMap();
    }

    synchronized void addJar(String name) {
	jars.put(name, name);
    }

    synchronized boolean hasJar(String name) {
	if (jars.get(name) != null) {
	    return true;
	} else {
	    return false;
	}
    }

    /*
    * Flag and utility routines for recording whether local .jar files in lib/app/
    * have been loaded or not.  This is used as a performance optimization
    * so that hasJar() does not need to be checked against the filesystem
    * each time an applet is loaded.
    */
  
    private boolean localJarsLoaded = false;
  
    public boolean getLocalJarsLoaded()  {
        return(localJarsLoaded);
    }
  
    public void setLocalJarsLoaded(boolean loadedFlag)  {
        localJarsLoaded = loadedFlag;
    }
  
    /**
     * Acquire the lock.  This is used to prevent two AppletPanels
     * trying to classload JARs at the same time.
     */
    public final synchronized void lock() throws InterruptedException {
	while (locked) {
	    wait();
	}
	locked = true;
    }

    /**
     * Release the lock.   This allows other people do to classloading.
     */
    public final synchronized void unlock() {
	locked = false;
	notifyAll();
    }


    public synchronized static boolean checkListsValidity()
    {
	ClassLoaderInfo cli = null;

	//Make sure only one classloader per codebase is marked
	//cachable in the infos list
	Collection collection = infos.values();
	Iterator iter = collection.iterator();
	while (iter.hasNext()) {
	    ArrayList list = (ArrayList)iter.next();
	    if(list != null) {
		int cachableCount = 0;
		Iterator listIter = list.iterator();
		while(listIter.hasNext()) {
		    cli = (ClassLoaderInfo)listIter.next();
		    if (cli != null && cli.isCachable == true) {
			cachableCount++;
		    }
		}
		if(cachableCount > 1) {
		    //assert (false, "Infos list has more than one cachable classloader");
		    return false;
		}
	    }
	} 

	//Make sure all the classloaders are cachable in the
	//zombies list
	iter = zombies.iterator();
	while (iter.hasNext()) {
	    cli = (ClassLoaderInfo) iter.next();
	    if (cli != null && cli.isCachable == false) {
		    //assert(false, "Zombies list has a non-cachable classloader");
		    return false;
	    }
	}

	return true;
    }

    /**
     * Private class used to hold a soft reference to a ClassLoader
     */
    private class LoaderReference extends SoftReference 
    {
	public LoaderReference(ClassLoader referrent) 
	{
	    super(referrent, refQueue);
	}

	public String getCodebase()
	{
	    return codebase.toString();
	}

	public String getKey()
	{
	    return key;
	}
    }
}
