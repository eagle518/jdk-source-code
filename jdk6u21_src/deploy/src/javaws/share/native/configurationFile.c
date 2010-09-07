/*
 * @(#)configurationFile.c	1.67 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "propertyParser.h"
#include "configurationFile.h"
#include "util.h"
#include "msgString.h"
#include "launcher.h"
#include "versionId.h"

static PropertyFileEntry* UserCfgFileHead = NULL;

/* ------------------------------------------------------------------------ */
/* Configuration file management */

/*
 * Store all user regular properties, unregistered JREs, and (registered &&
 * confirmed) JREs in the user properties file
 */
void StoreConfigurationFile(int verbose) {
  char propFullFileName[MAXPATHLEN];
  
  /* Build string to user-specific propFileName (e.g., "deployment.properties"
     or "deployment.properties.jdpa") */
  sysStrNPrintF(propFullFileName, sizeof(propFullFileName), "%s%c%s",
          sysGetDeploymentUserHome(),
          FILE_SEPARATOR,
          CFG_FILENAME);

  storePropertyFile(propFullFileName, UserCfgFileHead);
  if (verbose) {
    printf("StoreConfigurationFile: %s\n", propFullFileName);
    PrintPropertyEntry(UserCfgFileHead);
  }
}  

static int nIndices;
static JREIndexMap indexArray[255];
static char * nonRegisteredSystemJRE = NULL;

int LoadConfigurationFile(int verbose) {

  nIndices = 0;
  memset(indexArray, 0, sizeof(indexArray));
  LoadCfgFile(CFG_FILENAME, &UserCfgFileHead, verbose);
  return (UserCfgFileHead != 0);
}


/* CL: Code factored out of original version of LoadConfigurationFile(),
 * so this code can be used by other types of configuration loading
 * functions if desired.
 */
void LoadCfgFile(char* propFileName, PropertyFileEntry** UserPropertyFileEntry, int verbose) {
  char propFullFileName[MAXPATHLEN];
 
  /* Build string to user-specific property file name
     (e.g., "deployment.properties" or "deployment.properties.jdpa") */
  sysStrNPrintF(propFullFileName, sizeof(propFullFileName), "%s%c%s",
          sysGetDeploymentUserHome(),
          FILE_SEPARATOR,
          propFileName);

  /* Load and parse the users configuration property file, adding to system */
  *UserPropertyFileEntry = parsePropertyFile(propFullFileName, NULL); 
  if (verbose) {
    printf("LoadCfgFile: %s\n", propFullFileName);
    PrintPropertyEntry(*UserPropertyFileEntry);
  }
}

static int ensureDeployDownloaded() {
    struct stat statBuf;
    int result = stat(GetDeployJarPath(), &statBuf) == 0;
    if (!result) {
        // deploy.jar does not exist, we need to download it
        char java[MAXPATHLEN];
        char *appHome = sysGetApplicationHome();  
        char *argv[5];
        int len = strlen(appHome);
        strcpy(java, appHome);
        java[len] = java[len - 4]; // copy the path separator preceding "bin"
        java[len + 1] = 0;
        strcat(java, "javaw");
        argv[0] = "javaw";
        argv[1] = "sun.jkernel.DownloadManager";
        argv[2] = "-download";
        argv[3] = "deploy";
        argv[4] = NULL;
        sysExec(SYS_EXEC_WAIT, java, argv);
        result = stat(GetDeployJarPath(), &statBuf) == 0;
    }

    return result;
}

