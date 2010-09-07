JRE_DIR = "jre" + "JDK_VERSION";
// this function verifies disk space in kilobytes
function verifyDiskSpace(dirPath, spaceRequired)
{
  var spaceAvailable;

  // Get the available disk space on the given path
  spaceAvailable = fileGetDiskSpaceAvailable(dirPath);

  // Convert the available disk space into kilobytes
  spaceAvailable = parseInt(spaceAvailable / 1024);

  // do the verification
  if(spaceAvailable < spaceRequired)
  {
    logComment("Insufficient disk space: " + dirPath);
    logComment("  required : " + spaceRequired + " K");
    logComment("  available: " + spaceAvailable + " K");
    return(false);
  }

  return(true);
}

// The size of the jre binary in killobytes
var srDest = 60000;

var err = initInstall("Sun Java", "/Sun/Java", "JDK_VERSION"); 
logComment("initInstall: " + err);

var fPlugins= getFolder("Plugins");
logComment("plugins folder: " + fPlugins);

if (verifyDiskSpace(fPlugins, srDest))
{
    err = addDirectory("JRE_Plugin_Linux_i586",
    		       "JDK_VERSION",
                       JRE_DIR,   // jar source folder 
                       fPlugins,           // target folder 
                       "java",            // target subdir 
                       true );             // force flag 

    logComment("addDirectory() returned: " + err);

    // create symlink: plugins/libjavaplugin_oji.so ->
    //                 plugins/java/plugin/i386/libjavaplugin_oji.so
    var lnk = fPlugins + "libjavaplugin_oji.so";
    var tgt = fPlugins + "java/plugin/i386/ns7/libjavaplugin_oji.so";
    var ignoreErr = execute("symlink.sh", tgt + " " + lnk, true);
    logComment("execute symlink.sh "+tgt+" "+lnk+" returned: "+ignoreErr);

    if (err==SUCCESS)
    {
	    err = performInstall(); 
	    logComment("performInstall() returned: " + err);
    }
    else
    {
	    cancelInstall(err);
	    logComment("cancelInstall() returned: " + err);
    }
}
else
    cancelInstall(INSUFFICIENT_DISK_SPACE);
