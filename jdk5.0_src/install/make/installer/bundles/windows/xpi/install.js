XML_URL = "HTTP_SERVER/update/JDKMAJOR.JDKMINOR.JDKMICRO/FULL_VERSION.xml"

// global variable
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

//
// Check error
//
function checkError(errorValue)
{
  if((errorValue != SUCCESS) && (errorValue != REBOOT_NEEDED))
  {
    abortInstall(errorValue);
    return(true);
  }

  return(false);
} // end checkError()

// main
{
    var err = initInstall("Java(TM) 2 Runtime Environment (U.S. English version) Standard Edition vJDKMAJOR.JDKMINOR.JDKMICROJDKUPDATE for Netscape(TM) 7 Release ", 
			  "/Sun/Java", 
			  "JDKMAJOR.JDKMINOR.JDKMICRO.JDKUPDATE"); 
    logComment("initInstall() returned: " + err);
    
    // check return value
    if (!checkError(err))
    {
	var fPlugins = getFolder("Program");
        srcDest = 60000; // We need 60 MByte disk space at least
	if (verifyDiskSpace(fPlugins, srcDest))
    	{
            // Install Java 
	    // check return value
	    logComment("installJava execute:");
	    var err = execute("jinstall.exe", XML_URL, true);
	    logComment("installJava ret: " + err);
		    
	    if (!err)
	    {
		logComment("performInstall() returned: " + err);
		err = performInstall();
		
	    }
	}
	else
	{
	    cancelInstall(INSUFFICIENT_DISK_SPACE);
	}
    }
}

// end main