void UpdateJREInfo (JREDescription *jre) {
  char *argv[10];
  int buflen = 1024;
  char *buf = NULL, *start, *end;
  static char path[MAXPATHLEN];

  if (!ensureDeployDownloaded())
    exit(1);

  buf = (char *)malloc(buflen);
  argv[0] = jre->path;
  argv[1] = "-Dkernel.background.download=false";
  argv[2] = "-classpath";
  argv[3] = sysQuoteString(GetDeployJarPath());
  argv[4] = JRELOCATORCLASSNAME;
  argv[5] = NULL;

  sysExec2Buf(jre->path, 5, argv, buf, &buflen);

  /* find version info */
  if (buflen > 0) { 

    start = strstr(buf, PRODUCT_ID);
    if (start != NULL) {
      start += strlen(PRODUCT_ID);
      end = strchr(start, '\n');
      if (end > start) {
        char *p = jre->product_version = (char *)malloc(end - start + 1);
        while(!isWhitespace(*start)) {
            *p++ = *start++;
        }
        *p = 0;
      }
    }

    start = strstr(buf, PLATFORM_ID);
    if (start != NULL) {
      start += strlen(PLATFORM_ID);
      end = strchr(start, '\n');
      if (end > start) {
        char *p = jre->platform_version = (char *)malloc(end - start + 1);
        while(!isWhitespace(*start)) {
            *p++ = *start++;
        }
        *p = 0;
      }
    }

    /* 1.2* and before is no longer allowed */
    if  ((jre->product_version != NULL) && 
        (strncmp(jre->product_version, "1.2", 3) <= 0)) {
        jre->product_version = NULL;
    }

    if ((jre->platform_version != NULL) && 
        (strncmp(jre->platform_version, "1.2", 3) <= 0)) {
        jre->platform_version = NULL;
    }
  }

  jre->href = strdup(DEFHREF);
  jre->osname = PLATFORM;
  jre->osarch = sysGetOsArch();
}

/* some foward refs: */
int laterVersion(int index1, int index2, int verbose);
int getUniqueIndexArrayIndex();

/*
 * Get a list of all the JREs from the registry.  For each JRE in this
 * list, 1. add it to the config list if it's not already there, and
 * 2. mark it as "confirmed" in the config list.  Then, remove all
 * unconfirmed registered JREs from the config list.
 */
