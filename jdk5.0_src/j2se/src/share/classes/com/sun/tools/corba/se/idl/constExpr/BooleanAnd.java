/*
 * @(#)BooleanAnd.java	1.12 03/12/19
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
 * @(#)BooleanAnd.java	1.12 03/12/19
 */

package com.sun.tools.corba.se.idl.constExpr;

// NOTES:

import com.sun.tools.corba.se.idl.Util;
import java.math.BigInteger;

public class BooleanAnd extends BinaryExpr
{
  protected BooleanAnd (Expression leftOperand, Expression rightOperand)
  {
    super ("&&", leftOperand, rightOperand);
  } // ctor

  public Object evaluate () throws EvaluationException
  {
    try
    {
      Object tmpL = left ().evaluate ();
      Object tmpR = right ().evaluate ();
      Boolean l;
      Boolean r;

      //daz   if (tmpL instanceof Number)
      //        l = new Boolean (((Number)tmpL).longValue () != 0);
      //      else
      //        l = (Boolean)tmpL;
      if (tmpL instanceof Number)
      {
        if (tmpL instanceof BigInteger)
          l = new Boolean (((BigInteger)tmpL).compareTo (BigInteger.valueOf (0)) != 0);
        else
          l = new Boolean (((Number)tmpL).longValue () != 0);
      }
      else
        l = (Boolean)tmpL;
      //daz   if (tmpR instanceof Number)
      //        r = new Boolean (((Number)tmpR).longValue () != 0);
      //      else
      //        r = (Boolean)tmpR;
      if (tmpR instanceof Number)
      {
        if (tmpR instanceof BigInteger)
          r = new Boolean (((BigInteger)tmpR).compareTo (zero) != 0);
        else
          r = new Boolean (((Number)tmpR).longValue () != 0);
      }
      else
        r = (Boolean)tmpR;

      value (new Boolean (l.booleanValue () && r.booleanValue ()));
    }
    catch (ClassCastException e)
    {
      String[] parameters = {Util.getMessage ("EvaluationException.booleanAnd"), left ().value ().getClass ().getName (), right ().value ().getClass ().getName ()};
      throw new EvaluationException (Util.getMessage ("EvaluationException.1", parameters));
    }
    return value ();
  } // evaluate
} // class BooleanAnd
