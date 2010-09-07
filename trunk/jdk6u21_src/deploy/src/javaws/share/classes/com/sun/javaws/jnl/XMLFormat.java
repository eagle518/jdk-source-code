/*
 * @(#)XMLFormat.java	1.77 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;

import com.sun.javaws.util.GeneralUtil;
import java.io.IOException;
import java.net.URL;
import java.net.URI;
import java.net.MalformedURLException;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Properties;
import com.sun.javaws.Globals;
import com.sun.javaws.exceptions.MissingFieldException;
import com.sun.javaws.exceptions.BadFieldException;
import com.sun.javaws.exceptions.JNLParseException;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.xml.XMLEncoding;
import com.sun.deploy.xml.XMLParser;
import com.sun.deploy.xml.XMLNode;
import com.sun.deploy.xml.BadTokenException;
import com.sun.deploy.cache.AssociationDesc;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.Environment;
import java.io.File;
import java.net.URISyntaxException;

public class XMLFormat {

    /**
     * thisCodebase, if set, is used to determine the codebase, if JNLP codebase is not absolute
     *                 thisCodebase is always NULL in case of JAVAWS launch
     *
     * @param thisCodebase Optional base URL of this JNLPDesc location, when launched from a browser 
     * @param documentbaseBrowser Optional browsers documentBase
     */
    public static LaunchDesc parse(byte[] bits, URL thisCodebase, 
            URL documentbaseBrowser, URL originalRequestHref)
       throws IOException, BadFieldException, MissingFieldException, JNLParseException {
       return parse(bits, thisCodebase, documentbaseBrowser,
               originalRequestHref, LaunchSelection.createDefaultMatchJRE());
    }

    public static LaunchDesc parse(byte[] bits, URL thisCodebase, URL documentbaseBrowser,
            URL originalRequestHref, LaunchSelection.MatchJREIf matchImpl)
       throws IOException, BadFieldException, MissingFieldException, JNLParseException {

        String source;
        XMLNode root;
        XMLParser parser;

        try {
            source = XMLEncoding.decodeXML(bits);
        } catch (Exception e) {
            throw new JNLParseException(null, e,
                "exception determining encoding of jnlp file", 0);
        }

        // we don't throw BadTokenException in XMLParser with minor syntax errors
        // The exception is saved in the parser. When the parsed JNLP file is so malformed
        // that it can not be used as a valid LaunchDesc, the saved the exception will be thrown
        // because it is closer to the root cause.
        // The rational here is that we don't want to break existing minor malformed jnlp
        // files as long as they are usable.
        try {
            parser = new XMLParser(source);
            root = parser.parse();
        } catch (BadTokenException bte) {
            throw new JNLParseException(source, bte,
                        "wrong kind of token found", bte.getLine());
        } catch (Exception e) {
            throw new JNLParseException(source, e,
                        "exception parsing jnlp file", 0);
        }

        InformationDesc info = null;
        ResourcesDesc resources = null;
        UpdateDesc update = null;
        ApplicationDesc application = null;
        AppletDesc applet = null;
        LibraryDesc libraryDef = null;
        InstallerDesc installerDef = null;
        String internalCommand = null;

        if (root == null || root.getName() == null) {
            throw new JNLParseException(source, null, null, 0);
        }

        // Check if we should launch the player or viewer
        if (root.getName().equals("player") ||
                root.getName().equals("viewer")) {
            String cpTab = XMLUtils.getAttribute(root, null, "tab");
            return LaunchDescFactory.buildInternalLaunchDesc(
                    root.getName(), source, cpTab);
        }

        // Check that root element is a <jnlp> tag
        if (!root.getName().equals("jnlp")) {
            throwNewException(source, parser, new MissingFieldException(source, "<jnlp>"));
        }

        // Read <jnlp> attributes (path is empty, i.e., "")
        // (spec, version, codebase, href)
        String specVersion = XMLUtils.getAttribute(root, "", "spec", "1.0+");
        String version = XMLUtils.getAttribute(root, "", "version");

        // Make sure the codebase URL ends with a '/'.
        //
        // Regarding the JNLP spec,
        // the thisCodebase is used to determine the codebase.
        //      codebase = new URL(thisCodebase, codebase)
        // thisCodebase is always NULL in case of JAVAWS launch
        URL codebase = URLUtil.asPathURL(XMLUtils.getAttributeURL(source, thisCodebase, root, "", "codebase"));
        if( codebase == null && thisCodebase!=null ) {
            codebase=thisCodebase;
        }
        // for java web start, it is okay for codebase to be omitted in jnlp 
        // for java plugin, codebase should be available from the browser, even 
        // if it is ommited in the jnlp file 
        if (codebase==null && Environment.isJavaPlugin()) { 
            throwNewException(source, parser, new MissingFieldException(source, "<jnlp>(<codebase>|InternalError(thisCodebase)"));
        }

        URL documentBase = XMLUtils.getAttributeURL(source, codebase, root, "<applet-desc>", "documentbase");

        if (documentbaseBrowser!=null) {
            documentBase=documentbaseBrowser;  // take over browsers's documentbase, unconditional
        }

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
        int type = 0;
        if (XMLUtils.isElementPath(root, "<application-desc>")) {
            type = LaunchDesc.APPLICATION_DESC_TYPE;
            application = buildApplicationDesc(source, root);
        } else if (XMLUtils.isElementPath(root, "<component-desc>")) {
            type = LaunchDesc.LIBRARY_DESC_TYPE;
            libraryDef = buildLibraryDesc(source, root);
        } else if (XMLUtils.isElementPath(root, "<installer-desc>")) {
            if (Cache.isCacheEnabled() == false) {
                throwNewException(source, parser, new BadFieldException(source, "<installer-desc>", ""));
            }
            type = LaunchDesc.INSTALLER_DESC_TYPE;
            installerDef = buildInstallerDesc(source, codebase, root);
        } else if (XMLUtils.isElementPath(root, "<applet-desc>")) {
            type = LaunchDesc.APPLET_DESC_TYPE;
            applet = buildAppletDesc(source, codebase, documentBase, root);
        } else {
            throwNewException(source, parser,
                    new MissingFieldException(source, "<jnlp>(<application-desc>|<applet-desc>|<installer-desc>|<component-desc>)"));
        }

        update = getUpdateDesc(root);

        info = buildInformationDesc(source, codebase, root);
      
        resources = buildResourcesDesc(source, codebase, root, false);

        LaunchDesc launchDesc = new LaunchDesc(
            specVersion,
            codebase,
            href,
            version,
            info,
            security,
            update,
            resources,
            type,
            application,
            applet,
            libraryDef,
            installerDef,
            internalCommand,
            source,
            bits,
            matchImpl);

        //In some cases href inside the xml file
        //is different from one we use to download it or get it from the cache.
        //Keep actual href to be able to find it back
        launchDesc.setSourceURL(originalRequestHref);

        if (Trace.isTraceLevelEnabled(TraceLevel.TEMP)) {
            Trace.println("returning LaunchDesc from XMLFormat.parse():\n" +
                        launchDesc, TraceLevel.TEMP);
        }

        return launchDesc;
    }

    /**
     * Helper method to check if there is saved BadTokenException need be thrown
     * @param source the LaunchDesc jnlp source
     * @param parser the XMLParser instance
     * @param e the exception to thrown if no saved BadTokenException
     */
    static private void throwNewException(String source, XMLParser parser, Exception e)
            throws JNLParseException, MissingFieldException, BadFieldException {
        BadTokenException bte = parser.getSavedException();
        if (bte != null) {
            Trace.println("JNLP Parse Exception: "+bte, TraceLevel.TEMP);
            throw new JNLParseException(source, bte,
                    "wrong kind of token found", bte.getLine());
        } else {
            if (e instanceof MissingFieldException) throw (MissingFieldException)e;
            if (e instanceof BadFieldException) throw (BadFieldException)e;
        }
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
        boolean offline = id1.supportsOfflineOperation() || id2.supportsOfflineOperation() ;

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
                    AssociationDesc[] associations = getAssociationDesc(
                                                        source, codebase, e);

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

        if (!rdesc.isEmpty()) {
            boolean pack200Enabled = rdesc.isPack200Enabled();
            boolean versionEnabled = rdesc.isVersionEnabled();
            if (pack200Enabled || versionEnabled) {
                JARDesc[] jds = rdesc.getLocalJarDescs();
                for (int i = 0; i < jds.length; i++) {
                    JARDesc jd = jds[i];
                    if (pack200Enabled) {
                        jd.setPack200Enabled();
                    }
                    if (versionEnabled) {
                        jd.setVersionEnabled();
                    }
                }
            }
        }
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
                } else if (kindStr.equals("shortcut")) {
                    kind = IconDesc.ICON_KIND_SHORTCUT;
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

    private static UpdateDesc getUpdateDesc(XMLNode e)
                throws MissingFieldException, BadFieldException {
        final ArrayList updates = new ArrayList();

        XMLUtils.visitElements(e, "<update>", new XMLUtils.ElementVisitor() {
            public void visitElement(XMLNode node)
                throws MissingFieldException, BadFieldException {
                String check =
                    XMLUtils.getAttribute(node, "", "check", "timeout");
                String policy = 
                    XMLUtils.getAttribute(node, "", "policy", "always");

                updates.add(new UpdateDesc(check, policy));
            }
        });

        if (updates.size() > 0) {
            return (UpdateDesc) updates.get(0);
        }

        // return a default Update Descriptor
        return new UpdateDesc("timeout", "always");
    }

    private static AssociationDesc[] getAssociationDesc(final String source, 
        final URL codebase, XMLNode e)
                throws MissingFieldException, BadFieldException {
        final ArrayList answer = new ArrayList();
        XMLUtils.visitElements(e, "<association>",
            new XMLUtils.ElementVisitor() {
            public void visitElement(XMLNode node)
                throws MissingFieldException, BadFieldException {

                String extensions = XMLUtils.getAttribute(
                                       node, "", "extensions");
        
                String mimeType = XMLUtils.getAttribute(
                                       node, "", "mime-type");
                String description = XMLUtils.getElementContents(
                                        node, "<description>");

                URL icon = XMLUtils.getAttributeURL(
                                source, codebase, node, "<icon>", "href");
                
                 if (extensions == null && mimeType == null) {
                    throw new MissingFieldException(source,
                                 "<association>(<extensions><mime-type>)");
                } else if (extensions == null) {
                    throw new MissingFieldException(source,
                                     "<association><extensions>");
                } else if (mimeType == null) {
                    throw new MissingFieldException(source,
                                     "<association><mime-type>");
                } 
                 
                // don't support uppercase extension and mime-type on gnome.
                if ("gnome".equals(System.getProperty("sun.desktop"))) {
                    extensions = extensions.toLowerCase();
                    mimeType = mimeType.toLowerCase();
                }

                 answer.add(new AssociationDesc(extensions, mimeType, 
                                                description, icon));
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
            if (Cache.isCacheEnabled() == false && isNativeLib) {
                throw new BadFieldException(source, "nativelib", href.toString());
            }
            boolean isLazy = false;
            boolean isMain = false;
            boolean isProgress = false;
            if ("lazy".equalsIgnoreCase(kindStr)) isLazy = true;
            if ("progress".equalsIgnoreCase(kindStr)) isProgress = true;
            if ("true".equalsIgnoreCase(mainStr)) isMain = true;
            JARDesc jd = null;
            if (Environment.isImportMode() && 
                    Environment.getImportModeCodebaseOverride() != null && 
                    href.toString().endsWith("/")) {
                // if jar href is a directory, import all files under the direcotry
                String jarHref = XMLUtils.getAttribute(e, "", "href");
                File file = null;
                try {
                    URI fileURI = new URI(
                            Environment.getImportModeCodebaseOverride().toString().replace("\\", "/") +
                            XMLUtils.getAttribute(e, "", "href"));
                    file = new File(fileURI);
                    
                } catch (URISyntaxException use) {
                    Trace.ignoredException(use);
                }
                if (file != null && file.isDirectory()) {
                    File[] files = file.listFiles();
                    for (int i=0; i< files.length; i++) {
                        try {
                            URL newHref = new URL(href.toString() +
                                    files[i].getName());
                            jd = new JARDesc(
                                    newHref, vd, isLazy, isMain, isNativeLib,
                                    part, size, rdesc, isProgress);                           
                            rdesc.addResource(jd);                           
                        } catch (MalformedURLException mue) {
                            Trace.ignoredException(mue);
                        }
                    }
                }
            } else {
                jd = new JARDesc(href, vd, isLazy, isMain, isNativeLib, 
                                 part, size, rdesc, isProgress);                           
                rdesc.addResource(jd); 
            }
           
        } else if (tag.equals("property")) {
            /*
             *  property tag
             */
            String name  = XMLUtils.getRequiredAttribute(source, e, "", "name");
            String value = XMLUtils.getRequiredAttributeEmptyOK(
                    source, e, "", "value");
            
            // Special case following jnlp properties. 
            // Don't set them as system properties. Instead store in LaunchDesc or JARDesc
            if (name.equals("jnlp.versionEnabled") && value.equalsIgnoreCase("true")) {
                rdesc.setVersionEnabled();
            } else if (name.equals("jnlp.packEnabled") && value.equalsIgnoreCase("true")) {
                rdesc.setPack200Enabled();
            } else if (name.equals("jnlp.concurrentDownloads")) {
                if (value != null) {
                    int n = 0;
                    try {
                        n = Integer.parseInt(value.trim());
                    } catch (NumberFormatException nfe) {
                    }
                    rdesc.setConcurrentDownloads(n);
                }
            } else {
                rdesc.addResource(new PropertyDesc(name, value));
            }
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
        } else if ((tag.equals("java") || tag.equals("j2se")) && !ignoreJres) {
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
    static private ApplicationDesc buildApplicationDesc(final String source, XMLNode root) 
        throws MissingFieldException, BadFieldException {
        String mainclass = XMLUtils.getAttribute(root, 
                           "<application-desc>", "main-class");
        String progressclass  = XMLUtils.getAttribute(root,
                                "<application-desc>", "progress-class");

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

        return new ApplicationDesc(mainclass, progressclass, args1);
    }


    /** Extract data from the application-desc tag */
    static private LibraryDesc buildLibraryDesc(final String source,
        XMLNode root) throws MissingFieldException, BadFieldException {
        String progressclass  = XMLUtils.getAttribute(root,
                                "<component-desc>", "progress-class");
        return new LibraryDesc(progressclass);
    }

    /** Extract data from the extension-desc tag */
    static private InstallerDesc buildInstallerDesc(String source, URL codebase, XMLNode root)
        throws MissingFieldException, BadFieldException {
        String mainclass = XMLUtils.getAttribute(root, "<installer-desc>", "main-class");
        return new InstallerDesc(mainclass);
    }

    /** Extract data from the applet tag */
    static private AppletDesc buildAppletDesc(final String source, URL codebase, URL documentBase, XMLNode root) throws MissingFieldException, BadFieldException {
        String appletclass  = XMLUtils.getRequiredAttribute(source, root, "<applet-desc>", "main-class");
        String progressclass  = XMLUtils.getAttribute(root, "<applet-desc>", "progress-class");
        String name         = XMLUtils.getRequiredAttribute(source, root, "<applet-desc>", "name");
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

        return new AppletDesc(name, appletclass, documentBase, width, height, params, progressclass);
    }
}

