package com.sun.xml.internal.rngom.ast.builder;

import com.sun.xml.internal.rngom.ast.om.Location;
import com.sun.xml.internal.rngom.ast.om.ParsedElementAnnotation;
import com.sun.xml.internal.rngom.ast.om.ParsedPattern;
import com.sun.xml.internal.rngom.parse.*;


public interface DataPatternBuilder<
    P extends ParsedPattern,
    E extends ParsedElementAnnotation,
    L extends Location,
    A extends Annotations<E,L,CL>,
    CL extends CommentList<L>> {

  void addParam(String name, String value, Context context, String ns, L loc, A anno) throws BuildException;
  void annotation(E ea);
  P makePattern(L loc, A anno) throws BuildException;
  P makePattern(P except, L loc, A anno) throws BuildException;
}
