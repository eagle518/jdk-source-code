/*
 * @(#)PackageUseWriter.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;
import java.io.*;
import java.util.*;

/**
 * Generate package usage information.
 *
 * @author Robert G. Field
 */
public class PackageUseWriter extends SubWriterHolderWriter {
    
    final PackageDoc pkgdoc;
    final SortedMap usingPackageToUsedClasses = new TreeMap();
    
    /**
     * Constructor.
     *
     * @param filename the file to be generated.
     * @throws IOException
     * @throws DocletAbortException
     */
    public PackageUseWriter(ConfigurationImpl configuration,
                            ClassUseMapper mapper, String filename,
                            PackageDoc pkgdoc) throws IOException {
        super(configuration, DirectoryManager.getDirectoryPath(pkgdoc),
              filename,
              DirectoryManager.getRelativePath(pkgdoc.name()));
        this.pkgdoc = pkgdoc;
        
        // by examining all classes in this package, find what packages
        // use these classes - produce a map between using package and
        // used classes.
        ClassDoc[] content = pkgdoc.allClasses();
        for (int i = 0; i < content.length; ++i) {
            ClassDoc usedClass = content[i];
            Set usingClasses = (Set)mapper.classToClass.get(usedClass.qualifiedName());
            if (usingClasses != null) {
                for (Iterator it = usingClasses.iterator(); it.hasNext(); ) {
                    ClassDoc usingClass = (ClassDoc)it.next();
                    PackageDoc usingPackage = usingClass.containingPackage();
                    Set usedClasses = (Set)usingPackageToUsedClasses
                        .get(usingPackage.name());
                    if (usedClasses == null) {
                        usedClasses = new TreeSet();
                        usingPackageToUsedClasses.put(Util.getPackageName(usingPackage),
                                                      usedClasses);
                    }
                    usedClasses.add(usedClass);
                }
            }
        }
    }
    
    /**
     * Generate a class page.
     *
     * @param configuration the current configuration of the doclet.
     * @param mapper        the mapping of the class usage.
     * @param pkgdoc        the package doc being documented.
     */
    public static void generate(ConfigurationImpl configuration,
                                ClassUseMapper mapper, PackageDoc pkgdoc) {
        PackageUseWriter pkgusegen;
        String filename = "package-use.html";
        try {
            pkgusegen = new PackageUseWriter(configuration,
                                             mapper, filename, pkgdoc);
            pkgusegen.generatePackageUseFile();
            pkgusegen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                "doclet.exception_encountered",
                exc.toString(), filename);
            throw new DocletAbortException();
        }
    }
    
    
    /**
     * Print the class use list.
     */
    protected void generatePackageUseFile() throws IOException {
        printPackageUseHeader();
        
        if (usingPackageToUsedClasses.isEmpty()) {
            printText("doclet.ClassUse_No.usage.of.0", pkgdoc.name());
            p();
        } else {
            generatePackageUse();
        }
        
        printPackageUseFooter();
    }
    
    /**
     * Print the class use list.
     */
    protected void generatePackageUse() throws IOException {
        if (configuration.packages.length > 1) {
            generatePackageList();
        }
        generateClassList();
    }
    
    protected void generatePackageList() throws IOException {
        tableIndexSummary();
        tableHeaderStart("#CCCCFF");
        printText("doclet.ClassUse_Packages.that.use.0",
            getPackageLink(pkgdoc, Util.getPackageName(pkgdoc), false));
        tableHeaderEnd();
        Iterator it = usingPackageToUsedClasses.keySet().iterator();
        while (it.hasNext()) {
            PackageDoc pkg = configuration.root.packageNamed((String)it.next());
            generatePackageUse(pkg);
        }
        tableEnd();
        space();
        p();
    }
    
    protected void generateClassList() throws IOException {
        Iterator itp = usingPackageToUsedClasses.keySet().iterator();
        while (itp.hasNext()) {
            String packageName = (String)itp.next();
            PackageDoc usingPackage = configuration.root.packageNamed(packageName);
            if (usingPackage != null) {
                anchor(usingPackage.name());
            }
            tableIndexSummary();
            tableHeaderStart("#CCCCFF");
            printText("doclet.ClassUse_Classes.in.0.used.by.1",
                getPackageLink(pkgdoc, Util.getPackageName(pkgdoc), false), 
                getPackageLink(usingPackage,Util.getPackageName(usingPackage), false));
            tableHeaderEnd();
            Iterator itc =
                ((Collection)usingPackageToUsedClasses.get(packageName))
                .iterator();
            while (itc.hasNext()) {
                printClassRow((ClassDoc)itc.next(), packageName);
            }
            tableEnd();
            space();
            p();
        }
    }
    
    protected void printClassRow(ClassDoc usedClass, String packageName) {
        String path = pathString(usedClass,
                                 "class-use/" + usedClass.name() + ".html");
        
        trBgcolorStyle("white", "TableRowColor");
        summaryRow(0);
        bold();
        printHyperLink(path, packageName, usedClass.name(), true);
        boldEnd();
        println(); br();
        printNbsps();
        printIndexComment(usedClass);
        summaryRowEnd();
        trEnd();
    }
    
    /**
     * Print the package use list.
     */
    protected void generatePackageUse(PackageDoc pkg) throws IOException {
        trBgcolorStyle("white", "TableRowColor");
        summaryRow(0);
        //Just want an anchor here.
        printHyperLink("", pkg.name(), Util.getPackageName(pkg), true);
        summaryRowEnd();
        summaryRow(0);
        if (pkg != null) {
            printSummaryComment(pkg);
        }
        space();
        summaryRowEnd();
        trEnd();
    }
    
    /**
     * Print the header for the class use Listing.
     */
    protected void printPackageUseHeader() {
        String packageLabel = configuration.getText("doclet.Package");
        String name = pkgdoc.name();
        printHtmlHeader(configuration.getText("doclet.Window_ClassUse_Header",
            packageLabel, name), null, true);
        navLinks(true);
        hr();
        center();
        h2();
        boldText("doclet.ClassUse_Title", packageLabel, name);
        h2End();
        centerEnd();
    }
    
    /**
     * Print the footer for the class use Listing.
     */
    protected void printPackageUseFooter() {
        hr();
        navLinks(false);
        printBottom();
        printBodyHtmlEnd();
    }
    
    
    /**
     * Print this package link
     */
    protected void navLinkPackage() {
        navCellStart();
        printHyperLink("package-summary.html", "", configuration.getText("doclet.Package"),
                       true, "NavBarFont1");
        navCellEnd();
    }
    
    /**
     * Print class use link
     */
    protected void navLinkClassUse() {
        navCellRevStart();
        fontStyle("NavBarFont1Rev");
        boldText("doclet.navClassUse");
        fontEnd();
        navCellEnd();
    }
    
    protected void navLinkTree() {
        navCellStart();
        printHyperLink("package-tree.html", "", configuration.getText("doclet.Tree"),
                       true, "NavBarFont1");
        navCellEnd();
    }
    
}
