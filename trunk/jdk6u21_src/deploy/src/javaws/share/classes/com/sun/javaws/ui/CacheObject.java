/*
 * @(#)CacheObject.java	1.38 10/03/24
 *
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import com.sun.javaws.*;
import com.sun.javaws.jnl.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.ui.ImageLoader;
import com.sun.deploy.ui.ImageLoaderCallback;

import java.awt.*;
import java.util.*;
import java.text.DateFormat;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.net.URL;

import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.ImageIcon;
import javax.swing.Icon;
import javax.swing.SwingConstants;
import javax.swing.BorderFactory;
import javax.swing.table.AbstractTableModel;
import javax.swing.UIManager;

/**
 *
 */
class CacheObject {

    private static final DateFormat df = DateFormat.getDateInstance();

    private static final String[] JNLP_COLUMN_KEYS = {
        "jnlp.viewer.app.column",
        "jnlp.viewer.vendor.column",
        "jnlp.viewer.type.column",
        "jnlp.viewer.date.column",
        "jnlp.viewer.size.column",
        "jnlp.viewer.status.column"
    };

    private static final String[] RES_COLUMN_KEYS = {
        "res.viewer.name.column",
        "res.viewer.url.column",
        "res.viewer.modified.column",
        "res.viewer.expired.column",
        "res.viewer.version.column",
        "res.viewer.size.column"
    };

    private static final String[] DEL_COLUMN_KEYS = {
        "del.viewer.app.column",
        "del.viewer.url.column"
    };

    private static String[] [] keys = {JNLP_COLUMN_KEYS, 
                                        RES_COLUMN_KEYS, 
                                        DEL_COLUMN_KEYS };

    private static TLabel titleLabel;
    private static TLabel vendorLabel;
    private static TLabel typeLabel;
    private static TLabel [] dateLabel = new TLabel[3];
    private static TLabel sizeLabel;
    private static TLabel statusLabel;
    private static TLabel versionLabel;
    private static TLabel urlLabel;

    private static ImageIcon onlineIcon;
    private static ImageIcon offlineIcon; 
    private static ImageIcon noLaunchIcon;
    private static ImageIcon java32;
    private static ImageIcon jnlp24;
    private static ImageIcon jar24;
    private static ImageIcon class24;
    private static ImageIcon image24;
    private static ImageIcon other24;

    private final int tableType;

    private final CacheEntry cacheEntry; 
    private final AbstractTableModel tableModel;

    private int objectType = -1;
    public final static int TYPE_DELETED = 0;
    public final static int TYPE_JNLP = 1;
    public final static int TYPE_JAR = 2;
    public final static int TYPE_CLASS = 3;
    public final static int TYPE_IMAGE = 4;
    public final static int TYPE_OTHER = 5;

    private static HashMap imageMap = new HashMap();

    /**
     * Construct the GUI ovjects for a row
     */
    public CacheObject(CacheEntry ce, AbstractTableModel model, int type) {
        tableType = type;
        cacheEntry = ce;
        tableModel = model;
        if (titleLabel == null) {
            titleLabel = new TLabel(JLabel.LEFT);
            vendorLabel = new TLabel(JLabel.LEFT);
            typeLabel = new TLabel(JLabel.CENTER);
            for (int i=0; i<3; i++) {
                dateLabel[i] = new TLabel(JLabel.RIGHT);
            }
            sizeLabel = new TLabel(JLabel.RIGHT);
            statusLabel = new TLabel(JLabel.CENTER);
            versionLabel = new TLabel(JLabel.CENTER);
            urlLabel = new TLabel(JLabel.LEFT);
    
            java32 = new ViewerIcon(0,
        	ResourceManager.class.getResource("image/java32.png"));
    
            jnlp24 = new ViewerIcon(0, 
        	ResourceManager.class.getResource("image/jnlp24.png"));

            jar24 = new ViewerIcon(0, 
        	ResourceManager.class.getResource("image/jar24.png"));
    
            class24 = new ViewerIcon(0, 
        	ResourceManager.class.getResource("image/class24.png"));
    
            image24 = new ViewerIcon(0, 
        	ResourceManager.class.getResource("image/image24.png"));

            other24 = new ViewerIcon(0, 
        	ResourceManager.class.getResource("image/other24.png"));
    
            onlineIcon = new ViewerIcon(0, 
                ResourceManager.class.getResource("image/connect24.png"));

            offlineIcon = new ViewerIcon(0, 
                ResourceManager.class.getResource("image/disconnect24.png"));
    
            noLaunchIcon = null;
        }
    }