int RescanJREs(int verbose) {
  JREDescription reg_list[255];
  int nJREs = 0;
  int i, j;
  char *path;
  char *osname;
  char *osarch;
  int return_value = FALSE;
  int haveRegistry = 1;

  memset(reg_list, 0, sizeof(JREDescription) * 255);
  sysGetRegistryJREs(reg_list, &nJREs);
 
  /**
   * On Unix platforms, reg_list[i].product_version == NULL,
   * because sysGetRegistryJREs Unix implementation fakes a registry entry
   */
  if(nJREs==0) {
      haveRegistry = 0;
  } else if(nJREs==1) {
      haveRegistry = reg_list[0].product_version != NULL;
  }
 
  for (i = 0; i < nJREs; i++) {
    /* check if the JRE is already in the list */
    int match = 0;
    /* compare path in Unicode format */
    char *reg_path = sysMBCSToSeqUnicode(reg_list[i].path);
    if(!haveRegistry) {
        // earmark the system JRE in case we have no registry
        nonRegisteredSystemJRE = strdup(reg_path);
    }

    for (j = 0; j < nIndices; j++) {
      int index = indexArray[j].index;
      path = GetJREJavaCmd(index);
      osname = GetJREOsName(index);
      osarch = GetJREOsArch(index);    
      if (path != NULL && sysStrCaseCmp(path, reg_path) == 0) {
        if(verbose) {
            printf("test %d: path:<%s> regPath<%s> match \n", index, path, reg_path);
        }
        /* make sure osname and osarch matches too, if they are specified */
        if (osname == NULL || (osname != NULL && sysStrCaseCmp(osname, reg_list[i].osname) == 0)) {
          if(verbose) {
              printf("test %d: osname:<%s> regOsname<%s> match \n", index, 
                (osname==NULL)?"NULL":osname, (reg_list[i].osname==NULL)?"NULL":reg_list[i].osname);
          }
          /* only compare osarch if osname is SunOS */
          if (osarch == NULL || (osname != NULL && strcmp(osname, "SunOS") != 0) || (osarch != NULL && sysStrCaseCmp(osarch, reg_list[i].osarch) == 0)) {
            if(verbose) {
                printf("test %d: osarch:<%s> regOsarch<%s> match \n", index, osarch, 
                    (reg_list[i].osarch==NULL)?"NULL":reg_list[i].osarch);
            }
            /**
             * it matches.  mark it confirmed and registered 
             * now match only if version is matching. 
	     *
	     * since product version may contain milestones, e.g. 1.6.0_11-beta
	     * compare only the version string without milestone.
             */
            if ( !haveRegistry ||
                 ( GetJREProductVersion(index) != NULL && 
                   reg_list[i].product_version != NULL &&
                   sysStrNCaseCmp(GetJREProductVersion(index), 
				  reg_list[i].product_version,
				  strlen(reg_list[i].product_version)) == 0) ) 
            {
                if(verbose) {
                    printf("test %d: productV:<%s> regProductV<%s> match \n", index, 
                        (GetJREProductVersion(index)==NULL)?"NULL":GetJREProductVersion(index), 
                        (reg_list[i].product_version==NULL)?"NULL":reg_list[i].product_version);
                }
                match = 1;
                indexArray[j].confirmed = 1;
                if (!isJRERegistered(index) && haveRegistry) {
                    SetJRERegistered(index, "true");
                    return_value = TRUE;
                }
            } else if(verbose) {
                printf("test %d: productV:<%s> regProductV<%s> doesn't match \n", index, 
                        (GetJREProductVersion(index)==NULL)?"NULL":GetJREProductVersion(index), 
                        (reg_list[i].product_version==NULL)?"NULL":reg_list[i].product_version);
            }
          } else if(verbose) {
            printf("test %d: osarch:<%s> regOsarch<%s> doesn't match \n", index, 
                    (osarch==NULL)?"NULL":osarch,
                    (reg_list[i].osarch==NULL)?"NULL":reg_list[i].osarch);
          }
        } else if(verbose) {
          printf("test %d: osname:<%s> regOsname<%s> doesn't match \n", index, 
                    (osname==NULL)?"NULL":osname,
                    (reg_list[i].osname==NULL)?"NULL":reg_list[i].osname);
        }
        /* keep going here - may be more with same path */
      } else if(verbose) {
          printf("test %d: path:<%s> regPath<%s> doesn't match\n", index, 
                    (path==NULL)?"NULL":path,
                    (reg_path==NULL)?"NULL":reg_path);
      }
    }

    /* if there was no match we need to add the JRE to the list */
    if (match == 0) {
      int new = getUniqueIndexArrayIndex();
      UpdateJREInfo(&reg_list[i]);

      /* fix for 4813021 */
      /* check to make sure all the info exist before adding */
      if (!(reg_list[i].platform_version == NULL ||
          reg_list[i].product_version == NULL ||
          reg_list[i].href == NULL ||
          reg_list[i].osname == NULL ||
          reg_list[i].osarch == NULL)) {

        indexArray[nIndices].index = new;
        indexArray[nIndices++].confirmed = 1;
        SetJREPlatformVersion(new, reg_list[i].platform_version);
        SetJREProductVersion(new, reg_list[i].product_version);
        SetJRELocation(new, reg_list[i].href);
        SetJREJavaCmd(new, reg_list[i].path);
        SetJREOsName(new, reg_list[i].osname);
        SetJREOsArch(new, reg_list[i].osarch);
        SetJREEnabled(new, "true");
        if ( haveRegistry ) {
            SetJRERegistered(new, "true");
        } else {
            SetJRERegistered(new, "false");
        }
        return_value = TRUE;
        if(verbose) {
            printf("test add new %d: regPath<%s>, registered: %d\n", new, 
                (reg_list[i].path==NULL)?"NULL":reg_list[i].path,
                isJRERegistered(new));
        }
      }
    }
  }
  /* now see if any unconfirmed registered jres exist */
  for (j=0; j<nIndices; j++) {
    int index = indexArray[j].index;
    if (isJRERegistered(index) && (indexArray[j].confirmed == 0)) {
        if(!haveRegistry) {
            SetJRERegistered(index, "false");
        }
        return_value = TRUE;
    }
  }

  return return_value;
}

int getUniqueIndexArrayIndex() {
    int i, j;
    int max = -1;
    for (i=0; i<nIndices; i++) {
       if (indexArray[i].index > max) max = indexArray[i].index;
    }
    /* check for a hole */
    for (j=0; j<max; j++) {
        for (i=0; i<nIndices; i++) { 
            if (indexArray[i].index == j) break;
        }
        if (i == nIndices) return j; /* nobody using j */
    }
    /* no holes */
    return max + 1;
}

void addToIndexArray(int newIndex) {
    int j;
    for (j = 0; j < nIndices; j++) {
        if (indexArray[j].index == newIndex) return; /* not new */
    }
    indexArray[j].index = newIndex;
    nIndices++;
}

