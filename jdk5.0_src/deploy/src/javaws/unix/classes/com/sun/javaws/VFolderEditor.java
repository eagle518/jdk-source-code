/*
 * @(#)VFolderEditor.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.*;

import com.sun.deploy.xml.*;
import com.sun.javaws.jnl.XMLUtils;
import com.sun.javaws.exceptions.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;

class VFolderEditor {

    private VFolderEditor() {}

    private final static String ALL_VFOLDER_INFO  = File.separator + "etc" + File.separator + "gnome" + File.separator + "gnome-vfs-2.0" + File.separator + "vfolders" + File.separator + "applications-all-users.vfolder-info";
    private final static String USER_VFOLDER_INFO = System.getProperty("user.home") + File.separator + ".gnome2" + File.separator + "vfolders" + File.separator + "applications.vfolder-info";
    private final static String RH8_VFOLDER_INFO  = File.separator + "etc" + File.separator + "X11" + File.separator + "desktop-menus" + File.separator + "applications.menu";

    private final static String appAlluserFolderinfo = "<Name>Applications</Name>\n<Parent>applications-all-users:///</Parent>\n";


    // extract XML declarations at the beginning of source
    private String extractXMLDeclaration(String source) {
	String xmlDecl = "";
	int start = source.indexOf("<?");
	int end = source.indexOf("?>");
	if (start == -1 || end == -1) return "";
	xmlDecl = source.substring(start, end + 2) + "\n";
	Trace.println("extract xmlDecl: " + xmlDecl, TraceLevel.BASIC);
	return xmlDecl;
    }

    // initialize the path to the current applications.vfolder-info file
    // for reading
    // on solaris, the file is under your home directory -
    //     <userhome>/.gnome2/vfolders
    // on RH linux 8.0, if there is no file under your home directory, we
    // use the default file at: /etc/X11/desktop-menus/
    private File getVfolderInfoFile(boolean allUsers) {
        File result = null;

        if (allUsers) {
            result = new File(ALL_VFOLDER_INFO);
        }
        else {
            result = new File(USER_VFOLDER_INFO);
        }

        if (result.exists() == false) {
            File parent = result.getParentFile();

            if (parent.exists() == false) {
                // try to create the parent directories
                try {
                    parent.mkdirs();
                }
                catch (SecurityException se) {
                    // the parent directories don't exist, and can't be created
                    result = null;
                }
            }
            else if (parent.canWrite() == false) {
                // don't have permission to write to this directory
                result = null;
            }
            // no else required; can write to the parent, so the file can be
            // created later

	    if (Config.getOSName().equals("Linux")) {
		// try the legacy RH8 directory	  
		File rh_vfile = new File(RH8_VFOLDER_INFO);
		
		// use the RH system file if it exist and is readable
		if ((rh_vfile.exists() == true) &&
		    (rh_vfile.canRead() == true)) {
		    result = rh_vfile;
		}
	    }
        }
	
	if (result != null) {
	    Trace.println("getVfolderInfoFile(" + allUsers + ") -> " + result.getAbsolutePath(), TraceLevel.BASIC);
	}

	return (result);
    }

    public static void  updateVFolderInfo(final String  folderName,
                                          final String  directoryPath,
                                          final String  desktopPath,
                                          final boolean allUsers,
                                          final boolean remove) {
	new VFolderEditor().updateVFolderInfoInternal(folderName,
                                                      directoryPath,
                                                      desktopPath,
                                                      allUsers,
                                                      remove);
    }

    // this is the main entry point to update the applications.vfolder-info
    // file.  if remove is false, it will add the folder with the directory
    // and desktop path.  if remove is true, it will remove the folder from
    // the vfolder-info file.
    private void updateVFolderInfoInternal(final String  folderName,
                                           final String  directoryPath,
                                           final String  desktopPath,
                                           final boolean allUsers,
                                           final boolean remove) {
	// this holds the generated application.vfolder-info xml content
	StringBuffer newVFolderInfoXML = new StringBuffer();

        File vfolderInfo = getVfolderInfoFile(allUsers);

        if (vfolderInfo == null) {
            Trace.println("VFolderFile update failed!", TraceLevel.BASIC);
        }
        else {
            int     len  = (int) vfolderInfo.length();
            XMLNode root = null;

            try {
                FileReader fr = new FileReader(vfolderInfo);
                char[] fileContents = new char[(int)len];
                fr.read(fileContents, 0, len);
                fr.close();

                String source = new String(fileContents);
                Trace.println("CURRENT VFolderFile: " + source, TraceLevel.BASIC);

                newVFolderInfoXML.append(extractXMLDeclaration(source));

                try {
                    root = (new XMLParser(source)).parse();
                    if (!(root.getName().equals("VFolderInfo"))) {
                        // bad applications.vfolder-info file
                        Trace.println("root node not found", TraceLevel.BASIC);
                        return;
                    }

                    updateVFolderInfoFile(root, folderName, directoryPath, desktopPath, allUsers, remove, newVFolderInfoXML);


                } catch (BadTokenException bte) {
                    // cannot parse the XML file, return
                    Trace.ignoredException(bte);
                    return;
                }

            } catch (FileNotFoundException fnfe) {
                Trace.ignoredException(fnfe);
                // create a new applications.vfolder-info
                createNewVFolderContents(folderName, directoryPath, desktopPath, allUsers, newVFolderInfoXML);
            } catch (IOException ioe) {
                // return
                Trace.ignoredException(ioe);
                return;
            }

            Trace.println("UPDATED VFolderFile: " + newVFolderInfoXML, TraceLevel.BASIC);
            writeVFolderFile(new File(USER_VFOLDER_INFO), newVFolderInfoXML);
        }
    }

    // create the contents of the applications.vfolder-info file from scratch
    private void createNewVFolderContents(final String  folderName,
                                          final String  directoryPath,
                                          final String  desktopPath,
                                          final boolean allUsers,
                                          StringBuffer  newVFolderInfoXML) {

	newVFolderInfoXML.append("<?xml version=\"1.0\"?>\n");
	newVFolderInfoXML.append("<VFolderInfo>\n");
        addNewFolderInfo(folderName, directoryPath, desktopPath, !allUsers, true, true, newVFolderInfoXML);
	newVFolderInfoXML.append("</VFolderInfo>\n");
    }

    // write out the application.vfolder-info file
    private void writeVFolderFile(File vfolderInfo, StringBuffer contents) {
	// always create a new file
	try {
            PrintWriter pw = new PrintWriter(new BufferedWriter(new FileWriter(vfolderInfo)));
	    pw.print(contents.toString());
	    pw.flush();
	    pw.close();
	} catch (IOException ioe) {
	    Trace.ignoredException(ioe);
	}
    }

    // add the Folder element and it's attribute to the current vfolder-info
    // contents
    private void addNewFolderInfo(final String folderName,
                                  final String directoryPath,
                                  final String desktopPath,
                                  boolean      addAppAllUserFolder,
                                  boolean      addNewFolder,
                                  boolean      addQuery,
                                  StringBuffer newVFolderInfoXML) {

	if (addAppAllUserFolder) {
	    newVFolderInfoXML.append("<Folder>\n");
	    newVFolderInfoXML.append(appAlluserFolderinfo);
	}
	if (addNewFolder) {
	    newVFolderInfoXML.append("<Folder>\n");
	    newVFolderInfoXML.append("<Name>" + folderName + "</Name>\n");
	}
	if (directoryPath != null) {
	    // fix 4907894
	    String newDirectoryPath = directoryPath.replaceAll("//", "/");
	    newVFolderInfoXML.append("<Desktop>" + newDirectoryPath + "</Desktop>\n");
	}

	if (addQuery) {
	    newVFolderInfoXML.append("<Query>\n<And>\n<Keyword>Application</Keyword>\n<Keyword>" + folderName + "</Keyword>\n</And>\n</Query>\n");
	}
	if (desktopPath != null) {
	    newVFolderInfoXML.append("<Include>" + desktopPath + "</Include>\n");
	}
	if (addNewFolder) {
	    newVFolderInfoXML.append("</Folder>\n");
	}
	if (addAppAllUserFolder) {
	    newVFolderInfoXML.append("</Folder>\n");
	}

    }


    // this methods should be called with the root node for the applications
    // folder, and it will go through all the folders under it, and perform
    // add/remove of the specified folder entry accordingly.
    private void searchForFolder(XMLNode e, final String folderName, final String directoryPath, final String desktopPath, final boolean remove, final Boolean[] updated, final StringBuffer newVFolderInfoXML) {

	final Boolean[] folderFound = new Boolean[1];
	folderFound[0] = Boolean.FALSE;
	final Boolean[] addQuery = new Boolean[1];
	addQuery[0] = Boolean.TRUE;


	while (e != null) {
	    Trace.println("searchForFolder: XMLNode: " + e, TraceLevel.BASIC);
	    newVFolderInfoXML.append("<" + e.getName() + ">\n");



	    try {
		XMLUtils.visitChildrenElements(e, new XMLUtils.ElementVisitor() {

			public void visitElement(XMLNode e) {

			    boolean addTag = true;
			    String tag = "";

			    if (e.getName().equals("Name") && e.getNested().toString().equals(folderName)) {
				folderFound[0] = Boolean.TRUE;
			    }

			    if (folderFound[0].booleanValue() && e.getName().equals("Include") && e.getNested().toString().indexOf(desktopPath + ".desktop") != -1 && remove) {
				updated[0] = Boolean.TRUE;
				return;
			    }

			    if (e.getNested() == null) {
				tag += "<" + e.getName() + "/>\n";
			    } else {
				tag += "<" + e.getName() + ">" + e.getNested().toString() + "</" + e.getName() + ">\n";
			    }



			    if (folderFound[0].booleanValue() && e.getName().equals("Query") &&
				e.getNested().toString().indexOf(folderName) != -1) {

				addQuery[0] = Boolean.FALSE;
			    }

			    if (folderFound[0].booleanValue() && directoryPath != null &&
				e.getName().equals("Desktop") && !remove) {
				addTag = false;
			    }
			    if (folderFound[0].booleanValue() && desktopPath != null &&
				e.getName().equals("Include") &&
				e.getNested().toString().equals(desktopPath)) {
				addTag = false;
			    }
			    if (addTag) {

				newVFolderInfoXML.append(tag);

			    }
			}
		    });
	    } catch (BadFieldException bfe) {
		Trace.ignoredException(bfe);
	    } catch (MissingFieldException mfe) {
		Trace.ignoredException(mfe);
	    }




	    if (folderFound[0].booleanValue() && !(updated[0].booleanValue())) {

		addNewFolderInfo(folderName, directoryPath, desktopPath, false, false, addQuery[0].booleanValue(), newVFolderInfoXML);
		updated[0] = Boolean.TRUE;


		folderFound[0] = Boolean.FALSE;
	    }

	    newVFolderInfoXML.append("</" + e.getName() + ">\n");
	    removeEmptyFolder(newVFolderInfoXML, directoryPath);
	    e = e.getNext();
	}
	if (!(updated[0].booleanValue()) && !remove) {
	    addNewFolderInfo(folderName, directoryPath, desktopPath, false, true, addQuery[0].booleanValue(), newVFolderInfoXML);
	    updated[0] = Boolean.TRUE;
	}


    }

    // Remove empty folder contents (<Folder></Folder>)
    private void removeEmptyFolder(StringBuffer newVFolderInfoXML, final String directoryFilePath) {
	String ftag = "<Folder>";
	int start = newVFolderInfoXML.lastIndexOf("<Folder>");
	int end = newVFolderInfoXML.lastIndexOf("</Folder>");
	// only remove the folder if directory path matches
	if (newVFolderInfoXML.substring(start, end).indexOf("<Include>") == -1 && newVFolderInfoXML.substring(start, end).indexOf(directoryFilePath) != -1 && newVFolderInfoXML.substring(start, end).indexOf("application-all-users:///") == -1) {
	    newVFolderInfoXML.delete(start, newVFolderInfoXML.length());
	    if (directoryFilePath != null) {
	
		new File(directoryFilePath).delete();
		Trace.println("file removed: " + directoryFilePath, TraceLevel.BASIC);
	    }
	} else if (newVFolderInfoXML.lastIndexOf("</Folder>") - newVFolderInfoXML.lastIndexOf("<Folder>") == (ftag.length() + 1)) {
	    String a = newVFolderInfoXML.substring(newVFolderInfoXML.lastIndexOf("<Folder>"), newVFolderInfoXML.length()).trim();

	    if (a.equals("<Folder>\n</Folder>")) {


		newVFolderInfoXML.delete(newVFolderInfoXML.lastIndexOf("<Folder>"), newVFolderInfoXML.length());

	    }
	}

    }

    private void updateVFolderInfoFile(XMLNode            e,
                                       final String       folderName,
                                       final String       directoryPath,
                                       final String       desktopPath,
                                       final boolean      allUsers,
                                       final boolean      remove,
                                       final StringBuffer newVFolderInfoXML) {
	final Boolean [] foundRoot        = new Boolean[1];
        final Boolean [] updated          = new Boolean[1];
        final Boolean [] foundAppAllUsers = new Boolean[1];

	foundRoot[0]        = Boolean.FALSE;
	updated[0]          = Boolean.FALSE;
	foundAppAllUsers[0] = Boolean.FALSE;

	updateVFolderInfoFileInternal(e,
                                      folderName,
                                      directoryPath,
                                      desktopPath,
                                      remove,
                                      foundRoot,
                                      updated,
                                      foundAppAllUsers,
                                      newVFolderInfoXML);

	if (!(updated[0].booleanValue()) && !remove) {
	    // if no applications-all-users folder found, add new one
	    // if foundRoot is true, applications-all-user folder
	    // is found

            boolean addAppAllUserFolder = (((allUsers == false) &&
                                            (foundRoot[0].booleanValue() == false)) ?
                                                true : false);

	    addNewFolderInfo(folderName,
                             directoryPath,
                             desktopPath,
                             addAppAllUserFolder,
                             true,
                             true,
                             newVFolderInfoXML);
	    if (foundRoot[0].booleanValue()) {
		newVFolderInfoXML.append("</Folder>\n");
	    }
	}
	newVFolderInfoXML.append("</" + e.getName() + ">\n");
	if (!(foundAppAllUsers[0].booleanValue())) {
	    newVFolderInfoXML.append("</" + e.getName() + ">\n");
	}


    }

    // this method parse thourgh the applications.vfolder-info file, starting
    // with the root node <VFolderInfo> that get passed in
    // It performs the following actions:
    //  1.  Make sure MergeDir exists; otherwise add it (needed for RH linux)
    //  2.  Looking for the applications root folder entry, and use it
    //        to call searchForFolder, which will do the add/remove of
    //        applications folder/shortcuts
    //
    private void updateVFolderInfoFileInternal(XMLNode               e,
                                               final String          folderName,
                                               final String          directoryPath,
                                               final String          desktopPath,
                                               final boolean         remove,
                                               final Boolean      [] foundRoot,
                                               final Boolean      [] updated,
                                               final Boolean      [] foundAppAllUsers,
                                               final StringBuffer    newVFolderInfoXML) {

	newVFolderInfoXML.append("<" + e.getName() + ">\n");
	Trace.println("updateVFolderInfoFile: XMLNode: " + e, TraceLevel.BASIC);


	try {
	XMLUtils.visitChildrenElements(e, new XMLUtils.ElementVisitor() {

		private boolean ignoreFolder = false;

		public void visitElement(XMLNode e) {

		    if (!(e.getName().equals("Folder"))) {

			if (e.getNested() == null) {
			   newVFolderInfoXML.append("<" + e.getName() + "/>\n");
			} else {

			   newVFolderInfoXML.append("<" + e.getName() + ">" + e.getNested().toString() + "</" + e.getName() + ">\n");
			}

			// Solaris GNOME 2.0
			if (e.getName().equals("Parent") && e.getNested().toString().equals("applications-all-users:///")) {

			    ignoreFolder = true;
			    foundRoot[0] = Boolean.TRUE;
			    foundAppAllUsers[0] = Boolean.TRUE;
			}
			// RH Linux 8.0 GNOME
			if (e.getName().equals("Desktop") && e.getNested().toString().equals("Applications.directory")) {

			    ignoreFolder = true;
			    foundRoot[0] = Boolean.TRUE;
			    foundAppAllUsers[0] = Boolean.TRUE;
			}
		    } else {
			newVFolderInfoXML.append("\n");

			if (foundRoot[0].booleanValue()) {

			    searchForFolder(e, folderName, directoryPath, desktopPath, remove, updated, newVFolderInfoXML);
			    foundRoot[0] = Boolean.FALSE;


			    newVFolderInfoXML.append("</" + e.getName() + ">\n");

			} else {

			    if (!ignoreFolder) {
				updateVFolderInfoFileInternal(e,
                                                              folderName,
                                                              directoryPath,
                                                              desktopPath,
                                                              remove,
                                                              foundRoot,
                                                              updated,
                                                              foundAppAllUsers,
                                                              newVFolderInfoXML);
			    }
			}

		    }

		}
	    });
	} catch (BadFieldException bfe) {
	    Trace.ignoredException(bfe);
	} catch (MissingFieldException mfe) {
	    Trace.ignoredException(mfe);
	}



    }

}