    public int getObjectType() {
	if (objectType < 0) {
	    objectType = TYPE_OTHER;
	    if (tableType == CacheTable.JNLP_TYPE) {
	        objectType = TYPE_JNLP;
	    } else if (tableType == CacheTable.DELETED_TYPE) {
	        objectType = TYPE_DELETED;
	    } else {
	        getNameString();
	        if (nameString.endsWith("jnlp")) {
		    objectType = TYPE_JNLP;
	        } else if (nameString.endsWith("jar") ||
			   nameString.endsWith("zip")) {
		    objectType = TYPE_JAR;
	        } else if (nameString.endsWith("class")) {
		    objectType = TYPE_CLASS;
	        } else if (nameString.endsWith("jpg") ||
			   nameString.endsWith("gif") ||
			   nameString.endsWith("png") ||
			   nameString.endsWith("ico")) {
		    objectType = TYPE_IMAGE;
	        }
	    }
	}
	return objectType;
    }

    private String deletedTitleString;
    private String deletedUrlString;
    public CacheObject(String title, String url, AbstractTableModel model) {
	this(null, model, CacheTable.DELETED_TYPE);
	deletedTitleString = title;
	deletedUrlString = url;
    }

    public String getDeletedTitle() {
	return deletedTitleString;
    }

    public String getDeletedUrl() {
        return deletedUrlString;
    }

    public static String getColumnName(int column, int type) {
        return ResourceManager.getMessage(keys[type][column]);
    }

    public static int getColumnCount(int type) {
	return keys[type].length;
    }

    public static String getHeaderToolTipText(int column, int type) {
	String key = "";
	key = keys[type][column];
        return ResourceManager.getString(key + ".tooltip");
    }
	
    public static int getPreferredWidth(int column, int type) {
	if (type == CacheTable.JNLP_TYPE) {
            switch (column) {
                case 0:
                    return 200;		// icon and title
                case 1:
                    return 140;		// vendor
                case 2:
                    return 76;		// type
                case 3:
                    return 76;		// date
                case 4:
                    return 64;		// size
	        case 5:
		    return 64;		// status
	    }
	} else if (type == CacheTable.RESOURCE_TYPE) {
            switch (column) {
                case 0:
                    return 120;		// icon and name
                case 1:
		    return 220;		// url
                case 2:
                    return 76;		// date
                case 3:
                    return 76;		// date
                case 4:
                    return 64;		// version
	        case 5:
                    return 64;		// size
	    }
	} else {
	    if (column == 0) {
		return 200;		// title
	    } else {
		return 420;		// url
	    }
	}
	return 600;	// total of other numbers
    }    

    public static Class getClass(int column, int type) {
	return JLabel.class;
    }    

    public Object getObject(int column) {
	if (tableType == CacheTable.JNLP_TYPE) {
            switch (column) {
                case 0:
                    return (Object) getTitleLabel();
                case 1:
                    return (Object) getVendorLabel();
                case 2:
                    return (Object) getTypeLabel();
                case 3:
                    return (Object) getDateLabel(0);
                case 4:
                    return (Object) getSizeLabel();
	        case 5:
	            return (Object) getStatusLabel();
	    }
	} else if (tableType == CacheTable.RESOURCE_TYPE) {
	    switch (column) {
		case 0:
		    return (Object) getNameLabel();
		case 1:
		    return (Object) getUrlLabel();
		case 2:
		    return (Object) getDateLabel(1);
		case 3:
		    return (Object) getDateLabel(2);
		case 4:
		    return (Object) getVersionLabel();
		case 5: 
		    return (Object) getSizeLabel();
	    }
	} 
	if (column == 0) {
	    return (Object) getDeletedTitleLabel();
	} else {
            return (Object) getDeletedUrlLabel();
	}
    }    

    public boolean isEditable(int column) {
	return false;
    }

    public void setValue(int column, Object value) {
    }

    private final int VIEWER_ICON_SIZE = 24;
    