void SetJREKey(int i, char *part, char *value) {
  char key[MAX_KEY_SIZE];
  sysStrNPrintF(key, sizeof(key), "%s%d%s", CFG_JRE_KEY, i, part);
  SetProperty(key, value);
}

void SetProperty (char *key, char *value) {
    UserCfgFileHead = AddProperty(UserCfgFileHead, key, value);
}

void SetJREPlatformVersion(int i, char *value) {
    SetJREKey(i, PLATFORM_VERSION, value);
}

void SetJREProductVersion(int i, char *value) {
    SetJREKey(i, PRODUCT_VERSION, value);
}

void SetJRELocation(int i, char *value) {
    SetJREKey(i, PRODUCT_LOCATION, value);
}

void SetJREJavaCmd(int i, char *value) {
    SetJREKey(i, INSTALL_PATH, value);
}

void SetJREOsName(int i, char *value) {
    SetJREKey(i, OS_NAME, value);
}

void SetJREOsArch(int i, char *value) {
    SetJREKey(i, OS_ARCH, value);
}

void SetJREEnabled(int i, char *value) {
    SetJREKey(i, ISENABLED, value);
}

void SetJRERegistered(int i, char *value) {
    SetJREKey(i, ISREGISTERED, value);
}

char* GetJREKey(int i, char* part) {
    char key[MAX_KEY_SIZE];
    char* value;
    sysStrNPrintF(key, sizeof(key), "%s%d%s", CFG_JRE_KEY, i, part);
    value = GetPropertyValue(UserCfgFileHead, key);
    return value;
}

char* GetJREPlatformVersion(int i) {
    return GetJREKey(i, PLATFORM_VERSION);
}

char* GetJREProductVersion(int i) {
    return GetJREKey(i, PRODUCT_VERSION);
}

char* GetJRELocation(int i) {
    return GetJREKey(i, PRODUCT_LOCATION);
}

char* GetJREJavaDebugCmd(int i) {
  return sysGetDebugJavaCmd(GetJREKey(i, INSTALL_PATH));
}

char* GetJREJavaCmd(int i) {
  return GetJREKey(i, INSTALL_PATH);
}

char* GetJREOsName(int i) {
    return GetJREKey(i, OS_NAME);
}

char* GetJREOsArch(int i) {
    return GetJREKey(i, OS_ARCH);
}

int isJREEnabled(int i) {
    char *value = GetJREKey(i, ISENABLED);
    if (value != NULL && sysStrCaseCmp(value,"false")==0) {
      return 0;
    }
    return 1;
}

int isJRERegistered(int i) {
    char *value = GetJREKey(i, ISREGISTERED);
    if (value != NULL && sysStrCaseCmp(value,"true")==0) {
      return 1;
    }
    return 0;
}

int isJREConfirmed(int index) {
  int i;
  for (i = 0; i < nIndices; i++) {
    if (indexArray[i].index == index) {
      return indexArray[i].confirmed;
    }
  }  
  return FALSE;
}


/*
 * Returns the index of the best matching JRE entry in the
 * configuration file. This might be the default JRE if no
 * match is found
 *
 * We will first try to match on the first entry in the version
 * string, e.g., a version string of the form "1.3 1.2", we
 * will try to match on "1.3". If that fails, we will match on
 * the full string.
 *
 */
int DetermineVersion(char* version, char* location, int verbose) {
   char* firstMatch, *p;
   int index;
   
   firstMatch = strdup(version);
   p = strchr(firstMatch, ' ');
   while (p != NULL) {
      *p = '\0';
      index = matchVersionString(firstMatch, location, verbose);
      if (index != -1) return index;
      /* OK - look at the next one if there is one */
      firstMatch = p+1;
      if (*firstMatch != 0) {
         p = strchr(firstMatch, ' ');
      } else {
         p = NULL;
      }
   }
   
   index = matchVersionString(firstMatch, location, verbose);
   if (index == -1) {
      /* nothing matches, return latest one, including disabled */
      int best = -1;
      int pos;
      for (pos = 0; pos < nIndices; pos++) {
         index = indexArray[pos].index;
         if ((GetJREOsName(index) == NULL && GetJREOsArch(index) == NULL) ||
             (GetJREOsName(index) != NULL && GetJREOsArch(index) != NULL &&
              strcmp(sysGetOsName(), GetJREOsName(index)) == 0 &&
              strcmp(sysGetOsArch(), GetJREOsArch(index)) == 0)) {
           /* fix for 4813021 */
           /* only make the comparison if the JRE platform and product info
              exist */
           if (GetJREPlatformVersion(index) != NULL && GetJREProductVersion(index) != NULL) {
             best = laterVersion(best, index, verbose);
           }
         }
      }   
      if(verbose) {
          printf("DetermineVersion best match: %d: %s\n", best, GetJREJavaCmd(best));
      }
      return best;
   } else if(verbose) {
      printf("DetermineVersion matchVersionString OK: %d: %s\n", index, GetJREJavaCmd(index));
   }
   return index;
}

