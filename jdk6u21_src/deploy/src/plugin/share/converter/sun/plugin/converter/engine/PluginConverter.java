/*
 * @(#)PluginConverter.java	1.40 02/04/08
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import sun.plugin.converter.util.*;
import sun.plugin.converter.ResourceHandler;
import java.io.*;
import java.util.*;
import java.text.DateFormat;
import javax.swing.*;


public class PluginConverter implements Converter, Runnable {
    private String templateFilePath = null;
    private static String defaultTemplateFileName = "templates/default.tpl";
    private File logFile = null;
    private boolean createLog = false;
    private boolean processSubDirs = false;
    private boolean stopConversion = false;
    private static ConverterProgressEvent event = null;

    private int majorVersion = 0;
    private int minorVersion = 0;
    private int microVersion = 0;
    private int updateVersion = 0;
    private int updateCabVersion = 0;
    private int updateStringLen = 0;
    private int buildNumber = 0;

    private char updateAlphaVersion = 'a';

    private String updateString = null;

    private String version = null;
    private String nodotVersion = null;
    private String cabVersion = null;
    private String urlString = null;

    private String cabFileLocation = null;
    private String nsFileLocation = null;
    private String smartUpdateLocation = null;
    private String mimeType = null;
    private String classId = null;

    private String defaultCabFileLocation = null;
    private String defaultNsFileLocation = null;
    private String defaultSmartUpdateLocation = null;
    private String defaultMimeType = null;
    private String defaultClassId = null;

    private String genericClassId = "clsid:8AD9C840-044E-11D1-B3E9-00805F499D93";

    private String userPropsFileName = "converter.props";
    private String defaultLogFileName = "convertlog.txt";

    private String caption = null;

    private boolean commandLineInvocation = false;
    private boolean staticVersioning = false;
    private boolean outdated = false;

    private static final String propFileHeader = "#  Java Plug-in HTML Converter Properties";

    private ConvertSet convertSet = null;
    //  Properties that will be changable in a bean box or on a command line
    private boolean showProgressStdOut = true;
    private PrintManyPlaces printer = null;
    
    /**
     * boolean that keeps track of the forceOverwrite
     */
    private boolean forceOverwrite = false;

    private OutputStream log = null;
    private PrintStream progress = null;
    private boolean debug = false;
    private Vector listeners = new Vector();

    private Thread runConvert = null;

    private boolean stdin = false;
    private boolean stdout = false;

    private Properties p = null;
    private String versionState = null;

    /*
     *Constructor used in GUI version
     */
    public PluginConverter() throws FileNotFoundException {
	this(false, false);
    }

    /**
       Constructor
    */
    public PluginConverter(boolean logfile, boolean showProgress) throws FileNotFoundException {
        setVersion(System.getProperty("java.version"));
	setBuildNumber(System.getProperty("java.runtime.version"));
        setValues();
	setCreateLog(logfile);
	setShowProgressStdOut(showProgress);

	try {
	    setTemplateFilePath(getDefaultTemplateFilePath());
	    if(createLog) {
		setLogFile(getDefaultLogFile());
	    }
	}
	catch(FileNotFoundException e) {
	    e.printStackTrace();
	    throw e;
        }

        caption = ResourceHandler.getMessage("caption.warning");

        //
        // Set staticVersioning to the default.
        //

	setStaticVersioning(staticVersioning);
    }

    /*
     * Set the version for the converter based on the targeted release.
     *
     * @param The version string to target.
     */
    private void setVersion(String rawVersion) {

        //
        // The version string is everything upto the first dash, if present.
        //

        int dashIndex = rawVersion.indexOf("-");
        if ( dashIndex < 0 ) {
            version = rawVersion;
        }
        else {
            version = rawVersion.substring(0,dashIndex);
        }

        //
        // Based on the version, set the release variables.
        //

        StringTokenizer st = new StringTokenizer(version,"._");
        if ( st.hasMoreTokens() ) {
            majorVersion = new Integer(st.nextToken()).intValue();
        }
        if ( st.hasMoreTokens() ) {
            minorVersion = new Integer(st.nextToken()).intValue();
        }
        if ( st.hasMoreTokens() ) {
            microVersion = new Integer(st.nextToken()).intValue();
        }
        if ( st.hasMoreTokens() ) {
            updateString = st.nextToken();
	    updateStringLen = updateString.length();
            if (updateStringLen <= 2) {
                updateVersion =
                    new Integer(updateString).intValue();
                updateCabVersion = updateVersion * 10;
            } else {
                updateVersion = new Integer(updateString.substring(0,2)).intValue();
                updateAlphaVersion = updateString.charAt(updateStringLen-1);
                int updateAlphaVersionValue =
                    Character.getNumericValue(updateAlphaVersion) -
                    Character.getNumericValue('a') + 1;
                updateCabVersion = updateVersion * 10 + updateAlphaVersionValue;
            }
        }
    }

    /*
     * Set the build number for the converter based on the targeted release.
     *
     * @param The version string to target.
     */
    private void setBuildNumber(String rawVersion) {
        //
        // The build number appears after the last 'b'
        //
        String buildNumberStr = null;
        int idx = rawVersion.lastIndexOf("b");
	if ( idx != -1 && rawVersion.length() > (idx+1) ) {
            buildNumberStr = rawVersion.substring(idx);
        } else {
            buildNumberStr = "b00";
	}
        // extract the build number
	try {
	    buildNumber = Integer.parseInt(buildNumberStr.substring(1));
	} catch ( NumberFormatException e ) {
            buildNumber = 0;
	}
    }

    /*
     * Generate the various default string values based on version.
     */
    private void setValues() {

        //
        // Set the nodot and cab version strings.
        //

        nodotVersion = majorVersion + "_" + minorVersion;
        cabVersion = minorVersion + "," + microVersion + ",";

        if ( isStaticVersioning() ) {
            nodotVersion += "_" + microVersion;
	    cabVersion += updateCabVersion + "," + buildNumber;
            if ( updateVersion > 0 ) {
                nodotVersion += "_" + updateString;
            }
	    versionState = "static";
        }
        else {
            cabVersion += "0," + buildNumber;
	    versionState = "dynamic";
        }


        //
        // Set the file locations.
        //

        if ( majorVersion > 1 || minorVersion > 4 || (minorVersion >= 4 && microVersion >= 2)) {
           urlString = "http://java.sun.com/update/" + majorVersion + "." + minorVersion + "." + microVersion;
	}
	else {
           urlString = "http://java.sun.com/products/plugin/autodl";
	}
        
        // install cab file name convention: 
        //     jinstall-1_x_x_xx-win.cab            for releases prior 1.4.1
        //     jinstall-1_x_x_xx-windows-i586.cab  for releases 1.4.1 to 1.5
	//     jinstall-6ux-windows-i586.cab      for releases 1.6 and after
	if ( majorVersion > 1 || minorVersion >=6 ){
             defaultCabFileLocation = urlString + "/jinstall-" + 
	       minorVersion;
	     if (updateVersion > 0){
	       defaultCabFileLocation = defaultCabFileLocation +
		 "u" + updateVersion;
	     }

	     defaultCabFileLocation = defaultCabFileLocation + 
	       "-windows-i586.cab#Version=" + cabVersion;
	} else if ( majorVersion > 1 || minorVersion > 4 || (minorVersion >= 4 && microVersion >= 1)) {
             defaultCabFileLocation = urlString + "/jinstall-" + nodotVersion + 
            "-windows-i586.cab#Version=" + cabVersion;
        } 
        else {
             defaultCabFileLocation = urlString + "/jinstall-" + nodotVersion + 
            "-win.cab#Version=" + cabVersion;
        }
        
        defaultNsFileLocation =  "http://java.sun.com/products/plugin/index.html#download";
       
        //  jre file name convention: 
        //     jre-1_x_x_xx-win.jar            for releases prior 1.4.1
        //     jre-1_x_x_xx-windowns-i586.jar  for releases 1.4.1 and after
        if ( majorVersion > 1 || minorVersion > 4 || (minorVersion >= 4 && microVersion >= 1)) {
            defaultSmartUpdateLocation = urlString + "/jre-" + nodotVersion + "-windows-i586.jar";
        }
        else {
            defaultSmartUpdateLocation = urlString + "/jre-" + nodotVersion + "-win.jar";
        }

	// Create the default mime type depending on static or dynamic versioning.
        if ( isStaticVersioning() ) {
	    defaultMimeType = "application/x-java-applet;jpi-version=" + version;
        } else {
	    defaultMimeType = "application/x-java-applet;version=" + version;
	}


        //
        // Set the default classid
        //

        defaultClassId = "clsid:CAFEEFAC-";

        for ( int i=0; i<4-(""+majorVersion+minorVersion).length(); i++ ) {
            defaultClassId += "0";
        }
        defaultClassId += "" + majorVersion + minorVersion;
        defaultClassId += "-";

        for ( int i=0; i<4-(""+microVersion).length(); i++ ) {
            defaultClassId += "0";
        }
        defaultClassId += microVersion;
        defaultClassId += "-";

        for ( int i=0; i<4-(""+updateVersion).length(); i++ ) {
            defaultClassId += "0";
        }
        defaultClassId += updateVersion;

	if (updateStringLen <=2) {
            defaultClassId += "-ABCDEFFEDCBA";
        } else {
            if (Character.isLowerCase(updateAlphaVersion)) {
                updateAlphaVersion = Character.toUpperCase(updateAlphaVersion);
            }
            updateAlphaVersion++;
            String clsidSubStr = String.valueOf(updateAlphaVersion);
            defaultClassId += "-" + clsidSubStr + "BCDEFFEDCBA";
        }

        cabFileLocation = defaultCabFileLocation;
        nsFileLocation = defaultNsFileLocation;
        smartUpdateLocation = defaultSmartUpdateLocation;
        mimeType = defaultMimeType;
	//
	// If the version is 1.3.0, use the generic classid as the dynamic classid
	// was not implemented until 1.3.0_01.
	//

	if ( version.equals("1.3.0") ) {
	    defaultClassId = genericClassId;
	}

        classId = defaultClassId;
	

        //
        // Read in the user properties file and override defaults.
        //

        if (p == null) {

	    File f = new File(userPropsFileName);
	    if ( f.exists() ) {
		try {
		    FileInputStream fis = new FileInputStream(f);              
		    p = new Properties();
		    p.load(fis);
		    fis.close();
		    versionState = (String) p.getProperty("last.converter.state", versionState);
			if(versionState.compareTo("static") == 0)
			    staticVersioning = true;
			else
			    staticVersioning = false;
			cabFileLocation = p.getProperty(versionState + ".converter.cab.file.loc", cabFileLocation);
			nsFileLocation = p.getProperty(versionState + ".converter.plugin.file.loc", nsFileLocation);
			smartUpdateLocation = p.getProperty(versionState + ".converter.smartupdate.file.loc", smartUpdateLocation);
			mimeType = p.getProperty(versionState + ".converter.template.mimetype", mimeType);
			classId = p.getProperty(versionState + ".converter.classid", classId);
		}
		catch(Exception e) {
		}
	    } else {
		p = new Properties();
	    }
	}
	
    }


    public void persistConverterSetting() {

	if(!outdated)
	    return;

	p.put("last.converter.state",versionState);
	if(nsFileLocation!=null)
	    p.put(versionState + ".converter.plugin.file.loc",p.getProperty(versionState + ".converter.plugin.file.loc", nsFileLocation));
	if(cabFileLocation!=null)
	    p.put(versionState + ".converter.cab.file.loc",p.getProperty(versionState + ".converter.cab.file.loc", cabFileLocation));
	if(smartUpdateLocation!=null)
	    p.put(versionState + ".converter.smartupdate.file.loc",p.getProperty(versionState + ".converter.smartupdate.file.loc", smartUpdateLocation));
	if(mimeType!=null)
	    p.put(versionState + ".converter.template.mimetype",p.getProperty(versionState + ".converter.template.mimetype", mimeType));
	if(classId!=null)
	    p.put(versionState + ".converter.classid",p.getProperty(versionState + ".converter.classid", classId));


	File f = null;
	try {
	    f = new File(userPropsFileName);
			    
	    if(f.exists()) {
		f.delete();
	    }
	    FileOutputStream fos = new FileOutputStream(f);
	    p.store(fos, propFileHeader);
	    fos.flush();
	    fos.close();
	}
	catch(IOException e) {
	    System.out.println(ResourceHandler.getMessage("plugin_converter.saveerror"));
	}
    }
	
    public void setConvertSet(ConvertSet c) {
        convertSet = c;
    }
    public ConvertSet getConvertSet() {
        return convertSet;
    }

    /*
     *Do we show progress in Stdout
     */
    public void setShowProgressStdOut(boolean b) { 
	if(showProgressStdOut != b) {
	    showProgressStdOut = b;
	}
    }
    public boolean isShowProgressStdOut() {
        return showProgressStdOut;
    }

    public void setCommandLine(boolean commandLineInvocation) {
	this.commandLineInvocation = commandLineInvocation;
    }
    public boolean getCommandLine() {
	return commandLineInvocation;
    }

    /*
     * Progress can be posted out via the ConverterProgressEvent object
     */
    public void addConverterProgressListener(ConverterProgressListener l) {
	listeners.addElement(l);
    }
    public void removedConverterProgressListener(ConverterProgressListener l) {
	listeners.removeElement(l);
    }
    public void fireEvent(ConverterProgressEvent e) {
	Vector list = (Vector)listeners.clone();
	for(int i = 0;i<list.size();i++) {
	    ConverterProgressListener listener = (ConverterProgressListener)list.elementAt(i);
	    listener.converterProgressUpdate(e);
	}
    }

    public void startConversion() {
	if(debug) System.out.println("Debug is on");
	if(runConvert==null) {
	    runConvert = new Thread(this);

	    runConvert.start();
	}
    }

    public void stopConversion() {
	if((runConvert != null) && runConvert.isAlive()) {
			
	    stopConversion = true;
	    if(progress!=null) {
		progress.println(ResourceHandler.getMessage("plugin_converter.cancel"));
	    }
	}
	runConvert = null;
    }

    public void run() {
	try {
	    setPrintOutput();
	}
	catch(IOException e) {
	    System.out.println(ResourceHandler.getMessage("plugin_converter.logerror"));
	}

	Date now = new Date();

	if (progress!=null) {
            progress.println(ResourceHandler.getMessage("plugin_converter.appletconv") 
                             +(DateFormat.getDateInstance(DateFormat.LONG)).format(now)+" "+
                             (DateFormat.getTimeInstance(DateFormat.LONG)).format(now));
        }

	//  Starting to convert a convertSet (a directory - could be a sub directory)
	event = new ConverterProgressEvent(this);

	stopConversion = false;
	try {
                runConversion(convertSet);
	}
	catch (IOException e) {
	    e.printStackTrace();
	}

	event.setStatus(ConverterProgressEvent.ALL_DONE);
	fireEvent(event);
	if (progress!=null) {
	    progress.println(ResourceHandler.getMessage("plugin_converter.done") + event.getFilesProcessed()
			     + ResourceHandler.getMessage("plugin_converter.appletfound") 
			     + event.getAppletsFound());
        }
    }


    private void runConversion (ConvertSet convertSet) 
        throws IOException {
       
        File sourceFile = null;
        File destFile = null;
        String fileName = null;
        InputStream is = null;

	//  Get the Source Files in the ConvertSet passed in
	Enumeration files = convertSet.getFilesEnumeration();
	while(files.hasMoreElements() && !stopConversion) {
	    //  Get the current name to operation on, this could be a filename or directory
	    if (!stdout) {
               destFile = new File(convertSet.getDestinationPath(), "temp.conv");
	    }
	    fileName = (String)files.nextElement();
            sourceFile = new File(convertSet.getSourcePath(), fileName);
	    //  If this is a directory, and we are supposed to process sub dirs, then we do so.
              if(sourceFile.isDirectory()) {
		if(processSubDirs) {
		    ConvertSet subSet = new DefaultSet();
		    boolean filesOnly = ((FlexFilter)convertSet.getFilenameFilter()).isFilesOnly();
		    ((FlexFilter)((DefaultSet)subSet).getFilenameFilter()).setFilesOnly(filesOnly);
		    try {
			subSet.setSourcePath(sourceFile,convertSet.getFilenameFilter());
		    }
		    catch(NotDirectoryException e) {
			e.printStackTrace();
		    }

		    subSet.setDestinationPath(new File(convertSet.getDestinationPath(),fileName));
		    subSet.setBackupPath(new File(convertSet.getBackupPath(),fileName));
                    runConversion(subSet);
		}

		//If sub-directory, skip processing
		continue;
	      }
	      //If the file doesn't exist or doesn't have write permissions, simply return
	      if(sourceFile.exists()) {
		if(!sourceFile.canWrite()) {
		    if(!commandLineInvocation) JOptionPane.showMessageDialog(null,ResourceHandler.getMessage("file_unwritable.info") + sourceFile, caption, JOptionPane.ERROR_MESSAGE );
		    else System.out.println(ResourceHandler.getMessage("file_unwritable.info") + sourceFile);
		        return;						      
		}
	      }
	      else {
	        if(!commandLineInvocation) JOptionPane.showMessageDialog(null, ResourceHandler.getMessage("file_notexists.info") + sourceFile, caption, JOptionPane.ERROR_MESSAGE );
	        else System.out.println(ResourceHandler.getMessage("file_notexists.info") + sourceFile);
		    return;
	      }
            
            is = getClass().getClassLoader().getResourceAsStream(getTemplateFilePath());
	    if (is == null) {
		File f = new File(getTemplateFilePath());
		is = new FileInputStream(f);
	    } 



            //  Get a ConverFile object with a template
	    ConvertFile conFiler = new ConvertFile(is);
	    
            //we need to set the sourcefile so we can guessencoding
	        conFiler.setSource(sourceFile);
	    
            //we need to guess the encoding so we can us it for countwords
	    conFiler.guessEncoding();
            if (StdUtils.countWords(sourceFile,"<APPLET", true, conFiler.getEncoding())>0) {
        	if(progress!=null) {
		    progress.print(new File(convertSet.getSourcePath(),fileName)
				   + ResourceHandler.getMessage("plugin_converter.processing"));
                }

		//  Send out progress information
       		if (!stdin)
                event.setSourcePath(convertSet.getSourcePath().getAbsolutePath());
		if (!stdout)
                 event.setDestinationPath(convertSet.getDestinationPath().getAbsolutePath());
		event.setBackupPath(convertSet.getBackupPath().getAbsolutePath());
		if(!stdin)
                    event.setCurrentFile(fileName);
		event.setStatus(ConverterProgressEvent.CONVERTING);
		fireEvent(event);  //  Fire converting event to preceed processing
		conFiler.setSource(sourceFile);
                if (!stdout) 
	           conFiler.setDestination(destFile);
                else 
                   conFiler.setStandardOutput(stdout); 
                conFiler.setCabFileLocation(getCabFileLocation());
		conFiler.setNSFileLocation(getNSFileLocation());
		conFiler.setSmartUpdateLocation(getSmartUpdateLocation());
		conFiler.setMimeType(getMimeType());
		conFiler.setClassId(getClassId());

		try {
		    boolean gotApplets = conFiler.convert();  //  Convert file
		    if (!gotApplets) {
			if (progress!=null) {
			    progress.print(ResourceHandler.getMessage("plugin_converter.converted"));
                        }
			destFile.delete();
		    }
		    else { 
			//Create the backup directory
			if(!StdUtils.createDirs(convertSet.getBackupPath())) {
			    if(progress!=null) {
				progress.println(ResourceHandler.getMessage("plugin_converter.nobackuppath"));
                            }
			    event.setStatus(ConverterProgressEvent.DEST_DIR_NOT_CREATED);
			    fireEvent(event);

			    if(!commandLineInvocation) JOptionPane.showMessageDialog(null, ResourceHandler.getMessage("file_unwritable.info") + convertSet.getBackupPath().getAbsolutePath(), caption, JOptionPane.ERROR_MESSAGE );    
			    else System.out.println(ResourceHandler.getMessage("file_unwritable.info") + convertSet.getBackupPath().getAbsolutePath());
			    return;
			}

			System.gc();
			System.runFinalization();

			//
			// Save a backup copy, and update the original with the converted html.
			//

                    if(!stdin) {
			File backupFile = new File(convertSet.getBackupPath().getAbsolutePath(),fileName);
			File originalFile = new File(destFile.getParent() + File.separator + fileName);

			if ( !updateFiles(backupFile,originalFile,destFile) ) {
  		  	    if(!commandLineInvocation)JOptionPane.showMessageDialog(null,
							  ResourceHandler.getMessage("plugin_converter.failure") +
							  destFile.getParent() + File.separator + fileName, 
							  caption, JOptionPane.ERROR_MESSAGE);
			    else System.out.println(ResourceHandler.getMessage("plugin_converter.failure") +
							  destFile.getParent() + File.separator + fileName);
			}
                      }
		    }

		    event.setStatus(ConverterProgressEvent.DONE);  //  Done converting file

		    //Update summary information
		    event.setFilesProcessed(event.getFilesProcessed()+1);
		    event.setAppletsFound(event.getAppletsFound()+conFiler.getAppletsFound());
		    event.setErrorsFound(event.getErrorsFound()+conFiler.getErrors());
		    fireEvent(event);

		    if (progress!=null) {
			progress.println(ResourceHandler.getMessage("plugin_converter.donefound") 
					 + conFiler.getAppletsFound());
                    }
		}
		catch (Exception e) {
		    event.setStatus(ConverterProgressEvent.ERROR);  //  Done converting file

		    //  Update summary information
		    event.setFilesProcessed(event.getFilesProcessed());
		    event.setAppletsFound(event.getAppletsFound()+conFiler.getAppletsFound());
		    event.setErrorsFound(event.getErrorsFound()+1);
		    fireEvent(event);

		    if(progress!=null) {
			progress.println(ResourceHandler.getMessage("plugin_converter.seetrace"));
			e.printStackTrace(progress);
		    } 
		}							
            }
	    else {
		if (progress!=null) {
		    progress.println(ResourceHandler.getMessage("plugin_converter.noapplet") +sourceFile);
                }
	    } 
           if (stdin && sourceFile.exists()) {
               sourceFile.delete();   //removing the temperately created source file
           }  
        }
    }

    /**
     * Update the backup and original files.
     *
     * @param backupFile The backup file object.
     * @param originalFile The original file object.
     * @param convertedFile The converted file object.
     * @return True if the files were successfully updated, false otherwise.
     */
    public boolean updateFiles(File backupFile, File originalFile, File convertedFile) {

	//
	// Make sure we do not overwrite the backup copy without permission.
	// First we move the original source to the backup copy.  Then
	// we rename the converted file to the same name as the original.
	//

	return ( backupDirExists(backupFile) &&
		 backupFileIsWritable(backupFile,originalFile) && 
                 renameFileTo(originalFile, backupFile) &&
                 renameFileTo(convertedFile, originalFile));
    }

    /**
     * Returns true if the backup directory exists or creates it if it doesnt.
     *
     * @param backupFile The backup file object.
     * @return True if the back up directory exists
     */
  
    public boolean backupDirExists(File backupFile){
      File tempDir = new File(backupFile.getParent());
	
      // If the destination directory of the backupFile exists return true
      // else create it.
	if(tempDir.exists()) return true;
	else {
	    try{
	      tempDir.mkdir();
	    }catch(SecurityException e){ e.printStackTrace(); }
	    return true;
	}
    }
  
  /**
   * Renames origionalFile to newFile. newFile will be overwritten if it
   * exists!
   *
   * @param origionalFile The file.
   * @param newFile Where the file is going (including path and filename);
   * @return True is it was successfully copied.
   */
  
  public boolean renameFileTo(File origionalFile, File newFile){
    try{
      copy(origionalFile.getAbsolutePath(), newFile.getAbsolutePath());
    }catch(IOException e){ e.printStackTrace(); }
    try{
      origionalFile.delete();
    }catch(SecurityException e){ e.printStackTrace(); }
    return true;
  }
  
  /**
   * Copies one file to another.
   *
   * @param from Where the file is.
   * @param to Destination.
   */
  public void copy(String source, String destination) throws IOException{
    BufferedInputStream bis = null;
    BufferedOutputStream bos = null;
    int bytesRead = -1;
    final int bufsize = (int) new File(source).length();
    final byte [] buffer = new byte[bufsize];
    
    try {
      bis = new BufferedInputStream(new FileInputStream(source));
      bos = new BufferedOutputStream(new FileOutputStream(destination));
      
      bis.read(buffer,0,bufsize);
      bos.write(buffer,0,bufsize);
    }catch (IOException e) {
      e.printStackTrace();
    }
    finally {
      bis.close();
      bos.close();
    }
  }

    /**
     * Sets the force overwrite backup files value.
     *
     * @param forceOverwrite True to overwrite backup files.
     */
    public void setOverwrite(boolean forceOverwrite){
        this.forceOverwrite = forceOverwrite;
    }

    /**
     * Return true if the backup file is available for writing.
     *
     * @param backupFile The backupFile object.
     * @return True if the backup file is available for writing.
     */
    public boolean backupFileIsWritable(File backupFile, File originalFile) {
	if ( backupFile.exists() ) {
	    if(commandLineInvocation && !forceOverwrite) System.out.println(ResourceHandler.getMessage("plugin_converter.overwrite1") + originalFile.getAbsolutePath());
	    else if (commandLineInvocation && forceOverwrite) return true;
	    //
	    // A backup copy already exists.  Let the user make a choice to overwrite.
	    //
	    	    
	    else if ( JOptionPane.showConfirmDialog( null, 
						ResourceHandler.getMessage("plugin_converter.overwrite1") +
						originalFile.getAbsolutePath() +
						ResourceHandler.getMessage("plugin_converter.overwrite2"), 
						caption,
						JOptionPane.YES_NO_OPTION ) == JOptionPane.YES_OPTION) {
		
		//
		// The user said it's ok to overwrite.
		//
		
		backupFile.delete();
	    }
	    else {
		return false;
	    }
	}
	return true;
    }

  
    public static String getDefaultTemplateFileName() {
	return defaultTemplateFileName;
    }
    
    
    /*
     * Name of template file to be used
     */
    public void setTemplateFilePath(String fileName) throws FileNotFoundException {
	try {
    	    // First, try to load it from JAR file directly
	    InputStream is = getClass().getClassLoader().getResourceAsStream(fileName);
	    
	    if (is != null) {
		templateFilePath = fileName;
		return;
	    }

	    // Second, try to load it locally
	    File f = new File(fileName);

	    if (f.isFile()) {
		templateFilePath = fileName;
		return;
	    }
	}
	catch (Throwable e) {
	    e.printStackTrace();
	}

	throw new FileNotFoundException(fileName);
    }

    public String getTemplateFilePath() {
        return templateFilePath;
    }

    public String getDefaultTemplateFilePath() throws FileNotFoundException {
	// Look for the template file from classloader
	ClassLoader loader = getClass().getClassLoader();

	if (loader == null) {
	    throw new FileNotFoundException(defaultTemplateFileName);
        }

	InputStream is = loader.getResourceAsStream(defaultTemplateFileName);

	if (is == null) {
	    throw new FileNotFoundException(defaultTemplateFileName);
        }
	else {
	    return defaultTemplateFileName;
        }
    }

    /*
     * Stream to direct log information
     */
    public void setLogFile(java.io.File f) throws IllegalArgumentException {
	if(f.exists()) {
	    if(progress!=null) 
                progress.println(ResourceHandler.getMessage("plugin_converter.writelog"));
	}
	else {
	    if(f.getParent() == null) {
		f = new File(System.getProperty("user.dir"),f.getName());
	    }
	    else {
		File dirs = new File(f.getParent());
		if(!dirs.exists())
                    dirs.mkdirs();
	    }
	}		
	
	logFile = f;
    }

    /**
     * Get the static versioning flag.  If true, only the current version of 
     * Java will be used to run applets.  If false, any version of Java that
     * can handle the current mimetype can be used.
     *
     * @return True if only the current Java version should be used to run applet.
     */
    public boolean isStaticVersioning() {
        return staticVersioning;
    }

    /**
     * Set whether static versioning is to be used.
     *
     * @param staticVersioning True if only a specific version of Java should be used to run the applet, otherwise the latest available JRE in the family will be used.
     */
    public void setStaticVersioning(boolean staticVersioning) {
        this.staticVersioning = staticVersioning;
        if ( staticVersioning ) {
            setVersion(System.getProperty("java.version"));
            setValues();
	    String mimeType = getMimeType();
	    if ( mimeType.endsWith(".0") ) {
		setMimeType(mimeType.substring(0,mimeType.length()-2));
	    }
            setClassId(defaultClassId);
        }
        else {
            setVersion(majorVersion + "." + minorVersion);
            setValues();
            int jpiIndex = getMimeType().indexOf("jpi-");
            if ( jpiIndex >= 0 ) {
                setMimeType(getMimeType().substring(0,jpiIndex) +
                            getMimeType().substring(jpiIndex+4));
            }
            setClassId(genericClassId);
        }
    }

    public java.io.File getLogFile() {
        return logFile;
    }

    public java.io.File getDefaultLogFile() {
	return new File(System.getProperty("user.dir"),defaultLogFileName);
    }
	
    public void setPrintOutput() throws IOException {
	if(printer!=null) printer.closeAll();
	printer = new PrintManyPlaces();
	if(showProgressStdOut || debug) {
	    printer.addPlace(System.out);
	}
		
	if(createLog) {	    	  
	    printer.addPlace(new FileOutputStream(getLogFile(), true));
	}
			
	if(printer.countPlaces()>0) {
	    progress = new PrintStream(printer);
	}
	else {
	    progress = null;
	}
    }

    /*
      Do we create a log?
    */
    public void setCreateLog(boolean b) { 
	if(b != createLog) {
	    createLog = b; 
	}
    }

    public boolean isCreateLog() {
        return createLog;
    }

    /*
     * Do we process into subdirectories
     */
    public void setRecurse(boolean b) { 
	((FlexFilter)convertSet.getFilenameFilter()).setFilesOnly(!b);
	processSubDirs = b;
    }
    public boolean isRecurse() {
        return processSubDirs;
    }

    public String getCabFileLocation() {
	return  p.getProperty(versionState + ".converter.cab.file.loc", cabFileLocation);
    }
    public String getDefaultCabFileLocation() {
	return defaultCabFileLocation;
    }
    public void setCabFileLocation(String location) {
	String temp = p.getProperty(versionState + ".converter.cab.file.loc");
	if (temp == null) {
	    p.put(versionState + ".converter.cab.file.loc",location);
	    if (!location.equals(defaultCabFileLocation))
		outdated=true;
	} else {
	    if (!location.equals(temp)) {
		p.put(versionState + ".converter.cab.file.loc",location);
		outdated=true;
	    }
	}
    }

    public String getNSFileLocation() {
	return  p.getProperty(versionState + ".converter.plugin.file.loc", nsFileLocation);
    }
    public String getDefaultNSFileLocation() {
	return defaultNsFileLocation;
    }
    public void setNSFileLocation(String location) {
	String temp = p.getProperty(versionState + ".converter.plugin.file.loc", nsFileLocation);
	if (temp == null) {
	    p.put(versionState + ".converter.plugin.file.loc",location);
	    if (!location.equals(defaultNsFileLocation))
		outdated=true;
	} else {
	    if (!location.equals(temp)) {
		p.put(versionState + ".converter.plugin.file.loc",location);
		outdated=true;
	    }
	}
    }

    public String getSmartUpdateLocation() {
	return p.getProperty(versionState + ".converter.smartupdate.file.loc", smartUpdateLocation);
    }
    public String getDefaultSmartUpdateLocation() {
	return defaultSmartUpdateLocation;
    }
    public void setSmartUpdateLocation(String location) {
	String temp = p.getProperty(versionState + ".converter.smartupdate.file.loc");
	if (temp == null) {
	    p.put(versionState + ".converter.smartupdate.file.loc",location);
	    if (!location.equals(defaultSmartUpdateLocation))
		outdated=true;
	} else {
	    if (!location.equals(temp)) {
		p.put(versionState + ".converter.smartupdate.file.loc",location);
		outdated=true;
	    }
	}
    }

    public String getMimeType() {
	return  p.getProperty(versionState + ".converter.template.mimetype", mimeType);
    }
    public void setMimeType(String type) {
	String temp = p.getProperty(versionState + ".converter.template.mimetype");
	if (temp == null) {
	    p.put(versionState + ".converter.template.mimetype",type);
	    if (!type.equals(mimeType))
		outdated=true;
	} else {
	    if (!type.equals(temp)) {
		p.put(versionState + ".converter.template.mimetype",type);
		outdated=true;
	    }
	}
    }

    public String getDefaultMimeType() {
	return defaultMimeType;
    }
    public void setDefaultMimeType(String defaultMimeType) {
	this.defaultMimeType = defaultMimeType;
    }

    public String getClassId() {
	return  p.getProperty(versionState + ".converter.classid", classId);
    }
    public void setClassId(String classId) {
        this.classId = classId;
	String temp = p.getProperty(versionState + ".converter.classid");
	if (temp == null) {
	    p.put(versionState + ".converter.classid",classId);
	} else {
	    if (!classId.equals(temp)) {
		p.put(versionState + ".converter.classid",classId);
		outdated=true;
	    }
	}
    }

    public void setSourceType (boolean isStdin) {
        stdin = isStdin;
    }

    public void setDestType (boolean isStdout) {
        stdout = isStdout;
    }

    public String toString() {
	String str = "";

	str += "\nShow Progress StdOut:  "+showProgressStdOut;
	str += "\ndebug:  "+debug;
	str += "\nTemplate File Path:  "+templateFilePath.toString();
	str += "\nCount Listeners:  "+listeners.size();
	str += "\nLog File Path:  "+logFile.toString();
	str += "\nCreate Log:  "+createLog;
	str += "\nConvert Set:  "+convertSet.toString();
	str += "\nProcess Sub Dirs:  "+processSubDirs;

	return str;
    }
}