    private String titleString = null;
    public String getTitleString() {
	if (titleString == null) {
	    titleString = getTitle();
	}
	return titleString;
    }
    private ImageIcon icon = null;
    private TLabel getTitleLabel() {
	if (icon == null) {
            IconDesc id = null;
	    LaunchDesc ld = getLaunchDesc();
	    if (ld != null) {
		InformationDesc info = ld.getInformation();
		if (info != null) {
		    id = info.getIconLocation(VIEWER_ICON_SIZE,
			IconDesc.ICON_KIND_DEFAULT);
		}
	    }
	    if (id != null) {
		icon = new ViewerIcon(VIEWER_ICON_SIZE, 
			id.getLocation(), id.getVersion());
	    }
            if (icon == null) {
	        icon = java32;
	    }
	}
	if ( (icon != null) && (icon.getIconWidth() > 0) && 
			        (icon.getIconHeight() > 0)) {
            titleLabel.setIcon(icon);
	} else {
	    titleLabel.setIcon(java32);
	}
	titleLabel.setText(getTitleString());
	return titleLabel;
    }

    private TLabel getDeletedTitleLabel() {
	titleLabel.setIcon(null);
	titleLabel.setText(deletedTitleString);
	return titleLabel;
    }

    private TLabel getDeletedUrlLabel() {
	urlLabel.setText(deletedUrlString);
	return urlLabel;
    }

    private String nameString = null;

    public String getNameString() {
	if (nameString == null) {
	    String url = cacheEntry.getURL();
	    int i = url.lastIndexOf("/");
	    int index = (i < 0 || i >= (url.length()-1)) ? 0 : i + 1;
	    nameString = url.substring(index);
	    i = nameString.lastIndexOf(".jarjnlp");
	    if (i > 0) {
		nameString = nameString.substring(0,i) + ".jnlp";
	    }
	}
	return nameString;
    }

    private ImageIcon getJNLPIcon() {
	if (icon == null) {
            IconDesc id = null;
	    LaunchDesc ld = getLaunchDesc();
	    if (ld != null) {
		InformationDesc info = ld.getInformation();
		if (info != null) {
		    id = info.getIconLocation(VIEWER_ICON_SIZE,
			IconDesc.ICON_KIND_DEFAULT);
		}
	    }
	    if (id != null) {
		icon = new ViewerIcon(VIEWER_ICON_SIZE, 
			id.getLocation(), id.getVersion());
	    }
            if (icon == null) {
	        icon = jnlp24;
    	    }
	}
	return icon;
    }

    private ImageIcon getJarIcon() {
	return jar24;
    }
    private ImageIcon getClassIcon() {
	return class24;
    }
    private ImageIcon getImageIcon() {
	return image24;
    }
    private ImageIcon getOtherIcon() {
	return other24;
    }

    private ImageIcon getTypeIcon() {
        if (icon == null) {
	    switch (getObjectType()) {
		case TYPE_JNLP:
		    icon = getJNLPIcon();
		    break;

		case TYPE_JAR:
		    icon = getJarIcon();
		    break;

		case TYPE_CLASS:
		    icon = getClassIcon();
		    break;

		case TYPE_IMAGE:
		    icon = getImageIcon();
		    break;
		
		default:
		    icon = getOtherIcon();
		    break;
	
	    }
	}
	return icon;
    }

    private TLabel getNameLabel() {
	titleLabel.setText(getNameString());
	titleLabel.setIcon(getTypeIcon());
	return titleLabel;
    }

    private String vendorString = null; 
    public String getVendorString() {
	if (vendorString == null) {
	    vendorString = getVendor();
	}
	return vendorString;
    }
    private TLabel getVendorLabel() {
	vendorLabel.setText(getVendorString());
	return vendorLabel;
    }

    public static String getLaunchTypeString(int launchType) {
         switch (launchType) {
             case LaunchDesc.APPLICATION_DESC_TYPE:
                 return ResourceManager.getMessage("viewer.application");
             case LaunchDesc.APPLET_DESC_TYPE:
 		return ResourceManager.getMessage("viewer.applet");
             case LaunchDesc.LIBRARY_DESC_TYPE:
 		return ResourceManager.getMessage("viewer.extension");
             case LaunchDesc.INSTALLER_DESC_TYPE:
 		return ResourceManager.getMessage("viewer.installer");
 	}
 	return "";
     }


    private String typeString = null;
    public String getTypeString() {
	if (typeString == null) {
	    typeString = getLaunchTypeString(getLaunchDesc().getLaunchType());
	}
        return typeString;
    }

