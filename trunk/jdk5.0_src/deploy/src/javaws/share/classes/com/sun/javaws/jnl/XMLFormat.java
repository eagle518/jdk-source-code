/*
 * @(#)XMLFormat.java	1.48 04/06/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;

import com.sun.javaws.util.GeneralUtil;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.ByteArrayInputStream;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Properties;
import java.util.StringTokenizer;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.Globals;
import com.sun.javaws.exceptions.MissingFieldException;
import com.sun.javaws.exceptions.BadFieldException;
import com.sun.javaws.exceptions.JNLParseException;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.xml.XMLEncoding;
import com.sun.deploy.xml.XMLParser;
import com.sun.deploy.xml.XMLNode;
import com.sun.deploy.xml.BadTokenException;

public class XMLFormat {

    public static LaunchDesc parse(byte[] bits) throws IOException,
		BadFieldException, MissingFieldException, JNLParseException {

	String source;
	String encoding;
	XMLNode root;

	try {
            source = XMLEncoding.decodeXML(bits);
	} catch (Exception e) {
	    throw new JNLParseException(null, e,
		"exception determining encoding of jnlp file", 0);
	}

	try {
            root = (new XMLParser(source)).parse();
	} catch (BadTokenException bte) {
	    throw new JNLParseException(source, bte,
			"wrong kind of token found", bte.getLine());
	} catch (Exception e) {
	    throw new JNLParseException(source, e,
			"exception parsing jnlp file", 0);
	}

        InformationDesc info = null;
        ResourcesDesc resources = null;
        ApplicationDesc application = null;
        AppletDesc applet = null;
        LibraryDesc libraryDef = null;
        InstallerDesc installerDef = null;
        String internalCommand = null;

        // Check if we should launch the player or viewer
        if (root.getName().equals("player") ||
	    root.getName().equals("viewer")) {
	    String cpTab = XMLUtils.getAttribute(root, null, "tab");
	    return LaunchDescFactory.buildInternalLaunchDesc(
		root.getName(), source, cpTab);
        }

        // Check that root element is a <jnlp> tag
        if (!root.getName().equals("jnlp")) {
	    throw new MissingFieldException(source, "<jnlp>");  }

        // Read <jnlp> attributes (path is empty, i.e., "")
        // (spec, version, codebase, href)
        String specVersion = XMLUtils.getAttribute(root, "", "spec", "1.0+");
        String version = XMLUtils.getAttribute(root, "", "version");
        // Make sure the codebase URL ends with a '/'.
        URL codebase = URLUtil.asPathURL(XMLUtils.getAttributeURL(source, root, "", "codebase"));
        // Get href for JNLP file
        URL href = XMLUtils.getAttributeURL(source, codebase, root, "", "href");

        // Read <security> attributes
        int security = LaunchDesc.SANDBOX_SECURITY;
        if (XMLUtils.isElementPath(root, "<security><all-permissions>")) {
	    security = LaunchDesc.ALLPERMISSIONS_SECURITY;
        } else if (XMLUtils.isElementPath(root, "<security><j2ee-application-client-permissions>")) {
	    security = LaunchDesc.J2EE_APP_CLIENT_SECURITY;
        }

        // Figure out which mode
        int type;
        if (XMLUtils.isElementPath(root, "<application-desc>")) {
	    type = LaunchDesc.APPLICATION_DESC_TYPE;
	    application = buildApplicationDesc(source, root);
        } else if (XMLUtils.isElementPath(root, "<component-desc>")) {
	    type = LaunchDesc.LIBRARY_DESC_TYPE;
	    libraryDef = buildLibraryDesc(source, root);
        } else if (XMLUtils.isElementPath(root, "<installer-desc>")) {
	    type = LaunchDesc.INSTALLER_DESC_TYPE;
	    installerDef = buildInstallerDesc(source, codebase, root);
        } else if (XMLUtils.isElementPath(root, "<applet-desc>")) {
	    type = LaunchDesc.APPLET_DESC_TYPE;
	    applet = buildAppletDesc(source, codebase, root);
        } else {
	    throw new MissingFieldException(source, "<jnlp>(<application-desc>|<applet-desc>|<installer-desc>|<component-desc>)");
        }


        info = buildInformationDesc(source, codebase, root);
        resources = buildResourcesDesc(source, codebase, root, false);

        LaunchDesc launchDesc = new LaunchDesc(
	    specVersion,
	    codebase,
	    href,
	    version,
	    info,
	    security,
	    resources,
	    type,
	    application,
	    applet,
	    libraryDef,
	    installerDef,
	    internalCommand,
	    source,
	    bits);

	Trace.println("returning LaunchDesc from XMLFormat.parse():\n" +
			launchDesc, TraceLevel.TEMP);

        return launchDesc;
    }

    /** Create a combine informationDesc in the two informationDesc. The information present in id1
     *  overwrite the information present in id2
     */
    static private InformationDesc combineInformationDesc(InformationDesc id1, InformationDesc id2) {
        if (id1 == null) return id2;
        if (id2 == null) return id1;

        String title  = (id1.getTitle() != null) ? id1.getTitle() : id2.getTitle();
        String vendor = (id1.getVendor() != null) ? id1.getVendor() : id2.getVendor();
        URL    home   = (id1.getHome() != null) ? id1.getHome() :  id2.getHome();

        /** Copy descriptions */
        String[] descriptions = new String[InformationDesc.NOF_DESC];
        for(int i = 0; i < descriptions.length; i++) {
	    descriptions[i] = (id1.getDescription(i) != null) ? id1.getDescription(i) : id2.getDescription(i);
        }

        /** Icons */
        ArrayList iconList = new ArrayList();
        if (id2.getIcons() != null) iconList.addAll(Arrays.asList(id2.getIcons()));
        if (id1.getIcons() != null) iconList.addAll(Arrays.asList(id1.getIcons()));
        IconDesc[] icons = new IconDesc[iconList.size()];
        icons = (IconDesc[])iconList.toArray(icons);

        // If one of them say true, it is true
        boolean offline = id1.supportsOfflineOperation() || id1.supportsOfflineOperation() ;

	ShortcutDesc hints = (id1.getShortcut() != null) ? id1.getShortcut() : id2.getShortcut();

	AssociationDesc[] asd = ( AssociationDesc[] ) addArrays(
	    (Object[])id1.getAssociations(), (Object[])id2.getAssociations());

	RContentDesc[] rcd = ( RContentDesc[] ) addArrays(
	  (Object[])id1.getRelatedContent(), (Object[])id2.getRelatedContent());

        return new InformationDesc(title,
				   vendor,
				   home,
				   descriptions,
				   icons,
				   hints,
				   rcd,
				   asd,
				   offline);
    }

    /** Extract data from <information> tag */
    static private InformationDesc buildInformationDesc(final String source, final URL codebase, XMLNode root)
        throws MissingFieldException, BadFieldException {
        final ArrayList list = new ArrayList();

        // Iterates over all <information> nodes ignoring the type
        XMLUtils.visitElements(root,
	    "<information>", new XMLUtils.ElementVisitor() {
	    public void visitElement(XMLNode e) throws
		BadFieldException, MissingFieldException {

		// Locale info. (null if not present)
		String[] localeList = GeneralUtil.getStringList(
				XMLUtils.getAttribute(e, "", "locale"));

                // Check for right os, archictecture, and locale
                String[] os     = GeneralUtil.getStringList(
				XMLUtils.getAttribute(e, "", "os",   null));
                String[] arch   = GeneralUtil.getStringList(
				XMLUtils.getAttribute(e, "", "arch", null));
                String[] locale = GeneralUtil.getStringList(
				XMLUtils.getAttribute(e, "", "locale", null));
                String[] platform = GeneralUtil.getStringList(
				XMLUtils.getAttribute(e, "", "platform", null));
                if (GeneralUtil.prefixMatchStringList(
					os, Config.getOSName()) &&
		    GeneralUtil.prefixMatchStringList(
		    			arch, Config.getOSArch()) &&
		    GeneralUtil.prefixMatchStringList(
					platform, Config.getOSPlatform()) &&
                    matchDefaultLocale(locale))
		{
		    // Title, vendor, home
		    String title = XMLUtils.getElementContents(e, "<title>");
		    String vendor = XMLUtils.getElementContents(e, "<vendor>");
		    URL home = XMLUtils.getAttributeURL(
				source, codebase, e, "<homepage>", "href");

		    // Descriptions
		    String[] descriptions =
				new String[InformationDesc.NOF_DESC];
		    descriptions[InformationDesc.DESC_DEFAULT] =
		        XMLUtils.getElementContentsWithAttribute(
			e, "<description>", "kind", "", null);
		    descriptions[InformationDesc.DESC_ONELINE] =
		        XMLUtils.getElementContentsWithAttribute(
			e, "<description>", "kind", "one-line", null);
		    descriptions[InformationDesc.DESC_SHORT] =
		        XMLUtils.getElementContentsWithAttribute(
			e, "<description>", "kind", "short", null);
		    descriptions[InformationDesc.DESC_TOOLTIP] =
		        XMLUtils.getElementContentsWithAttribute(
			e, "<description>", "kind", "tooltip", null);

		    // Icons
		    IconDesc[] icons = getIconDescs(source, codebase, e);

		    // Shortcut hints
		    ShortcutDesc shortcuts = getShortcutDesc(e);

		    // related-content hints
		    RContentDesc[] relatedContents =
			getRContentDescs(source, codebase, e);

		    // association hints
		    AssociationDesc[] associations = getAssociationDesc(source, e);

		    list.add(new InformationDesc(
		        title, vendor, home, descriptions, icons,
			shortcuts, relatedContents, associations,
		        XMLUtils.isElementPath(e, "<offline-allowed>")));
		}
	    }
	});


        /* Combine all information desc. information in a single one for
	 * the current locale using the following prorities:
	 *   1. locale == language_country_variant
	 *   2. locale == lauguage_country
	 *   3. locale == lauguage
	 *   4. no or empty locale
	 */
        InformationDesc normId = new InformationDesc(null, null, null, null, null, null, null, null, false);

        for(int i = 0; i < list.size(); i++) {
	    InformationDesc id = (InformationDesc)list.get(i);
	    normId = combineInformationDesc(id, normId);
        }

        // If no <information><title> tag was found, throw exception
        if (normId.getTitle() == null) {
	    throw new MissingFieldException(source, "<jnlp><information><title>");
        }
        // If no <information> tag was found, throw exception
        if (normId.getVendor() == null) {
	    throw new MissingFieldException(source, "<jnlp><information><vendor>");
        }

        return normId;
    }


    static private Object[] addArrays (Object[] a1, Object[] a2) {
        if (a1 == null) return a2;
        if (a2 == null) return a1;
        ArrayList list = new ArrayList();
        int i;
        for (i=0; i<a1.length; list.add(a1[i++]));
        for (i=0; i<a2.length; list.add(a2[i++]));
        return (Object[]) (list.toArray(a1));
    }

    static public boolean matchDefaultLocale(String[] localeStr) {
	return GeneralUtil.matchLocale(localeStr, Globals.getDefaultLocale());
    }

    /** Extract data from <resources> tag. There is only one. */
    static final ResourcesDesc buildResourcesDesc(final String source, final URL codebase, XMLNode root, final boolean ignoreJres)
        throws MissingFieldException, BadFieldException {
        // Extract classpath directives
        final ResourcesDesc rdesc = new ResourcesDesc();

        // Iterate over all entries
        XMLUtils.visitElements(root, "<resources>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException, BadFieldException {
			// Check for right os, archictecture, and locale
			String[] os     = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "os",   null));
			String[] arch   = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "arch", null));
			String[] locale = GeneralUtil.getStringList(XMLUtils.getAttribute(e, "", "locale", null));
			if (GeneralUtil.prefixMatchStringList(os, Config.getOSName()) &&
			    GeneralUtil.prefixMatchStringList(arch, Config.getOSArch()) &&
			    matchDefaultLocale(locale)) {
			    // Now visit all children in this node
			    XMLUtils.visitChildrenElements(e, new XMLUtils.ElementVisitor() {
					public void visitElement(XMLNode e2) throws MissingFieldException, BadFieldException {
					    handleResourceElement(source, codebase, e2, rdesc, ignoreJres);
					}
				    });
			}
		    }
		});
        return (rdesc.isEmpty()) ? null : rdesc;
    }

    private static IconDesc[] getIconDescs(final String source,
					   final URL codebase, XMLNode e)
		throws MissingFieldException, BadFieldException {
	final ArrayList answer = new ArrayList();
	XMLUtils.visitElements(e, "<icon>", new XMLUtils.ElementVisitor() {
	    public void visitElement(XMLNode icon) throws
		MissingFieldException, BadFieldException {
		String kindStr = XMLUtils.getAttribute(icon, "", "kind", "");
		URL href =
		    XMLUtils.getRequiredURL(source, codebase, icon, "", "href");
		String version =
		    XMLUtils.getAttribute(icon, "", "version", null);
		int height =
		    XMLUtils.getIntAttribute(source, icon, "", "height", 0);
		int width  =
		    XMLUtils.getIntAttribute(source, icon, "", "width", 0);
		int depth  =
		    XMLUtils.getIntAttribute(source, icon, "", "depth", 0);

		// Convert the size
		int kind   = IconDesc.ICON_KIND_DEFAULT;
		if (kindStr.equals("selected")) {
		    kind = IconDesc.ICON_KIND_SELECTED;
		} else if (kindStr.equals("disabled")) {
		    kind = IconDesc.ICON_KIND_DISABLED;
		} else if (kindStr.equals("rollover")) {
		    kind = IconDesc.ICON_KIND_ROLLOVER;
		} else if (kindStr.equals("splash")) {
		    kind = IconDesc.ICON_KIND_SPLASH;
		}
		answer.add(
		    new IconDesc(href, version, height, width, depth, kind));
	    }
	});
	return (IconDesc[])answer.toArray(new IconDesc[answer.size()]);
    }

    private static ShortcutDesc getShortcutDesc(XMLNode e)
		throws MissingFieldException, BadFieldException {
        final ArrayList shortcuts = new ArrayList();

        XMLUtils.visitElements(e, "<shortcut>", new XMLUtils.ElementVisitor() {
            public void visitElement(XMLNode shortcutNode)
                throws MissingFieldException, BadFieldException {
                String online =
		    XMLUtils.getAttribute(shortcutNode, "", "online", "true");
                boolean onlineHinted = online.equalsIgnoreCase("true");
                boolean desktopHinted =
		    XMLUtils.isElementPath(shortcutNode, "<desktop>");
                boolean menuHinted =
		    XMLUtils.isElementPath(shortcutNode, "<menu>");
                String submenuHinted =
		    XMLUtils.getAttribute(shortcutNode, "<menu>", "submenu");
                shortcuts.add(new ShortcutDesc(onlineHinted,
                        desktopHinted, menuHinted, submenuHinted));
            }
        });

        if (shortcuts.size() > 0) {
            return (ShortcutDesc) shortcuts.get(0);
        }
	return null;
    }

    private static AssociationDesc[] getAssociationDesc(final String source, XMLNode e)
		throws MissingFieldException, BadFieldException {
        final ArrayList answer = new ArrayList();
        XMLUtils.visitElements(e, "<association>",
	    new XMLUtils.ElementVisitor() {
            public void visitElement(XMLNode node)
                throws MissingFieldException, BadFieldException {
		String extensions =
		    XMLUtils.getRequiredAttribute(source, node, "", "extensions");
		String mimeType =
		    XMLUtils.getRequiredAttribute(source, node, "", "mime-type");     
		answer.add(new AssociationDesc(extensions, mimeType));
	    }
	});
	return (AssociationDesc[])answer.toArray(
		new AssociationDesc[answer.size()]);
    }

    private static RContentDesc[] getRContentDescs(final String source,
                                           final URL codebase, XMLNode e)
		throws MissingFieldException, BadFieldException {
        final ArrayList answer = new ArrayList();
        XMLUtils.visitElements(e, "<related-content>",
            new XMLUtils.ElementVisitor() {
            public void visitElement(XMLNode node)
                throws MissingFieldException, BadFieldException {
		URL href =
                    XMLUtils.getRequiredURL(source, codebase, node, "", "href");
                String title = XMLUtils.getElementContents(node, "<title>");
                String description =
		    XMLUtils.getElementContents(node, "<description>");
		URL icon = XMLUtils.getAttributeURL(
                                source, codebase, node, "<icon>", "href");
                answer.add(new RContentDesc(href, title, description, icon));
            }
        });
        return (RContentDesc[])answer.toArray(new RContentDesc[answer.size()]);
    }


    /** Handle the individual entries in a resource desc */
    private static void handleResourceElement(String source, URL codebase,
	XMLNode e, ResourcesDesc rdesc, boolean ignoreJres) throws
	MissingFieldException, BadFieldException
    {
        String tag = e.getName();

        if (tag.equals("jar") || tag.equals("nativelib")) {
	    /*
	     * jar/nativelib elements
	     */
	    URL href = XMLUtils.getRequiredURL(source, codebase, e, "", "href");
	    String vd = XMLUtils.getAttribute(e, "", "version", null);
	    String kindStr = XMLUtils.getAttribute(e, "", "download");
	    String mainStr = XMLUtils.getAttribute(e, "", "main");
	    String part    = XMLUtils.getAttribute(e, "", "part");
	    int    size    = XMLUtils.getIntAttribute(source, e, "", "size", 0);
	    boolean isNativeLib = tag.equals("nativelib");
	    boolean isLazy = false;
	    boolean isMain = false;
	    if ("lazy".equalsIgnoreCase(kindStr)) isLazy = true;
	    if ("true".equalsIgnoreCase(mainStr)) isMain = true;
	    JARDesc jd = new JARDesc(
		  href, vd, isLazy, isMain, isNativeLib, part, size, rdesc);
	    rdesc.addResource(jd);
        } else if (tag.equals("property")) {
	    /*
	     *  property tag
	     */
	    String name  = XMLUtils.getRequiredAttribute(source, e, "", "name");
	    String value = XMLUtils.getRequiredAttributeEmptyOK(
				source, e, "", "value");
	    rdesc.addResource(new PropertyDesc(name, value));
        } else if (tag.equals("package")) {
	    /*
	     * package tag
	     */
	    String name = XMLUtils.getRequiredAttribute(source, e, "", "name");
	    String part = XMLUtils.getRequiredAttribute(source, e, "", "part");
	    String recurs = XMLUtils.getAttribute(e, "", "recursive", "false");
	    boolean isRecursive = "true".equals(recurs);
	    rdesc.addResource(new PackageDesc(name, part, isRecursive));
        } else if (tag.equals("extension")) {
	    String name = XMLUtils.getAttribute(e, "", "name");
	    URL href = XMLUtils.getRequiredURL(source, codebase, e, "", "href");
	    String vd = XMLUtils.getAttribute(e, "", "version", null);
	    // Iterate through ext=download resources
	    ExtDownloadDesc[] eds = getExtDownloadDescs(source, e);
	    rdesc.addResource(new ExtensionDesc(name, href, vd, eds));
        } else if (tag.equals("j2se") && !ignoreJres) {
	    /*
	     * j2se element
	     */
	    String version  =
		XMLUtils.getRequiredAttribute(source, e, "", "version");

	    // Get optional href attribute (vendor specific URL)
	    URL href =
		XMLUtils.getAttributeURL(source, codebase, e, "", "href");

	    String minheapstr =
		XMLUtils.getAttribute(e, "", "initial-heap-size");
	    String maxheapstr =
		XMLUtils.getAttribute(e, "", "max-heap-size");

	    String vmargs =
		XMLUtils.getAttribute(e, "", "java-vm-args");
	    long minheap = -1;
	    long maxheap = -1;
	    minheap = GeneralUtil.heapValToLong(minheapstr);
	    maxheap = GeneralUtil.heapValToLong(maxheapstr);

	    ResourcesDesc cbs = buildResourcesDesc(source, codebase, e, true);

	    // JRE
	    JREDesc jreDesc = new JREDesc(
		version,
		minheap,
		maxheap,
		vmargs,
		href,
		cbs);

	    rdesc.addResource(jreDesc);
        }
    }

    /** Parses the ExtDownloadDesc[] elements */
    static private ExtDownloadDesc[] getExtDownloadDescs(final String source, XMLNode root)
        throws BadFieldException, MissingFieldException {
        final ArrayList al = new ArrayList();

        XMLUtils.visitElements(root, "<ext-download>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException {
			String extPart = XMLUtils.getRequiredAttribute(source, e, "", "ext-part");
			String part = XMLUtils.getAttribute(e, "", "part");
			String download = XMLUtils.getAttribute(e, "", "download", "eager");
			boolean isLazy = "lazy".equals(download);
			al.add(new ExtDownloadDesc(extPart, part, isLazy));
		    }
		});
        ExtDownloadDesc[] eds = new ExtDownloadDesc[al.size()];
        return (ExtDownloadDesc[])al.toArray(eds);
    }

    /** Extract data from the application-desc tag */
    static private ApplicationDesc buildApplicationDesc(final String source, XMLNode root) throws MissingFieldException, BadFieldException {
        String mainclass = XMLUtils.getAttribute(root, "<application-desc>", "main-class");

        final ArrayList al1 = new ArrayList();
        XMLUtils.visitElements(root, "<application-desc><argument>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException, BadFieldException {
			String arg = XMLUtils.getElementContents(e, "", null);
			if (arg == null) {
			    throw new BadFieldException(source, XMLUtils.getPathString(e), "");
			}

			al1.add(arg);
		    }
		});

        String[] args1 = new String[al1.size()];
        args1 = (String[])al1.toArray(args1);

        return new ApplicationDesc(mainclass, args1);
    }


    /** Extract data from the application-desc tag */
    static private LibraryDesc buildLibraryDesc(final String source,
	XMLNode root) throws MissingFieldException, BadFieldException {
        String uniqueId =
	    XMLUtils.getAttribute(root, "<component-desc>", "unique-id");

        return new LibraryDesc(uniqueId);
    }

    /** Extract data from the extension-desc tag */
    static private InstallerDesc buildInstallerDesc(String source, URL codebase, XMLNode root)
        throws MissingFieldException, BadFieldException {
        String mainclass = XMLUtils.getAttribute(root, "<installer-desc>", "main-class");
        return new InstallerDesc(mainclass);
    }

    /** Extract data from the applet tag */
    static private AppletDesc buildAppletDesc(final String source, URL codebase, XMLNode root) throws MissingFieldException, BadFieldException {
        String appletclass  = XMLUtils.getRequiredAttribute(source, root, "<applet-desc>", "main-class");
        String name         = XMLUtils.getRequiredAttribute(source, root, "<applet-desc>", "name");
        URL    documentBase = XMLUtils.getAttributeURL(source, codebase, root, "<applet-desc>", "documentbase");
        int    width        = XMLUtils.getRequiredIntAttribute(source, root, "<applet-desc>", "width");
        int    height       = XMLUtils.getRequiredIntAttribute(source, root, "<applet-desc>", "height");

        if (width  <= 0) throw new BadFieldException(source, XMLUtils.getPathString(root)+"<applet-desc>width",   new Integer(width).toString());
        if (height <= 0) throw new BadFieldException(source, XMLUtils.getPathString(root)+"<applet-desc>height",  new Integer(height).toString());

        final Properties params = new Properties();

        XMLUtils.visitElements(root, "<applet-desc><param>", new XMLUtils.ElementVisitor() {
		    public void visitElement(XMLNode e) throws MissingFieldException, BadFieldException {
			String pn = XMLUtils.getRequiredAttribute(
					source, e, "", "name");
			String pv = XMLUtils.getRequiredAttributeEmptyOK(
					source, e, "", "value");
			params.setProperty(pn, pv);
		    }
		});

        return new AppletDesc(name, appletclass, documentBase, width, height, params);
    }
}

