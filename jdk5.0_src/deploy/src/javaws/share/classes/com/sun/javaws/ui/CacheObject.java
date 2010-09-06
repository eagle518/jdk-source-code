/*
 * @(#)CacheObject.java	1.13 04/04/04
 *
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import com.sun.javaws.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.cache.*;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DialogFactory;

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

    private static final DateFormat _df = DateFormat.getDateInstance();

    private static final String[] COLUMN_KEYS = {
        "jnlp.viewer.app.column",
        "jnlp.viewer.vendor.column",
        "jnlp.viewer.type.column",
        "jnlp.viewer.date.column",
        "jnlp.viewer.size.column",
        "jnlp.viewer.status.column"
    };

    private static final int _columns = COLUMN_KEYS.length;

    private static TLabel _title;
    private static TLabel _vendor;
    private static TLabel _type;
    private static TLabel _date;
    private static TLabel _size;
    private static TLabel _status;

    private static ImageIcon _onlineIcon;
    private static ImageIcon _offlineIcon; 
    private static ImageIcon _noLaunchIcon;
    private static ImageIcon _java32;

    private final DiskCacheEntry _dce;
    private final AbstractTableModel _model;

    /**
     * Construct the GUI ovjects for a row
     */
    public CacheObject(DiskCacheEntry dce, AbstractTableModel model) {
	_dce = dce;
	_model = model;
	if (_title == null) {
    	    _title = new TLabel(JLabel.LEFT);
    	    _vendor = new TLabel(JLabel.LEFT);
    	    _type = new TLabel(JLabel.CENTER);
    	    _date = new TLabel(JLabel.RIGHT);
    	    _size = new TLabel(JLabel.RIGHT);
    	    _status = new TLabel(JLabel.CENTER);

	    _java32 = new ViewerIcon(0, 0, 
		ResourceManager.class.getResource("image/java32.png"));

	    _onlineIcon = new ViewerIcon(0, 0, 
		ResourceManager.class.getResource("image/online.gif"));
	    _offlineIcon = new ViewerIcon(0, 0, 
		ResourceManager.class.getResource("image/offline.gif"));

    	    _noLaunchIcon = null;
	}
    }

    public static String getColumnName(int column) {
        return ResourceManager.getMessage(COLUMN_KEYS[column]);
    }

    public static int getColumnCount() {
	return _columns;
    }

    public static String getHeaderToolTipText(int column) {
        return ResourceManager.getString(COLUMN_KEYS[column] + ".tooltip");
    }
	
    public static int getPreferredWidth(int column) {
        if (column < _columns) switch (column) {
            case 0:
                return 192;		// icon and title
            case 1:
                return 140;		// vendor
            case 2:
                return 70;		// type
            case 3:
                return 70;		// date
            case 4:
                return 64;		// size
	    case 5:
		return 64;		// status
	}
	throw new ArrayIndexOutOfBoundsException("column index: "+column);
    }    

    public static Class getClass(int column) {
        if (column < _columns) switch (column) {
            case 0:
                return JLabel.class;		// icon and title
            case 1:
                return JLabel.class;		// vendor
            case 2:
                return JLabel.class;		// type
            case 3:
                return JLabel.class;		// date
            case 4:
                return JLabel.class;		// size
	    case 5:
		return JLabel.class;		// status
	}
	throw new ArrayIndexOutOfBoundsException("column index: "+column);
    }    

    public Object getObject(int column) {
        if (column < _columns) switch (column) {
            case 0:
                return (Object) getTitleLabel();	// JLabel
            case 1:
                return (Object) getVendorLabel();	// JLabel
            case 2:
                return (Object) getTypeLabel();		// JLabel
            case 3:
                return (Object) getDateLabel();		// JLabel
            case 4:
                return (Object) getSizeLabel();		// JLabel
	    case 5:
	        return (Object) getStatusLabel();	// JLabel
	}
	throw new ArrayIndexOutOfBoundsException("column index: "+column);
    }    

    public boolean isEditable(int column) {
	return false;
    }

    public void setValue(int column, Object value) {
    }

    private final int ICON_W = 32;
    private final int ICON_H = 32;
    private String _titleString = null;
    public String getTitleString() {
	if (_titleString == null) {
	    _titleString = getTitle();
	}
	return _titleString;
    }
    private ImageIcon _icon = null;
    private JLabel getTitleLabel() {
	if (_icon == null) {
	    File iconFile = getIconFile();
	    if (iconFile != null) {
		_icon = new ViewerIcon(ICON_W, ICON_H, iconFile.getPath());
	    }
	    if (_icon == null) {
	        _icon = _java32;
	    }
	}
	if ( (_icon != null) && (_icon.getIconWidth() > 0) && 
			        (_icon.getIconHeight() > 0)) {
            _title.setIcon(_icon);
	}
	_title.setText(getTitleString());
	return _title;
    }

    private String _vendorString = null; 
    public String getVendorString() {
	if (_vendorString == null) {
	    _vendorString = getVendor();
	}
	return _vendorString;
    }
    private TLabel getVendorLabel() {
	_vendor.setText(getVendorString());
	return _vendor;
    }

    private String _typeString = null;
    public String getTypeString() {
	if (_typeString == null) {
	    _typeString = getLaunchTypeString(getLaunchDesc().getLaunchType());
	}
        return _typeString;
    }
    public static String getLaunchTypeString(int type) {
        switch (type) {
            case LaunchDesc.APPLICATION_DESC_TYPE:
                return ResourceManager.getMessage("jnlp.viewer.application");
            case LaunchDesc.APPLET_DESC_TYPE:
		return ResourceManager.getMessage("jnlp.viewer.applet");
            case LaunchDesc.LIBRARY_DESC_TYPE:
		return ResourceManager.getMessage("jnlp.viewer.extension");
            case LaunchDesc.INSTALLER_DESC_TYPE:
		return ResourceManager.getMessage("jnlp.viewer.installer");
	}
	return "";
    }
    private TLabel getTypeLabel() {
	_type.setText(getTypeString());
	return _type;
    }

    private Date _theDate = null;
    private String _dateString = null;
    public Date getDate() {
	if (_dateString == null) {
	    _theDate = getLastAccesed();
	    if (_theDate != null) {
	        _dateString = _df.format(_theDate);
	    } else {
		_dateString = "";
	    }
	}
	return _theDate;
    }
    private TLabel getDateLabel() {
	getDate();
	_date.setText(_dateString);
	return _date;
    }


    private long _theSize =0;
    private String _sizeString = null;
    public long getSize() {
	if (_sizeString == null) {
	    _theSize = getResourceSize();
	    if (_theSize > 10240) {
		_sizeString =  (" " + _theSize/1024 + " KB");
            } else {
           	_sizeString = (" " + _theSize/1024 + "." + 
			    (_theSize%1024)/102 + " KB");
            }
	}
	return _theSize;
    }
    private TLabel getSizeLabel() {
	getSize();
	_size.setText(_sizeString);
	return _size;
    }

    private int _statusInt = -1;
    private ImageIcon _statusIcon = null;
    private String _statusText = "";
    public int getStatus() {
	if (_statusInt < 0) {
            if (canLaunchOffline()) { 
                _statusInt = 2; 
            } else {
                 _statusInt =  (hasHref()) ? 1 : 0 ;
            }  
	    switch (_statusInt) {
		case 0:
		    _statusIcon = _noLaunchIcon;
		    if (getLaunchDesc().isApplicationDescriptor()) {
			_statusText = ResourceManager.getString(
		            "jnlp.viewer.norun1.tooltip", getTypeString());
		    } else {
			_statusText = ResourceManager.getString(
			    "jnlp.viewer.norun2.tooltip");
		    }
		    break;
		case 1:
		    _statusIcon = _onlineIcon;
		    _statusText = ResourceManager.getString(
			"jnlp.viewer.online.tooltip", getTypeString());
		    break;
		case 2:
		    _statusIcon = _offlineIcon;
		    _statusText =  ResourceManager.getString(
			"jnlp.viewer.offline.tooltip", getTypeString());
		    break;
	    }
	}
	return _statusInt;
    }
    private TLabel getStatusLabel() {
	getStatus();
	if ((_statusIcon == null) ||
	    (_statusIcon.getIconWidth() > 0) && 
	    (_statusIcon.getIconHeight() > 0)) {
	    _status.setIcon(_statusIcon);
	    _status.setToolTipText(_statusText);
	}
	return _status;	
    }

    public static void hasFocus(Component c, boolean hasFocus) {
	if (c instanceof TLabel) {
	    ((TLabel) c).hasFocus(hasFocus);
	}
    }

    public int compareColumns(CacheObject other, int column) {
	switch (column) {
	    case 0:
		return compareStrings(getTitleString(), other.getTitleString());
	    case 1:
		return compareStrings(getVendorString(), other.getVendorString());
	    case 2:
		return compareStrings(getTypeString(), other.getTypeString());
	    case 3: 
		return compareDates(getDate(), other.getDate());
	    case 4: 
		return compareLong(getSize(), other.getSize());
	    default:
	    case 5: 
		return compareInt(getStatus(), other.getStatus());
	}
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
    public DiskCacheEntry getDCE() {
	return _dce;
    }

    LaunchDesc _ld = null;
    LocalApplicationProperties _lap = null;

    public LaunchDesc getLaunchDesc() {
	if (_ld == null) {
	    try {
                _ld = LaunchDescFactory.buildDescriptor(_dce.getFile());
            } catch (Exception e) {
                Trace.ignoredException(e);
	    }
	}
	return _ld;
    }

    public LocalApplicationProperties getLocalApplicationProperties() {
	if (_lap == null) {
            _lap = Cache.getLocalApplicationProperties(_dce, getLaunchDesc());
	}
	return _lap;
    }

    public File getJnlpFile() {
	return _dce.getFile();
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
		InformationDesc.ICON_SIZE_MEDIUM, IconDesc.ICON_KIND_DEFAULT);
		DiskCacheEntry iconEntry = DownloadProtocol.getCachedVersion(
				           id.getLocation(), id.getVersion(), 
				           DownloadProtocol.IMAGE_DOWNLOAD);
                if (iconEntry != null) {
                    return iconEntry.getFile();
		}
	} catch (Exception e) {
	}
	return null;
    }

    public Date getLastAccesed() {
        return getLocalApplicationProperties().getLastAccessed();
    }

    public long getResourceSize() {
        return LaunchDownload.getCachedSize(getLaunchDesc());
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
	implements CacheImageLoaderCallback {

	private int _width;
	private int _height;
	
        public ViewerIcon(int w, int h, String path) {
	    super();
	    _width = w;
	    _height = h;
	    try {
		URL url = (new File(path)).toURL();
		if (url != null) {
	            CacheImageLoader.getInstance().loadImage(url, this);
		}
	    } catch (Exception e) {
		Trace.ignoredException(e);
	    }
	}

        public ViewerIcon(int w, int h, URL url) {
	    super();
	    _width = w;
	    _height = h;
	    if (url != null) {
	        CacheImageLoader.getInstance().loadImage(url, this);
	    }
	}

        /*
         * implementation of CacheImageLoaderCallback
         * when loaded - repaint
         */  
        public void imageAvailable(IconDesc id, Image image, File file) {
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
	            setImage(im);
	            _model.fireTableDataChanged();
		}
	    }).start(); 
        }

        public void finalImageAvailable(IconDesc id, Image image, File file) {
        }
    }
     
}