    private TLabel getTypeLabel() {
	typeLabel.setText(getTypeString());
	return typeLabel;
    }

    private Date [] date = new Date[3];
    private String [] dateString = new String [3];

    public Date getDate(int which) {
	if (dateString[which] == null) {
	    switch (which) {
		case 0: 
		    date[0] = getLastAccesed();
		    break;
		case 1:
		    date[1] = getLastModified();
		    break;
		case 2:
		    date[2] = getExpired();
		    break;
	    }
	    if (date[which] != null) {
	        dateString[which] = df.format(date[which]);
	    } else {
		dateString[which] = "";
	    }
	}
        return date[which];
    }

    private TLabel getDateLabel(int which) {
	getDate(which);
	dateLabel[which].setText(dateString[which]);
	return dateLabel[which];
    }

    private long theSize = 0;
    private String sizeString = null;

    public long getSize() {
	if (sizeString == null) {
	    theSize = getResourceSize();
	    if (theSize > 10240) {
		sizeString =  (" " + theSize/1024 + " KB");
            } else {
           	sizeString = (" " + theSize/1024 + "." + 
			    (theSize%1024)/102 + " KB");
            }
	}
	return theSize;
    }

    private TLabel getSizeLabel() {
	getSize();
	sizeLabel.setText(sizeString);
	return sizeLabel;
    }

    private int statusInt = -1;
    private ImageIcon statusIcon = null;
    private String statusText = "";
    public int getStatus() {
	if (statusInt < 0) {
	    statusInt = 0;
            if (canLaunchOffline()) { 
                statusInt |= 3; // apps that can run offline can also run online
            } 
	    if (hasHref()) {
                statusInt |= 1; 
            }  
	    switch (statusInt) {
		case 0:
		    statusIcon = noLaunchIcon;
		    if (getLaunchDesc().isApplicationDescriptor()) {
			statusText = ResourceManager.getString(
		            "viewer.norun1.tooltip", getTypeString());
		    } else {
			statusText = ResourceManager.getString(
			    "viewer.norun2.tooltip");
		    }
		    break;
		case 1:
		    statusIcon = onlineIcon;
		    statusText = ResourceManager.getString(
			"viewer.online.tooltip", getTypeString());
		    break;
		case 2:
		    statusIcon = offlineIcon;
		    statusText =  ResourceManager.getString(
			"viewer.offline.tooltip", getTypeString());
		    break;
		case 3:
		    statusIcon = offlineIcon;
		    statusText =  ResourceManager.getString(
			"viewer.onlineoffline.tooltip", getTypeString());
		    break;
	    }
	}
	return statusInt;
    }

    private TLabel getStatusLabel() {
	getStatus();
	if ((statusIcon == null) ||
	    (statusIcon.getIconWidth() > 0) && 
	    (statusIcon.getIconHeight() > 0)) {
	    statusLabel.setIcon(statusIcon);
	    statusLabel.setToolTipText(statusText);
	}
	return statusLabel;	
    }

    public static void hasFocus(Component c, boolean hasFocus) {
	if (c instanceof TLabel) {
	    ((TLabel) c).hasFocus(hasFocus);
	}
    }

    public int compareColumns(CacheObject other, int column) {
	if (tableType == CacheTable.RESOURCE_TYPE) {
	    switch (column) {
	        case 0:
		    return compareStrings(getNameString(), 
					  other.getNameString());
	        case 1:
		    return compareStrings(getUrlString(), other.getUrlString());
	        case 2:
		    return compareDates(getDate(1), other.getDate(1));
	        case 3: 
		    return compareDates(getDate(2), other.getDate(2));
	        case 4: 
		    return compareStrings(getVersionString(), 
					  other.getVersionString());
	        default:
	        case 5: 
		    return compareLong(getSize(), other.getSize());
	    }
	} else if (tableType == CacheTable.JNLP_TYPE) {
	    switch (column) {
	        case 0:
		    return compareStrings(getTitleString(), 
					  other.getTitleString());
	        case 1:
		    return compareStrings(getVendorString(), 
					  other.getVendorString());
	        case 2:
		    return compareStrings(getTypeString(), 
					  other.getTypeString());
	        case 3: 
		    return compareDates(getDate(0), other.getDate(0));
	        case 4: 
		    return compareLong(getSize(), other.getSize());
	        default:
	        case 5: 
		    return compareInt(getStatus(), other.getStatus());
	    }
	} else if (tableType == CacheTable.DELETED_TYPE) {
	    switch (column) {
	        case 0:
		    return compareStrings(getDeletedTitle(), 
					  other.getDeletedTitle());
		default:
	        case 1:
		    return compareStrings(getDeletedUrl(), 
					  other.getDeletedUrl());
	    }
	}
	return 0;
    }

