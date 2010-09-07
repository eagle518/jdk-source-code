/*
 * @(#)launchFile.c	1.42 10/04/02
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "util.h"
#include "launchFile.h"
#include "propertyParser.h"
#include "xmlparser.h"
#include <ctype.h>

/* Local methods */
static void ParseXMLLaunchFile(char* s, JNLFile* jnlfile, int verbose);
static int containsUnsupportedCharacter(char *s);
static char* escapeBackSlashCharacter(char *s);

/*
 * Parse a JNL file, and returns a structure with the information
 * needed by the C launcher 
 *
 * The interpretation of the JNL file is completly encapsulated into
 * this method, so several formats such as XML and property files can
 * be supported at the same time 
 */
JNLFile* ParseJNLFile(char *s, int verbose) {

    /* Initialze jnlfile structure */
    JNLFile* jnlfile = (JNLFile*) malloc(sizeof(JNLFile));
    jnlfile->jreVersion = NULL;
    jnlfile->jreLocation = NULL;
    jnlfile->isPlayer    = TRUE;
    jnlfile->splashPref = SPLASH_ALWAYS;  // default - show splash
    jnlfile->jnlp_url = NULL;
    jnlfile->canonicalHome = NULL;
    jnlfile->initialHeap =NULL;
    jnlfile->maxHeap = NULL;
    jnlfile->auxArgCount = 0;
    jnlfile->auxPropCount = 0;

    if (s != NULL) {
        ParseXMLLaunchFile(s, jnlfile, verbose);
    }
    
    return jnlfile;
}

