/*
 * @(#)launchFile.c	1.26 04/04/03
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "util.h"
#include "launchFile.h"
#include "propertyParser.h"
#include "xmlparser.h"

/* Local methods */
static void ParseXMLLaunchFile(char* s, JNLFile* jnlfile);

/*
 * Parse a JNL file, and returns a structure with the information
 * needed by the C launcher 
 *
 * The interpretation of the JNL file is completly encapsulated into
 * this method, so several formats such as XML and property files can
 * be supported at the same time 
 */
JNLFile* ParseJNLFile(char *s) {

    /* Initialze jnlfile structure */
    JNLFile* jnlfile = (JNLFile*) malloc(sizeof(JNLFile));
    jnlfile->jreVersion = NULL;
    jnlfile->jreLocation = NULL;
    jnlfile->isPlayer    = TRUE;
    jnlfile->jnlp_url = NULL;
    jnlfile->canonicalHome = NULL;
    jnlfile->initialHeap =NULL;
    jnlfile->maxHeap = NULL;
    jnlfile->auxArgCount = 0;
    jnlfile->auxPropCount = 0;

    if (s != NULL) {
        ParseXMLLaunchFile(s, jnlfile);
    }
    
    return jnlfile;
}

static void addAuxArg(JNLFile *jnlfile, char *AuxArgString) {
    if (jnlfile->auxArgCount < 20) {
        jnlfile->auxArg[jnlfile->auxArgCount] = strdup(AuxArgString);
        jnlfile->auxArgCount++;
    }   
}

static int indexOf(char *str, char c) {
    int i;
    for (i=0; i<strlen(str); i++) {
	if (str[i] == c) return i;
    }
    return -1;
}

static char **securePropertyKeys = NULL;
static int securePropertiesCount = -1;

static int isSecureProperty(char *key) {
    int i;
    extern int isDefaultSecureProperty(char *key);

    if (isDefaultSecureProperty(key)) {
	return 1;
    }

    if (securePropertiesCount < 0) {
	char *Keys;
	securePropertiesCount = 0;
	Keys = getConfigSecureProperties();
	if (Keys != NULL && strlen(Keys) > 0) {
	    char *remain;
	    int len = strlen(Keys);
	    securePropertiesCount = 1;
	    for (i=0; i<len; i++) {
		if (Keys[i] == ',') {
		    securePropertiesCount++;
		}
	    }
	    securePropertyKeys = calloc(securePropertiesCount, sizeof(char *));
	    remain = Keys;
	    while (strlen(remain) && i < securePropertiesCount) {
		int commaIndex = indexOf(remain, ',');
		if (commaIndex < 0) {
		    securePropertyKeys[i++] = strdup(remain);
		    break;
		} else {
	            remain[commaIndex++] = '\0';
		    while (remain[commaIndex] == ' ') { commaIndex++; }
		    securePropertyKeys[i++] = strdup(remain);
                    remain += commaIndex;
		}	
	    }
	}
    }
    for (i=0; i<securePropertiesCount; i++) {
        if (strcmp(key, securePropertyKeys[i]) == 0) {
       	    return 1;
        }
    }   
    return 0;
}


/* Parse the XML Launch file 
 *
 * Looks for the following path: <jnlp><resources><j2se version=attr>
 */