    private final static float[] dash = {1.0f, 2.0f};

    private final static BasicStroke _dashed = new BasicStroke(1.0f, 
	BasicStroke.CAP_SQUARE, BasicStroke.JOIN_MITER, 10.0f, dash, 0.0f);

    private class TLabel extends JLabel {

	boolean _focus = false;

	public TLabel(int align) {
	    super();
	    setOpaque(true);
	    setBorder(BorderFactory.createEmptyBorder(0,4,0,4));
	    setHorizontalAlignment(align);
	}

	public void paint(Graphics g) {
	    super.paint(g);
	    if (_focus && g instanceof Graphics2D) {
		Stroke old = ((Graphics2D) g).getStroke();
		((Graphics2D) g).setStroke(_dashed); 
		g.drawRect(0,0,getWidth()-1,getHeight()-1);
		((Graphics2D) g).setStroke(old); 
	    }
	}

	public void hasFocus(boolean bFocus) {
	    _focus = bFocus;
	}
    }

    private int compareStrings(String s1, String s2) {
	if (s1 == s2) { return 0; }
	if (s1 == null) { return -1; }
	if (s2 == null) { return 1; }
	return s1.compareTo(s2);
    }

    private int compareDates(Date d1, Date d2) {
	if (d1 == d2) { return 0; }
	if (d1 == null) { return -1; }
	if (d2 == null) { return 1; }
	return compareLong(d1.getTime(), d2.getTime()); 
    }

    private int compareLong(long l1, long l2) {
	if (l1 == l2) { return 0; }
	return (l1 < l2) ? -1 : 1;
    }

    private int compareInt(int l1, int l2) {
	if (l1 == l2) { return 0; }
	return (l1 < l2) ? -1 : 1;
    }

    
    /*
     * Access to the actual _dce object
     *
     * Lazy as possible
     */
    public CacheEntry getCE() {
        return cacheEntry;
    }

    LaunchDesc _ld = null;
    LocalApplicationProperties _lap = null;

    public LaunchDesc getLaunchDesc() {
	if (cacheEntry == null || (tableType == CacheTable.RESOURCE_TYPE && 
	    !(cacheEntry.getURL().endsWith("jnlp")))) {
	    return null;
	}
	if (_ld == null) {
	    try {
                _ld = LaunchDescFactory.buildDescriptor(	
					cacheEntry.getDataFile(), 
                    (null != cacheEntry) ? URLUtil.getBase(new URL(cacheEntry.getURL())) : null, 
                    null, new URL(cacheEntry.getURL()));
            } catch (Exception e) {
                Trace.ignoredException(e);
	    }
	}
	return _ld;
    }

    public LocalApplicationProperties getLocalApplicationProperties() {
	if (_lap == null) {
            _lap = Cache.getLocalApplicationProperties(cacheEntry);
	}
	return _lap;
    }

    public File getJnlpFile() {
	return cacheEntry.getDataFile();
    }

    public String getTitle() {
	try {
	    return getLaunchDesc().getInformation().getTitle();
	} catch (Exception e) {
	    return "";
	}
    }

    public String getVendor() {
	try {
	    return getLaunchDesc().getInformation().getVendor();
	} catch (Exception e) {
	    return "";
	}
    }

    public String getHref() {
	URL url = getLaunchDesc().getLocation();
	if (url != null) return url.toString();
	return null;
    }

    public File getIconFile() {
	try {
	    IconDesc id = getLaunchDesc().getInformation().getIconLocation(
		VIEWER_ICON_SIZE, IconDesc.ICON_KIND_DEFAULT);
		File iconEntry = DownloadEngine.getCachedFile(id.getLocation(), 
		    id.getVersion());
                if (iconEntry != null) {
                    return iconEntry;
		}
	} catch (Exception e) {
	}
	return null;
    }

    public Date getLastAccesed() {
        return getLocalApplicationProperties().getLastAccessed();
    }