static void addAuxArg(JNLFile *jnlfile, char *AuxArgString, int verbose) {
    if (jnlfile->auxArgCount < 20) {
        char * auxQ = sysQuoteString(AuxArgString);
        jnlfile->auxArg[jnlfile->auxArgCount] = auxQ;
        jnlfile->auxArgCount++;
        if(verbose) {
            printf("adding secure arg: <%s>\n", auxQ);
        }
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

int isSecureProperty(char *key) {
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
            i = 0;
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

static int validResources(XMLNode *resources) {
    char *os, *arch;

    if (resources == NULL) {
        return 0;
    }
    os = FindXMLAttribute(resources->_attributes, "os");
    if (os != NULL && (strcmp(os, sysGetOsName()) != 0)) {
        return 0;
    }
    arch = FindXMLAttribute(resources->_attributes, "arch");
    if (arch != NULL && (strcmp(arch, sysGetOsArch()) != 0)) {
        return 0;
    }
    return 1;
}

/* 
 * Disallow ", % and any character not in [ -~] 
 * returns 1 if string contains unsupported character
 * returns 0 otherwise
 */
static int containsUnsupportedCharacter(char *s) {
    int i = 0;
    int len = strlen(s);
    int charValue = 0;
 
    for (i = 0; i < len; i++) {
        charValue = (int)s[i];
        if ( charValue < ' ' || charValue > '~' || charValue == '"' || 
                charValue == '%') {
            return 1;
        }
    }
    return 0;
}

static char* escapeBackSlashCharacter(char *s) {
    int length0;
    int length1;
    int i;
    int j = 0;
    char *ss;
    
    if (s == NULL) {
        return NULL;
    }
 
    length0 = strlen(s);
    /* for null termination */
    length1 = length0 + 1;
    
    /* count the number of embedded backslash chars */
    for(i = 0; i < length0; i++) {
        if (s[i] == '\\') {
            length1 += 1;
        }
    }
       
    /* copy s, replacing each '\\' with '\\\\' */
    ss = (char *)calloc(length1, sizeof(char));
    
    if (ss == NULL) {
        return NULL;
    }
 
    for(i = 0; i < length0; i++) {
        if (s[i] == '\\') {
            ss[j++] = '\\';
        }
        ss[j++] = s[i];
    }
 
    ss[j] = '\0';
    
    return ss;
    
}

/* Parse the XML Launch file 
 *
 * Looks for the following path: <jnlp><resources><j2se version=attr>
 */
static void ParseXMLLaunchFile(char* s, JNLFile* jnlfile, int verbose) {   
    XMLNode* doc = NULL;
    XMLNode* node = NULL;
    XMLNode* resources = NULL;
    XMLNode* tempnode = NULL;
    XMLNode* jarnode = NULL;
    XMLNode* jnlpnode = NULL;
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
        jnlpnode = node;
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
         
            while (tempnode != NULL) {
                /* process valid resources element */
                if (validResources(tempnode)) {
                    
                    jarnode = FindXMLChild(tempnode->_sub, "jar");
                    
                    /* go thru all jars and find main jar */
                    while (jarnode && mainjar == NULL) {
                        str = FindXMLAttribute(jarnode->_attributes, "href");
                        
                        if (firstjar == NULL) {
                            firstjar = malloc(strlen(str) + 1);
                            strcpy(firstjar, str);
                        }
                        str2 = FindXMLAttribute(jarnode->_attributes, "main");
                        if (str2 != NULL && sysStrCaseCmp(str2, "true") == 0) {
                            mainjar = malloc(strlen(str) + 1);
                            strcpy(mainjar, str);
                        }
                        jarnode =jarnode->_next;
                    }
                    
                    /* if no main jar, return first jar */
                    if ((mainjar == NULL) && (firstjar != NULL)) {
                        mainjar = strdup(firstjar);
                    }
                    
                    if (codebase != NULL && mainjar != NULL) {
                        /* generate canonicalHome */
                        char *last;
                        char *jnlpString = "jnlp";
                        jnlfile->canonicalHome = malloc(strlen(codebase) +
                                strlen(mainjar) +
                                strlen(jnlpString) +
                                2);
                        strcpy(jnlfile->canonicalHome, codebase);
                        last = codebase + (strlen(codebase) - 1);
                        if (*last != '/') {
                            strcat(jnlfile->canonicalHome, "/");
                        }
                        strcat(jnlfile->canonicalHome, mainjar);
                        strcat(jnlfile->canonicalHome, jnlpString);
                    }
                }
                tempnode = FindXMLChild(tempnode->_next, "resources");
            }
        }

        if (node != NULL) {
            resources = FindXMLChild(node->_sub, "resources");
            while (resources != NULL) { 
                if (validResources(resources)) {
                    node = FindXMLChild(resources->_sub, "java");
                    if (node == NULL) {
                        node = FindXMLChild(resources->_sub, "j2se");
                    }
                    if (node != NULL) {
                        str = FindXMLAttribute(node->_attributes, "version");
                        if (str != NULL) {
                            jnlfile->jreVersion = strdup(str);
                        }
                        str = FindXMLAttribute(node->_attributes, "href");
                        if (str != NULL) {
                            jnlfile->jreLocation = strdup(str);
                        }   
                        str = FindXMLAttribute(node->_attributes, "max-heap-size");
                        if (str != NULL) {
                            jnlfile->maxHeap = strdup(str);
                        }
                        str = FindXMLAttribute(node->_attributes, "initial-heap-size");
                        if (str != NULL) {
                            jnlfile->initialHeap = strdup(str);
                        }
                        str = FindXMLAttribute(node->_attributes, "java-vm-args");
                        if (str != NULL) {
                            int i,j,len;
                            len = strlen(str);
                            if (len > 0) {
                                char* arg = 
                                    (char*)malloc((sizeof(char) * len) + 1);
                                if (arg != NULL) {
                                    for (i = 0; i < len; i++) {
                                        if (iswspace(str[i])) {
                                            str[i] = ' ';
                                        }
                                    }
                                    for (i=0, j=0; i<len; i++) {
                                        if (str[i] != ' ') {
                                            arg[j++] = str[i];
                                        } 
                                        if ((str[i] == ' ') || (i == (len-1))) {
                                            if (j > 0) {
                                                arg[j] = '\0';
                                                if (isSecureVmArg(arg)) {
                                                    addAuxArg(jnlfile, arg, verbose);
                                                }
                                                j = 0;
                                            }
                                        }
                                    }
                                    free(arg);
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
                                    if (containsUnsupportedCharacter(value)) {
                                        /*
                                         * ignored value with unsupported 
                                         * character in it
                                         */
                                    } else {
                                        char * auxArg = NULL;
                                        if(sysGetQuotesWholePropertySpec()) {
                                            int len = 16 + strlen(name) + strlen(value);
                                            char *_auxArg = malloc(len);
                                            int n = sysStrNPrintF(_auxArg, len, "-D%s=%s",name, value);
                                            if(0<=n && n<len) {
                                                auxArg = sysQuoteString(_auxArg);
                                            }
                                            free(_auxArg);
                                        } else {
                                            char *valueQ = sysQuoteString(value);
                                            int len = 16 + strlen(name) + strlen(valueQ);
                                            int n;
                                            auxArg = malloc(len);
                                            n = sysStrNPrintF(auxArg, len, "-D%s=%s",name, valueQ);
                                            if(0>n || n>=len) {
                                                free(auxArg);
                                                auxArg = NULL;
                                            }
                                            free(valueQ);
                                        }
                                        if (auxArg!=NULL && jnlfile->auxPropCount < 20) {
                                            jnlfile->auxProp[jnlfile->auxPropCount++] = auxArg;
                                            if(verbose) {
                                                printf("add secure props: <%s>\n", auxArg); 
                                            }
                                        }
                                    } 
                                }
                            }    
                        }
                        node = FindXMLChild(node->_next, "property");
                    }
                } 
                resources = FindXMLChild(resources->_next, "resources");
            } /* while (resources != NULL) */
        } 
        if (jnlpnode != NULL) {
            tempnode = FindXMLChild(jnlpnode->_sub, "application-desc");
            if (tempnode == NULL) {
                tempnode = FindXMLChild(jnlpnode->_sub, "applet-desc");
            }
            if (tempnode == NULL) {
                tempnode = FindXMLChild(jnlpnode->_sub, "component-desc");
            }
            if (tempnode != NULL) {
                if (FindXMLAttribute(tempnode->_attributes, 
                                     "progress-class") != NULL) {
                    /* if we specify a progress-class, no default splash */
                    jnlfile->splashPref = SPLASH_CUSTOM_ONLY;
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

