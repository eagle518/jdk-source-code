/*
 * @(#)BinaryExpr.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * COMPONENT_NAME: idl.parser
 *
 * ORIGINS: 27
 *
 * Licensed Materials - Property of IBM
 * 5639-D57 (C) COPYRIGHT International Business Machines Corp. 1997, 1999
 * RMI-IIOP v1.0
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * @(#)BinaryExpr.java	1.12 03/12/19
 */

package com.sun.tools.corba.se.idl.constExpr;

// NOTES:

import com.sun.tools.corba.se.idl.Util;
import java.math.BigInteger;

public abstract class BinaryExpr extends Expression
{
  public BinaryExpr (String operation, Expression leftOperand, Expression rightOperand)
  {
    _op    = operation;
    _left  = leftOperand;
    _right = rightOperand;
  } // ctor

  public void op (String op) {_op = (op == null)? "": op;}
  public String op () {return _op;}

  public void       left (Expression left) {_left = left;}
  public Expression left () {return _left;}

  public void       right (Expression right) {_right = right;}
  public Expression right () {return _right;}

  private String     _op    = "";
  private Expression _left  = null;
  private Expression _right = null;
} // class BinaryExpr
