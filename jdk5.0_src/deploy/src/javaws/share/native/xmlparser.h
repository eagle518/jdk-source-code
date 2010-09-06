/*
 * @(#)xmlparser.h	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef XMLPARSER_H
#define XMLPARSER_H

/*
 *  Contains a simply-minded XML parser.  
 *
 *  The following assumptions are made about the DTD, XML document:
 *   -  The encoding is UTF-8
 * 
 *   -  The parser sets all   unicode character >255 to 255. Thus,
 *      there can be no non-acsii characters in tags, attributes, or
 *      data used by the C program.
 *
 *   -  All attributes are passed as C data
 *
 *   -  No entities are defined except for the default ones, e.g.,
 *      &amp; (&), &lt; (<), &gt; (>), &apos; ('), and &quote(")
 *
 */

#define xmlTagType    0
#define xmlPCDataType 1

typedef struct _xmlNode XMLNode;
typedef struct _xmlAttribute XMLAttribute;

struct _xmlNode {
    int           _type;        /* Type of node: tag, pcdata, cdate */
    char*         _name;        /* Contents of node */
    XMLNode*      _next;        /* Next node at same level */
    XMLNode*      _sub;         /* First sub-node */
    XMLAttribute* _attributes;  /* List of attributes */  
};

struct _xmlAttribute {
    char* _name;              /* Name of attribute */
    char* _value;             /* Value of attribute */
    XMLAttribute* _next;      /* Next attribute for this tag */
}; 


/* Public interface */
void     RemoveNonAsciiUTF8FromBuffer(char *buf);
XMLNode* ParseXMLDocument    (char* buf);
void     FreeXMLDocument     (XMLNode* root);

/* Utility methods for parsing document */
XMLNode* FindXMLChild        (XMLNode* root,      char* name);
char*    FindXMLAttribute    (XMLAttribute* attr, char* name);

/* Debugging */
void PrintXMLDocument(XMLNode* node, int indt);

#endif
