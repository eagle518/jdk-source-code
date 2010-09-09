package com.sun.xml.internal.rngom.ast.builder;

import com.sun.xml.internal.rngom.ast.om.Location;


public interface CommentList<L extends Location> {
  void addComment(String value, L loc) throws BuildException;
}
