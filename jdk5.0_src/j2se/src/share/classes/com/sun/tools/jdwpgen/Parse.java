/*
 * @(#)Parse.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class Parse {

    final StreamTokenizer izer;
    final Map kindMap = new HashMap();

    Parse(Reader reader) {
        izer = new StreamTokenizer(new BufferedReader(reader));
        izer.resetSyntax();
        izer.slashStarComments(true);
        izer.slashSlashComments(true);
        izer.wordChars((int)'a', (int)'z');
        izer.wordChars((int)'A', (int)'Z');
        izer.wordChars((int)'0', (int)'9');
        izer.wordChars((int)'_', (int)'_');
        izer.wordChars((int)'-', (int)'-');
        izer.wordChars((int)'.', (int)'.');
        izer.whitespaceChars(0, 32);
        izer.quoteChar('"');
        izer.quoteChar('\'');

        kindMap.put("CommandSet", new CommandSetNode());
        kindMap.put("Command", new CommandNode());
        kindMap.put("Out", new OutNode());
        kindMap.put("Reply", new ReplyNode());
        kindMap.put("ErrorSet", new ErrorSetNode());
        kindMap.put("Error", new ErrorNode());
        kindMap.put("Event", new EventNode());
        kindMap.put("Repeat", new RepeatNode());
        kindMap.put("Group", new GroupNode());
        kindMap.put("Select", new SelectNode());
        kindMap.put("Alt", new AltNode());
        kindMap.put("ConstantSet", new ConstantSetNode());
        kindMap.put("Constant", new ConstantNode());
        kindMap.put("JVMDI", new JVMDINode());
        kindMap.put("int", new IntTypeNode());
        kindMap.put("long", new LongTypeNode());
        kindMap.put("boolean", new BooleanTypeNode());
        kindMap.put("object", new ObjectTypeNode());
        kindMap.put("threadObject", new ThreadObjectTypeNode());
        kindMap.put("threadGroupObject", new ThreadGroupObjectTypeNode());
        kindMap.put("arrayObject", new ArrayObjectTypeNode());
        kindMap.put("stringObject", new StringObjectTypeNode());
        kindMap.put("classLoaderObject", new ClassLoaderObjectTypeNode());
        kindMap.put("classObject", new ClassObjectTypeNode());
        kindMap.put("referenceType", new ReferenceTypeNode());
        kindMap.put("referenceTypeID", new ReferenceIDTypeNode());
        kindMap.put("classType", new ClassTypeNode());
        kindMap.put("interfaceType", new InterfaceTypeNode());
        kindMap.put("arrayType", new ArrayTypeNode());
        kindMap.put("method", new MethodTypeNode());
        kindMap.put("field", new FieldTypeNode());
        kindMap.put("frame", new FrameTypeNode());
        kindMap.put("string", new StringTypeNode());
        kindMap.put("value", new ValueTypeNode());
        kindMap.put("byte", new ByteTypeNode());
        kindMap.put("location", new LocationTypeNode());
        kindMap.put("tagged-object", new TaggedObjectTypeNode());
        kindMap.put("referenceTypeID", new ReferenceIDTypeNode());
        kindMap.put("typed-sequence", new ArrayRegionTypeNode());
        kindMap.put("untagged-value", new UntaggedValueTypeNode());
    }

    RootNode items() throws IOException {
        List list = new ArrayList();
        
        while (izer.nextToken() != StreamTokenizer.TT_EOF) {
            izer.pushBack();
            list.add(item());
        }
        RootNode node =  new RootNode();
        node.set("Root", list, 1);
        return node;
    }
        
    Node item() throws IOException {
        switch (izer.nextToken()) {
            case StreamTokenizer.TT_EOF:
                error("Unexpect end-of-file");
                return null;

            case StreamTokenizer.TT_WORD: {
                String name = izer.sval;
                if (izer.nextToken() == '=') {
                    int ntok = izer.nextToken();
                    if (ntok == StreamTokenizer.TT_WORD) {
                        return new NameValueNode(name, izer.sval);
                    } else if (ntok == '\'') {
                        return new NameValueNode(name, izer.sval.charAt(0));
                    } else {
                        error("Expected value after: " + name + " =");
                        return null;
                    }
                } else {
                    izer.pushBack();
                    return new NameNode(name);
                }
            }

            case '"':
                return new CommentNode(izer.sval);

            case '(': {
                if (izer.nextToken() == StreamTokenizer.TT_WORD) {
                    String kind = izer.sval;
                    List list = new ArrayList();
        
                    while (izer.nextToken() != ')') {
                        izer.pushBack();
                        list.add(item());
                    }
                    Node proto = (Node)(kindMap.get(kind));
                    if (proto == null) {
                        error("Invalid kind: " + kind);
                        return null;
                    } else {
                        try {
                            Node node = (Node)proto.getClass().newInstance();
                            node.set(kind, list, izer.lineno());
                            return node;
                        } catch (Exception exc) {
                            error(exc.toString());
                        }
                    }
                } else {
                    error("Expected kind identifier, got " + izer.ttype +
                          " : " + izer.sval);
                    return null;
                }
            }

            default:
                error("Unexpected character: '" + (char)izer.ttype + "'");
                return null;
        }
    }

    void error(String errmsg) {
        System.err.println(Main.specSource + ":" + izer.lineno() +
                           ": " + errmsg);
        System.exit(1);
    }
}
                    
                

                