int matchVersionString(char* version, char* location, int verbose) {
    int index = 0;
    int pos = 0;
    int bestMatch = -1;

    /* Then iterate */
    for (pos = 0; pos < nIndices; pos++) {
        index = indexArray[pos].index;
        /* fix for 4813021 */
        /* only make the comparison if the JRE platform and product info
           exist */
        if (GetJREPlatformVersion(index) != NULL && 
            GetJREProductVersion(index) != NULL &&
            JreMatch(version, location, index, verbose)) {
                bestMatch = laterVersion(bestMatch, index, verbose);
        }
    }
    return bestMatch;
}

int laterVersion(int index1, int index2, int verbose) {
    char * jreCmd;
    int comp;
    if (index1 < 0) {
        if(verbose) {
            printf("laterVersion platver nil>%s(*) - %s\n", GetJREPlatformVersion(index2), GetJREJavaCmd(index2));
        }
        return index2;
    }
    comp = strcmp(GetJREPlatformVersion(index1), GetJREPlatformVersion(index2));
    if (comp > 0) {
        if(verbose) {
            printf("laterVersion platver %s(*)>%s - %s\n", GetJREPlatformVersion(index1), GetJREPlatformVersion(index2), 
                GetJREJavaCmd(index1));
        }
        return index1;
    }
    if (comp < 0) {
        if(verbose) {
            printf("laterVersion platver %s<%s(*) - %s\n", GetJREPlatformVersion(index1), GetJREPlatformVersion(index2), 
                GetJREJavaCmd(index2));
        }
        return index2;
    }
    /* same platform version - try product version */
    comp = strcmp(GetJREProductVersion(index1), GetJREProductVersion(index2)); 
    if (comp > 0) {
        if(verbose) {
            printf("laterVersion prodver %s(*)>%s - %s\n", GetJREProductVersion(index1), GetJREProductVersion(index2), 
                GetJREJavaCmd(index1));
        }
        return index1; 
    }
    jreCmd = GetJREJavaCmd(index2);
    if (comp==0) {
        // if productversion ties and we have no registry (Unix),
        // try harder and compare with the running system JRE
        if(jreCmd!=NULL && nonRegisteredSystemJRE!=NULL && sysStrCaseCmp(jreCmd, nonRegisteredSystemJRE) == 0) {
            if(verbose) {
                printf("laterVersion prodver %s==%s(*) - %s (nonRegisteredSystemJRE)\n", 
                        GetJREProductVersion(index1), GetJREProductVersion(index2), jreCmd);
            }
            return index2;
        }
        if(verbose) {
            printf("laterVersion prodver %s(*)==%s - %s\n", GetJREProductVersion(index1), GetJREProductVersion(index2), 
                GetJREJavaCmd(index1));
        }
        return index1; // prodVers ties, and index2 doesn't match nonRegisteredSystemJRE
    } else if(verbose) {
        printf("laterVersion prodver %s<%s(*) - %s)\n", GetJREProductVersion(index1), GetJREProductVersion(index2), 
                jreCmd);
    }
    return index2; 
}

int isCurrentVersion(char *javaCmd, int verbose) {
    char *current = sysGetInstallJRE();
    /* we'd really like to see if two strings refer to the same file */
    int res = (current != NULL) && (strcmp(current, javaCmd) == 0);
    if(verbose) {
        printf("isCurrentVersion: %s - %s\n", 
            ((res==TRUE)?"TRUE":"FALSE"), javaCmd);
    }
    return res;
}

/*
 * Check if a particular entry matches
 *
 */