static void ParseXMLLaunchFile(char* s, JNLFile* jnlfile) {   
    XMLNode* doc = NULL;
    XMLNode* node = NULL;
    XMLNode* resources = NULL;
    XMLNode* tempnode = NULL;
    char* str;
    char* codebase;
    char* href;
    char *name;
    char *value;

    /* added for getting canonical home */
    char* firstjar = NULL;
    char* mainjar = NULL;
    char* str2;  

    jnlfile->jreVersion = NULL;
    jnlfile->jreLocation = NULL;
    jnlfile->isPlayer = FALSE;
    jnlfile->canonicalHome = NULL;

    /* Parse XML document. */
    doc = ParseXMLDocument(s);
    if (doc != NULL) {
      node = FindXMLChild(doc, "jnlp");      
     
      if (node != NULL) {
        codebase = FindXMLAttribute(node->_attributes, "codebase");
        href = FindXMLAttribute(node->_attributes, "href");
	if ((codebase != NULL) && (href != NULL)) {
	  if (strstr(href, "http:") == href) { 
	    jnlfile->jnlp_url = href;
	  } else {
	    char *last;
	    jnlfile->jnlp_url = malloc(strlen(codebase) + strlen(href) + 2);
	    strcpy(jnlfile->jnlp_url, codebase);
	    last = codebase + (strlen(codebase) - 1);
	    if (*last != '/') {
	      strcat(jnlfile->jnlp_url, "/");
	    }
	    strcat(jnlfile->jnlp_url, href);
	  }
	  /* canonical home is equal to href if href exist */
	  jnlfile->canonicalHome = malloc(strlen(jnlfile->jnlp_url) + 1);
	  strcpy(jnlfile->canonicalHome, jnlfile->jnlp_url);	
	}
      }

      /* find main jar to generate canonical home if no href */
      if (node != NULL && jnlfile->canonicalHome == NULL) {
         tempnode = FindXMLChild(node->_sub, "resources");
         if (tempnode != NULL) { 
        
	    tempnode = FindXMLChild(tempnode->_sub, "jar");
	  
	    /* go thru all jars and find main jar */	   
	    while (tempnode && mainjar == NULL) {	     
	      str = FindXMLAttribute(tempnode->_attributes, "href");	     
	      if (firstjar == NULL) {
		firstjar = malloc(strlen(str) + 1);	      
		strcpy(firstjar, str);	     
	      }	    
	      str2 = FindXMLAttribute(tempnode->_attributes, "main");	     
	      if (str2 != NULL && sysStrCaseCmp(str2, "true") == 0) {
		mainjar = malloc(strlen(str) + 1);	      
		strcpy(mainjar, str);		
	      }	   
	      tempnode = tempnode->_next;	      
	    }
	    /* if no main jar, return first jar */
	    if ((mainjar == NULL) && (firstjar != NULL)) {
	      mainjar = strdup(firstjar);	     
	    }

	    if (codebase != NULL && mainjar != NULL) {
	      /* generate canonicalHome */
	      char *last;
	      jnlfile->canonicalHome = malloc(strlen(codebase) + strlen(mainjar) + 2);
	      strcpy(jnlfile->canonicalHome, codebase);
	      last = codebase + (strlen(codebase) - 1);
	      if (*last != '/') {
		strcat(jnlfile->canonicalHome, "/");
	      }
	      strcat(jnlfile->canonicalHome, mainjar);
	     
	    }
	 }
      }
     

      if (node != NULL) {
         resources = FindXMLChild(node->_sub, "resources");
         if (resources != NULL) { 
            node = FindXMLChild(resources->_sub, "j2se");
            if (node != NULL) {
		str = FindXMLAttribute(node->_attributes, "version");
		if (str != NULL) jnlfile->jreVersion = strdup(str);
		str = FindXMLAttribute(node->_attributes, "href");
		if (str != NULL) jnlfile->jreLocation = strdup(str);
		  
		str = FindXMLAttribute(node->_attributes, "max-heap-size");
		if (str != NULL) jnlfile->maxHeap = strdup(str);
		str = FindXMLAttribute(node->_attributes, "initial-heap-size");
		if (str != NULL) jnlfile->initialHeap = strdup(str);
		str = FindXMLAttribute(node->_attributes, "java-vm-args");
		if (str != NULL) {
		    char arg[MAXPATHLEN];
		    int i,j,len;
		    len = strlen(str);
		    for (i=0, j=0; i<len; i++) {
			if (str[i] != ' ') {
			    arg[j++] = str[i];
			} 
			if ((str[i] == ' ') || (i == (len-1))) {
			    if (j > 0) {
			        arg[j] = '\0';
			        if (isSecureVmArg(arg)) {
			            addAuxArg(jnlfile, arg);
			        }
			        j = 0;
		            }
			}
		    }
		}
            }
            node = FindXMLChild(resources->_sub, "property");
            while (node != NULL) {
                name = FindXMLAttribute(node->_attributes, "name");
                if (name != NULL) {
                    value = FindXMLAttribute(node->_attributes, "value");
                    if (value != NULL) {
                        if (isSecureProperty(name)) {
                            char *auxArg = malloc(16 + strlen(name) +
                                                        strlen(value));
                            sprintf(auxArg, "-D%s=%s",name, value);
                            if (jnlfile->auxPropCount < 20) {
                                jnlfile->auxProp[jnlfile->auxPropCount++] = auxArg;
                            } /* */
                        }
                    }    
                }
                node = FindXMLChild(node->_next, "property");
            }
         }
      }
    }

    /* Check for player */
    if (doc != NULL && FindXMLChild(doc, "player") != NULL) {
         jnlfile->isPlayer = TRUE;
    }

    free(firstjar);
    free(mainjar);
    FreeXMLDocument(doc);
}


/* Release all memory allocated to a JNLFile */
void FreeJNLFile(JNLFile* jnlfile) {
    int i;
    if (jnlfile != NULL) {
        free(jnlfile->jreVersion);
        free(jnlfile->jreLocation);
        for (i=0; i<jnlfile->auxArgCount; i++) {
            if (jnlfile->auxArg[i] != NULL) {
                free(jnlfile->auxArg[i]);
            }
            jnlfile->auxArg[i] = NULL;
        }
        free(jnlfile);
    }
}

