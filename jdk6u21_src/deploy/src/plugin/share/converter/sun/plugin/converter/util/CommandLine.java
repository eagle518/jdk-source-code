/*
 * @(#)CommandLine.java	1.23 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import sun.plugin.converter.*;
import java.util.*;
import java.io.*;

public class CommandLine{

    /**
     * The command line to be processed
     */
    private String commandLine[];
    
    /**
     * boolean helper, do gui or not
     */
    private boolean gui = false;
    
    /**
     * boolean helper, are params present?
     */
    private boolean params = false;

    /**
     * boolean helper, was help a given parameter?
     */    
    private boolean help = false;

    /**
     * boolean helper,  was subdirs a given parameter?
     */    
    private boolean processSubDirs = false;

    /**
     * boolean helper, was simulate a given parameter?
     */    
    private boolean simulate = false;

    /**
     * boolean helper,  was -log a given parameter?
     */    
    private boolean logFile = false;

    /**
     * boolean helper, -f (overwrite option)
     */
    private boolean forceOverwrite = false;
    
    /**
     * holds the source directory
     */
    private String sourceStr;  

    /**
     * holds the destination directory
     */  
    private String destStr;  
  
    /**
     * holds the backup directory
     */
    private String backupStr;  

    /**
     * holds the template directory, plus the template file
     */
    private String templateStr; 
   
    /**
     * holds the log path specified on the commandline
     */
    private String logPath;

    /**
     * holds the progress string
     */
    private String progressStr = "false";

    /**
     * holds the file directory and file name
     */
    private Vector fileSpecs = new Vector();

    /**
     * Static versioning boolean.  True if the user wants only a
     * particular JRE to run the applet.
     */
    private boolean staticVersioning = true;

    /**
     * holds "/" or "\"
     */
    private String sep = System.getProperty("file.separator");

    private String destPathSuffix = "_CONV";
    private String backupPathSuffix = "_BAK";
    private int maxFileCharLength = 27;

    private boolean stdin = false;
    private boolean stdout = false;
    
    
    /**
     * This will take in the string given at the command line by the 
     * user find if it is null and calls readline().
     * @param s[] holds the given command line string
     */
        public CommandLine(String s[]) throws CommandLineException, IndexOutOfBoundsException {
	if (s==null || s.length == 0){ // there are no params	    
	    params = false;
	}
	else{
	    params = true;
	    commandLine = s;
            readLine();
	    //
	    // If no files are specified, default.
	    //

	       if ((isStdIn() == false) && fileSpecs.isEmpty()) {

		   String specs = ResourceHandler.getMessage("converter_gui.lablel2");
		   StringTokenizer st = new StringTokenizer(specs," ,");
		   while ( st.hasMoreTokens() ) {
		       fileSpecs.add(st.nextToken());
		   }
	       }
	       checkForProblems();
	}
    }
    
    /**
     * This will parse the given command line and set variables
     */

      public void readLine() throws CommandLineException, IndexOutOfBoundsException {
	    	    
	    String arg;
	    
	    try{
		for(int i = 0;i<commandLine.length;i++){
		    String rawArg = commandLine[i];
		    
		    arg = rawArg.trim().toUpperCase();  // Get a param

		    if(arg.startsWith("-")){
			if(isCmdLineOption(arg)){	//  File Spec	
			    if(arg.equals("-GUI")){	//  -gui	       
				i++;
				gui = true;
				i--;
			    }
			    else if(arg.equals("-HELP")){	//  -help	       
				i++;
				help = true;
				i--;
			    }
			    else if(arg.equals("-F")){         //  -f
				forceOverwrite = true;
			    }
			    else if(arg.equals("-SUBDIRS")){	//  -subdirs
				processSubDirs = true;
			    }
			    else if(arg.equals("-SOURCE")){  // -source directory
				sourceStr = commandLine[++i];
			    }
			    else if(arg.equals("-DEST")){   // -destination location
				destStr = commandLine[++i];
			    }
			    else if(arg.equals("-BACKUP")){	//  -backup location
				backupStr = commandLine[++i];
			    }
			    else if(arg.equals("-TEMPLATE")){  //  -template file
			    templateStr = commandLine[++i];
			    }
			    else if(arg.equals("-LOG")){//  -log file
				logFile = true;
				logPath = commandLine[++i];
			    }
			    else if(arg.equals("-PROGRESS")){   //  -progress
				progressStr = commandLine[++i];
			    }	
			    else if(arg.equals("-SIMULATE")){   //just a simulation?
				simulate = true;
			    }
			    else if(arg.equals("-LATEST")) {   // Use any JRE that supports this mimetype.
				staticVersioning = false;
			    }
			}
         		else {
			    throw new CommandLineException();
		 	}
		    }
		    else fileSpecs.add(rawArg);
		}	
	    } catch(IndexOutOfBoundsException e)
		{
                    help = true;	
                    throw e;	
		}
    }

    /**
     * This will make sure the destination path isn't the 
     * same as the backup directory, the application will 
     * terminate if they are the same
     */   
    private void checkForProblems() throws CommandLineException {

	if((backupStr != null) && (destStr != null)){
	    File fBackupTemp = new File(backupStr);
	    File fDestTemp = new File(destStr);
	    
	    if ( (fDestTemp.getAbsolutePath()).equals(fBackupTemp.getAbsolutePath())){
		System.out.println(ResourceHandler.getMessage("illegal_source_and_backup.info"));
		throw new CommandLineException();
	    }
	}
    }
    
    /**
     *This will return the the boolean logFile value
     * @return value of logfile
     */
    public boolean isLogFile(){
	return logFile;
    }

    /**
     *This will return the the boolean params value
     * @return value of params
     */
    public boolean areParams(){
	return params;
    }
    
    /**
     *This will return the the boolean help value
     * @return value of help
     */
    public boolean justHelp(){
	return help;
    }

    /**
     *This will return the the boolean processSubDirs value
     * @return value of processSubDirs
     */
    public boolean doSubDirs(){
	return processSubDirs;
    }
    
    /**
     *This will return the the boolean gui value
     * @return value of gui
     */
    public boolean doGUI(){
	return gui;
    }
    
    /**
     *This will return the the boolean progressStr value
     * @return value of progressStr
     */
    public boolean showProgress(){
	if(progressStr.equals("true"))
	    return true;
	return false;
    }
    
    /**
     *This will return the the boolean simulate value
     * @return value of simulate
     */
    public boolean justSimulate(){
	return simulate;
    }
    
    /**
     * Returns the value of -f, the force overwrite option.
     *
     * @return True to overwrite, false otherwise.
     */
    public boolean isOverwrite(){
      return forceOverwrite;
    }

    /**
     * This method will determine if the source is reading from standard in.
     * @retrun true, if the source is standard input; false, otherwise.
     */
    public boolean isStdIn() {
       if ((sourceStr != null) && sourceStr.equals("-")) {
          stdin = true;
          sourceStr = null;  // reset sourceStr; since "-" is not the path of source 
       }
       return stdin;
        
    } 

    /**
     * This method will determine if the destination is writing to standard output.
     * @retrun true, if the dest is standard output; false, otherwise.
     */
    public boolean isStdOut() {
       if ((destStr != null ) && destStr.equals("-")) {
          stdout = true;
          destStr = null;  // reset destStr; since "-" is not the path of destination 
       }
       return stdout;
    } 

    /**
     * This will return the absolute path of the source directory 
     * no matter what the case is.  If the user didn't enter a source
     * directory, it will return null 
     * @return the value of the sourceDir path
     */
    
    public String getSourceDir(){
	String tempSource;

 	if(sourceStr != null){
	    tempSource = doesExistAndAbsolute(sourceStr);
	    return tempSource;
	}
	else
	    return null;
    }
    
    /**
     * This will return the absolute path of the Destination directory 
     * no matter what the case is.  If the user didn't enter a dest
     * directory, it will return null 
     * @return the value of the destination path
     */
    
    public String getDestDir(){
	if(destStr != null){
	    String tempdestStr = doesExistAndAbsolute(destStr);
	    return tempdestStr;
	}
	else 
	    return null;
    }

    /**
     * This will return the absolute path of the backup directory 
     * no matter what the case is.  If the user didn't enter a backup
     * directory, it will return null 
     * @return the value of the backup directory
     */
    public String getBackupDir(){
	    return backupStr;
    }

    /**
     * This will always have to be a definate path up to the filename.
     * The doesExistAndAbsolute method will just make sure it exists
     * and that it is an absolute pathname.  If the user didn't enter
     * in a template path, it will return null
     * @return the path of the template file
     */

    public String getTemplateDir(){
	if(templateStr != null){
	    File bill = new File(templateStr);
	    String tempTemplate = doesExistAndAbsolute(templateStr);
	    return tempTemplate;
	}
	else
	    return null;
    }
    
    
    /**
     * This will return the absolute path to the log file
     * or null if no log file was specified.
     * @return the full path to the logfile
     */
    public String getLogFile() {
	String path = null;
	if ( logPath != null ) {
	    File log = new File(logPath);
	    path = doesExistAndAbsolute(log.getParent()) + 
		File.separator + log.getName();
	}
	return path;
    }

    /**
     * This will return the vector of the files to be converted
     * @return the vector of paths for the converted files
     */
    public Vector getFileSpecs(){
	return fileSpecs;
    }    

    /**
     * Get the staticVersioning boolean value.
     * @return True if any JRE supporting the mimetype can be used to run the applet.
     */
    public boolean getStaticVersioning() {
        return staticVersioning;
    }

    private boolean isCmdLineOption(String option){
	String test = option.trim().toUpperCase();
	
	return (
		test.equals("-SOURCE") ||
		test.equals("-DEST") ||
		test.equals("-BACKUP") ||
		test.equals("-F") ||
		test.equals("-SUBDIRS") ||
		test.equals("-TEMPLATE") ||
		test.equals("-LOG") ||
		test.equals("-PROGRESS") ||
		test.equals("-GUI") ||
		test.equals("-HELP") ||
		test.equals("-SIMULATE") ||
                test.equals("-LATEST"));
    }

    /**
     * Note: this is not used, but should be when we decide what length is 
     *
     */
    private String truncateString(String dir,int length) {
	//  If the name of the source dir is too long to take the suffix chars,
	//  we crop the source name before adding the suffix
	if(dir.length()>length) dir = dir.substring(0,length-1);
	return dir;
    }

    /**
     * This method will take in a string which will be a directory.
     * It will make sure that the directory exists and return the absolute path
     * @return the value of the absolute directory
     */

    private String doesExistAndAbsolute(String dir){
	
	File tempDirFile = new File(dir);
	try{
	    if(tempDirFile.exists()){
		if(tempDirFile.isAbsolute()){
		    return dir;
		}
		else{
		    tempDirFile = new File(System.getProperty("user.dir")+sep+dir);
		    if(tempDirFile.exists()){
			return tempDirFile.getAbsolutePath();
		    }
		    else{
			throw new NotDirectoryException(ResourceHandler.getMessage("caption.reldirnotfound")+":  "+tempDirFile.getAbsolutePath());
		    }
		}
	    }
	    else{
		throw new NotDirectoryException(ResourceHandler.getMessage("caption.absdirnotfound")+":  "+tempDirFile.getAbsolutePath());
	    }
	    
	}catch(NotDirectoryException e){
	   System.out.println(ResourceHandler.getMessage("caption.absdirnotfound")+":  "+tempDirFile.getAbsolutePath());
           return null;//this is just so it would compile 
        }
   }
    
    /**
     * This method will read the standard input and save the content to a temporary.
     * source file since the source file will be parsed mutiple times. The HTML source 
     * file is parsed and checked for Charset in HTML tags at first time, then source file 
     * is parsed again for the converting
     * The temporary file is read from standard input and saved in current 
     * directory and will be deleted after the conversion has completed. 
     * @return the vector of the converted temporary source file name 
     */

     public Vector getStandardInput() throws FileAccessException {
        byte[] buf = new byte[2048];
        int len = 0;
        FileInputStream fis = null;
        FileOutputStream fos = null;
        File tempSourceFile;
        String tmpSourceName = ".tmpSource_stdin";
     
        File outFile = new File(tmpSourceName);
        
        tempSourceFile = new File(System.getProperty("user.dir")+sep+tmpSourceName);
        if(!tempSourceFile.exists()){
           try {
               fis = new FileInputStream (FileDescriptor.in);
               fos =  new FileOutputStream(outFile);
               while ((len = fis.read(buf, 0, 2048)) != -1)
                  fos.write(buf, 0, len);
           }
           catch  (IOException e) {
               System.out.println(ResourceHandler.getMessage("plugin_converter.write_permission"));
               return null;
           } 
        }  
        else {
            throw new FileAccessException(ResourceHandler.getMessage("plugin_converter.overwrite"));
        }
        tempSourceFile = new File(System.getProperty("user.dir")+sep+tmpSourceName);
        if(tempSourceFile.exists()){
            fileSpecs.add(tmpSourceName);
            return fileSpecs;
        }
        else { 
            throw new FileAccessException(ResourceHandler.getMessage("plugin_converter.write_permission"));
        }
    } 
}