int JreMatch(char* version, char* location, int index, int verbose) {
    char* jrePlatformVersion;
    char* jreProductVersion;
    char* jreLocation;
    char* jreJavaCmd;
    struct stat statBuf;

    if (!isJREEnabled(index)) return FALSE;

    /* Get entries from configuration file */
    jrePlatformVersion = GetJREPlatformVersion(index);

    /* Empty entry? */
    if (jrePlatformVersion == NULL) return FALSE;
    
    jreProductVersion = GetJREProductVersion(index);

    jreLocation = GetJRELocation(index);
    jreJavaCmd = GetJREJavaCmd(index);
    
    /* Entry match? */

    /* if platform is 1.2 or less, it no longer matches anything */
    if (strncmp(jreProductVersion, "1.2", 3) <= 0) return FALSE;

    /* osname and osarch must match */
    /* if osname and osarch doesn't exist, we assume it is for the */
    /* current platform */
    if ((GetJREOsName(index) == NULL && GetJREOsArch(index) == NULL) || 
        (GetJREOsName(index) != NULL && GetJREOsArch(index) != NULL &&
        strcmp(sysGetOsName(), GetJREOsName(index)) == 0 &&
        strcmp(sysGetOsArch(), GetJREOsArch(index)) == 0)) {

      /* make sure the executable actually exists */
      if (stat(jreJavaCmd, &statBuf) == 0) {
        
        if (location == NULL) {
          /* Platform match 
           * Make sure that it is not a non-fcs version. A non-FCS
           * will have a - in the platform version for 1.3 and later.
           */        
          int matchVersion = MatchVersionString(version, jrePlatformVersion);
          if (matchVersion &&
              ( jreProductVersion == NULL ||
                strchr(jreProductVersion, '-') == NULL ||
                /* now -rev and -er are also post beta */
                (strstr(jreProductVersion, "-rev") != NULL) ||
                (strstr(jreProductVersion, "-er") != NULL) ||
                /* now -beta ok if it's the one this javaws comes with */
                isCurrentVersion(jreJavaCmd, verbose))) {
            if(verbose) {
                printf("JreMatch (1) mv %d, prodver %s ~ %s - %s\n", 
                    matchVersion, version, jreProductVersion, jreJavaCmd);
            }
            return TRUE;
          }
        } else {
          int matchVersion = MatchVersionString(version, jrePlatformVersion);
          if (jreLocation != NULL &&
              strcmp(location, jreLocation) == 0 &&
              matchVersion) {
            if(verbose) {
                printf("JreMatch (2) mv %d, prodver %s ~ %s - %s\n", 
                    matchVersion, version, jreProductVersion, jreJavaCmd);
            }
            return TRUE;
          }
        }
      }
    }
    return FALSE;
}

/*
 * Check if we should show the splash screen
 */
int isSplashScreenEnabled(void) {
  char* value = GetPropertyValue(UserCfgFileHead, CFG_SPLASH_MODE);
  return (value == NULL || strcmp(value, "false") != 0);
}

/*
 *  get preferred JRE versions for Application Manager and apps that don't say
 */
char *getDefaultJREs(void) {
  /* default: latest from 1.5 and above */
  return "1.5+";
}

char *getSplashFile(char *url) {
    char *splashCacheFile;
    PropertyFileEntry *splashCacheFileHead;

    splashCacheFile = GetPropertyValue(UserCfgFileHead, CFG_SPLASH_CACHE);

    if (splashCacheFile != NULL) {
        splashCacheFileHead = parsePropertyFile(splashCacheFile, NULL);
        if (splashCacheFileHead != NULL) {
            char *ret;
            char *escaped_url = NULL;
            if (url != NULL) {
                int len = strlen(url);
                escaped_url = calloc(2,len);
                if (escaped_url != NULL) {
                    int i, j;
                    for (i=0, j=0; i<len; i++) {
                        if (url[i] == ':' || url[i] == '=') {
                            escaped_url[j++] = '\\';
                        }
                        escaped_url[j++] = url[i];
                    }
                }
            }
            ret = GetPropertyValue(splashCacheFileHead, escaped_url);
            if (escaped_url != NULL) {
                free (escaped_url);
            }
            return ret;
        }
    }
    return NULL;
}

char *getConfigSecureProperties() {
    return GetPropertyValue(UserCfgFileHead, CFG_SECURE_PROPS);
} 
 

