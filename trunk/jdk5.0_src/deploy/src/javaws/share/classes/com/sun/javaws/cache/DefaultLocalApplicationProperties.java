/*
 * @(#)DefaultLocalApplicationProperties.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;

import java.io.*;
import java.net.*;
import java.text.*;
import java.util.*;

import com.sun.javaws.*;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.jnl.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


/**
 * This caches implementation of the LocalApplicationProperties.
 */

public class DefaultLocalApplicationProperties implements
        LocalApplicationProperties {

    private static final String REBOOT_NEEDED_KEY = "_default.rebootNeeded";
    private static final String UPDATE_CHECK_KEY = "_default.forcedUpdateCheck";
    private static final String NATIVELIB_DIR_KEY = "_default.nativeLibDir";
    private static final String INSTALL_DIR_KEY = "_default.installDir";
    private static final String LAST_ACCESSED_KEY = "_default.lastAccessed";
    private static final String LAUNCH_COUNT_KEY = "_default.launchCount";
    private static final String ASK_INSTALL_KEY = "_default.askedInstall";
    private static final String SHORTCUT_KEY = "_default.locallyInstalled";
    private static final String INDIRECT_PATH_KEY = "_default.indirectPath";
    private static final String ASSOCIATION_MIME_KEY = "_default.mime.types.";
    private static final String ASSOCIATION_EXTENSIONS_KEY =
					"_default.extensions.";


    private static final DateFormat _df = DateFormat.getDateTimeInstance();
    /**
     * Descriptor we represent.
     */
    private LaunchDesc _descriptor;

    /**
     * How we store information.
     */
    private Properties _properties;
    /**
     * Location for application/extension
     */
    private URL _location;
    /**
     * VersionId for application/extension
     */
    private String _versionId;
    /**
     * Last time the disk was accessed.
     */
    private long _lastAccessed;
    /*
     * Application or Extension descriptor
     */
    private boolean _isApplicationDescriptor;
    /**
     * True if the properties have changed without being saved.
     */
    private boolean _dirty;
    /**
     * True if the app is installed in the system cache.
     */
    private boolean _isLocallyInstalledSystem;

    public DefaultLocalApplicationProperties(URL location,
            String versionId, LaunchDesc descriptor,
            boolean isApplicationDescriptor) {
        _descriptor = descriptor;
        _location = location;
        _versionId = versionId;
        _isApplicationDescriptor = isApplicationDescriptor;
        _properties = getLocalApplicationPropertiesStorage(this);
        _isLocallyInstalledSystem = false;

    }

    /**
     * Returns the codebase for the application
     */
    public URL getLocation() {
        return _location;
    }

    /**
     * Returns the codebase for the application
     */
    public String getVersionId() {
        return _versionId;
    }

    /**
     * Returns the LaunchDescriptor these properties are associated with.
     */
    public LaunchDesc getLaunchDescriptor() {
        return _descriptor;
    }

    /**
     * Dates this application was last accessed (run).
     */
    public void setLastAccessed(Date date) {
        put(LAST_ACCESSED_KEY, _df.format(date));
    }

    public Date getLastAccessed() {
	return getDate(LAST_ACCESSED_KEY);
    }

    public void incrementLaunchCount() {
        int count = getLaunchCount();

        put(LAUNCH_COUNT_KEY, Integer.toString(++count));
    }

    /**
     * Number of times this application was launched. This will be
     * at least 1.
     */
    public int getLaunchCount() {
        return getInteger(LAUNCH_COUNT_KEY);
    }

    /**
     * Indicates if the user has been prompted if they want to do an
     * install.
     */
    public void setAskedForInstall(boolean askedForInstall) {
        put(ASK_INSTALL_KEY, new Boolean(askedForInstall).toString());
    }

    public boolean getAskedForInstall() {
        return getBoolean(ASK_INSTALL_KEY);
    }

    public void setRebootNeeded(boolean reboot) {
        put(REBOOT_NEEDED_KEY, new Boolean(reboot).toString());
    }

    public boolean isRebootNeeded() {
        return getBoolean(REBOOT_NEEDED_KEY);
    }

    public void setLocallyInstalled(boolean installed) {
        put(SHORTCUT_KEY, new Boolean(installed).toString());
    }

    public boolean isLocallyInstalled() {
        return getBoolean(SHORTCUT_KEY);
    }

    public boolean isLocallyInstalledSystem() {
        return (_isLocallyInstalledSystem);
    }

    public boolean forceUpdateCheck() {
        return getBoolean(UPDATE_CHECK_KEY);
    }

    public void setForceUpdateCheck(boolean state) {
        put(UPDATE_CHECK_KEY, new Boolean(state).toString());
    }

    public boolean isApplicationDescriptor() {
        return _isApplicationDescriptor;
    }

    public boolean isExtensionDescriptor()   {
        return !_isApplicationDescriptor;
    }

    public String getInstallDirectory() {
        return get(INSTALL_DIR_KEY);
    }

    public void setInstallDirectory(String path) {
        put(INSTALL_DIR_KEY, path);
    }

    public String getNativeLibDirectory() {
        return get(NATIVELIB_DIR_KEY);
    }

    public void setNativeLibDirectory(String path) {
        put(NATIVELIB_DIR_KEY, path);
    }

    public void setAssociations(AssociationDesc [] associations) {
	int i=0;
	if (associations == null) {
	    AssociationDesc [] old = getAssociations();
	    if (old != null) {
	        put(ASSOCIATION_MIME_KEY+i, null);
                put(ASSOCIATION_EXTENSIONS_KEY+i,  null);
	    }
	} else {
	    for (i=0; i<associations.length; i++) {
		put(ASSOCIATION_MIME_KEY+i,
		    associations[i].getMimeType());
	        put(ASSOCIATION_EXTENSIONS_KEY+i,
		    associations[i].getExtensions());
	    }
	    put(ASSOCIATION_MIME_KEY+i, null);
            put(ASSOCIATION_EXTENSIONS_KEY+i,  null);
	}
    }

    public void addAssociation(AssociationDesc association) {
	AssociationDesc [] list;
	AssociationDesc [] old = getAssociations();
	int i = 0;
	if (old == null) {
	    list = new AssociationDesc[1];
	} else {
	    list = new AssociationDesc[old.length + 1];
	    while (i < old.length) {
		list[i] = old[i];
		i++;
	    }
	}
	list[i] = association;
        setAssociations(list);
    }

    public AssociationDesc [] getAssociations() {
	ArrayList al = new ArrayList();
	for (int i=0; ; i++) {
	    String mimeType = get(ASSOCIATION_MIME_KEY+i);
	    String extensions = get(ASSOCIATION_EXTENSIONS_KEY+i);
	    if (mimeType != null || extensions != null) {
		al.add(new AssociationDesc(extensions, mimeType));
	    } else {
		break;
	    }
	}
	return (AssociationDesc []) (al.toArray(new AssociationDesc [0]));
    }

    public void put(String key, String value) {
        synchronized(DefaultLocalApplicationProperties.this) {
            if (value == null) {
                _properties.remove(key);
            }
            else {
                _properties.put(key, value);
            }
            _dirty = true;
        }
    }

    public String get(String key) {
        synchronized(DefaultLocalApplicationProperties.this) {
            return (String)_properties.get(key);
        }
    }

    /**
     * Returns an integer representation of <code>key</code>, returns 0
     * if not found, or there is an error in parsing the string.
     */
    public int getInteger(String key) {
        String value = get(key);

        if (value == null) {
            return 0;
        }
        int count = 0;
        try {
            count = Integer.parseInt(value);
        } catch (NumberFormatException nfe) {
            count = 0;
        }
        return count;
    }

    /**
     * Returns the boolean representation of <code>key</code>. This will
     * return false if <code>key</code> is not defined.
     */
    public boolean getBoolean(String key) {
        String value = get(key);

        if (value == null) {
            return false;
        }
        return Boolean.valueOf(value).booleanValue();
    }

    /**
     * Returns the date representation of <code>key</code>. This will
     * return null if <code>key</code> is not defined, or there is a parse
     * error.
     */
    public Date getDate(String key) {
        String value = get(key);

        if (value == null) {
            return null;
        }
        try {
            return _df.parse(value);
        } catch (ParseException pe) {
            return null;
        }
    }

    public boolean doesNewVersionExist() {
        synchronized(DefaultLocalApplicationProperties.this) {
            long cache = Cache.getLastAccessed();

            if (cache == 0) {
                return false;
            }
            if (cache > _lastAccessed) {
                return true;
            }
        }
        return false;
    }

    /**
     * Saves the properties.
     */
    synchronized public void store() throws IOException {
        putLocalApplicationPropertiesStorage(this, _properties);
        _dirty = false;
    }

    /**
     * Reloads the state of the receiver.
     */
    public void refreshIfNecessary() {
        synchronized(DefaultLocalApplicationProperties.this) {
            if (!_dirty && doesNewVersionExist()) {
                refresh();
            }
        }
    }

    /**
     * Reloads the state of the receiver.
     */
    public void refresh() {
        synchronized(DefaultLocalApplicationProperties.this) {
            Properties props = getLocalApplicationPropertiesStorage(this);
            _properties = props;
            _dirty = false;
        }
    }


    public boolean isShortcutSupported() {
        DiskCacheEntry dce = null;
        try {
            dce = Cache.getCacheEntry(Cache.APPLICATION_TYPE, _location, null);
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
            return false;
        }
        if (dce == null || dce.isEmpty()) {
            return false;
        }
        ShortcutDesc sd = _descriptor.getInformation().getShortcut();
        return ((sd == null) ||                 // no hints
                (sd.getDesktop()) ||        // hints want desktop
                (sd.getMenu()));                // hints want menu
    }


    /**
     * Returns the contents of local application properties
     * for the URL <code>url</code>.
     */
    private Properties getLocalApplicationPropertiesStorage(
	DefaultLocalApplicationProperties lap) {
        Properties props = new Properties();
        try {
            URL url = lap.getLocation();
            String versionId = lap.getVersionId();
            if (url != null) {
                char type = (lap.isApplicationDescriptor()) ?
			Cache.APPLICATION_TYPE : Cache.EXTENSION_TYPE;
                byte[] data = Cache.getLapData(type, url, versionId, true);
                if (data != null) {
                    props.load(new ByteArrayInputStream(data));
                    String value = (String) props.get(SHORTCUT_KEY);
                    if (value != null) {
                        _isLocallyInstalledSystem =
                            Boolean.valueOf(value).booleanValue();
                    }
                }
                data = Cache.getLapData(type, url, versionId, false);
                if (data != null) {
                    props.load(new ByteArrayInputStream(data));
                }
            }
        } catch(IOException ioe) {
            Trace.ignoredException(ioe);
        }
        return props;
    }

    /**
     * Saves the LocalApplicationProperties to the cache.
     */
    private void putLocalApplicationPropertiesStorage(
	DefaultLocalApplicationProperties lap, Properties props)
	throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        try {
            props.store(bos, "LAP");
        } catch (IOException ioe) {}
        bos.close();
        char type = (lap.isApplicationDescriptor()) ?
		Cache.APPLICATION_TYPE : Cache.EXTENSION_TYPE;
	Cache.putLapData(type, lap.getLocation(),
			 lap.getVersionId(), bos.toByteArray());
    }

}