    public Date getLastModified() {
	long time = cacheEntry.getLastModified();
	return (time > 0 ) ? new Date(time) : null;
    }

    public Date getExpired() {
	long time = cacheEntry.getExpirationDate();
	return (time > 0 ) ? new Date(time) : null;
    }

    public long getResourceSize() {
        long size = 0;
	if (tableType == CacheTable.JNLP_TYPE) {
            size += LaunchDownload.getCachedSize(getLaunchDesc());
	    size += cacheEntry.getSize();
	} else if (tableType == CacheTable.RESOURCE_TYPE) {
	    size += cacheEntry.getSize();
        }
	return size;
    }

    private String versionString = null;

    public String getVersionString() {
	if (versionString == null) {
	    versionString = cacheEntry.getVersion();
	    if (versionString == null) {
		versionString = "";
	    }
	}
        return versionString;
    }
    public TLabel getVersionLabel() {
	versionLabel.setText(getVersionString());
	return versionLabel;
    }

    public String getUrlString() {
	return cacheEntry.getURL();
    }
    
    public String getCodebaseIP() {
        return cacheEntry.getCodebaseIP();
    }

    public TLabel getUrlLabel() {
	urlLabel.setText(cacheEntry.getURL());
	return urlLabel;
    }

    public boolean inFilter(int launchType) {
	return (launchType == 0 || 
		launchType == getLaunchDesc().getLaunchType());
    }

    public boolean hasHref() {
	if (getLaunchDesc().isApplicationDescriptor()) {
	    return (_ld.getLocation() != null);
	}
	return false;
    }

    public boolean canLaunchOffline() {
	if (getLaunchDesc().isApplicationDescriptor()) {
	    return _ld.getInformation().supportsOfflineOperation();
	}
	return false;
    }

    private class ViewerIcon extends ImageIcon 
        implements ImageLoaderCallback {

        private int _width;
        private int _height;
        
        public ViewerIcon(int size, String path) {
            super();
            _width = size;
            _height = size;
            try {
                URL url = URLUtil.fileToURL(new File(path));
                if (url != null) {
		    if (!isCached(url, null)) {
                        ImageLoader.getInstance().loadImage(url, this, true);
		    }
                }
            } catch (Throwable t) {
                Trace.ignored(t);
            }
        }

        public ViewerIcon(int size, URL url, String ver) {
            super();
            _width = size;
            _height = size;
            try {
	        if (!isCached(url, ver)) {
                    ImageLoader.getInstance().loadImage(url, ver, this, true);
		}
            } catch (Throwable t) {
                Trace.ignored(t);
            }
        }

        public ViewerIcon(int size, URL url) {
            super();
            _width = size;
            _height = size;
            if (url == null) {
                url = ResourceManager.class.getResource("image/java32.png");
                _width = 24;
                _height = 24;
            }
            try {
                if (url != null) {
		    if (!isCached(url, null)) {
                        ImageLoader.getInstance().loadImage(url, this, true);
		    }
	        }
            } catch (Throwable t) {
                Trace.ignored(t);
            }
        }

	private boolean isCached(URL url, String version) {
            Image im = (Image) imageMap.get(getKey(url, version));
            if (im != null) {
                setImage(im);
                return true;
            } else {
                return false;
            }
        }

        private String getKey(URL url, String version) {
            String key = "" + url + "-w=" + _width + "-h=" +_height;
            if (version != null) {
                key += "-version=" + version;
            }
            return key;
        }

        /*
         * implementation of CacheImageLoaderCallback
         * when loaded - repaint
         */  
        public void imageAvailable(final URL url, final String version, 
                                   Image image, File file) {
            final int w = image.getWidth(null);
            final int h = image.getHeight(null);
            final Image imageIn = image;
            new Thread(new Runnable() {
                public void run() {
                    Image im = imageIn;
                    if ((_width > 0 && _height > 0) && 
                        (_width != w || _height != h)) {
                        im = imageIn.getScaledInstance(_width, _height, 
                                                    Image.SCALE_DEFAULT);
                    }
                    imageMap.put(getKey(url, version), im);
                    setImage(im);
                    tableModel.fireTableDataChanged();
                }
            }).start(); 
        }

        public void finalImageAvailable(URL url, String version,
                                        Image image, File file) {
            imageAvailable(url, version, image, file);
        }
    }
}


