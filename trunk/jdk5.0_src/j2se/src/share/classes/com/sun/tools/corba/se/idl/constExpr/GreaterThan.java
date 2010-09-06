/*
 * @(#)GreaterThan.java	1.12 03/12/19
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
 * @(#)GreaterThan.java	1.12 03/12/19
 */

package com.sun.tools.corba.se.idl.constExpr;

// NOTES:

import com.sun.tools.corba.se.idl.Util;
import java.math.BigInteger;

public class GreaterThan extends BinaryExpr
{
  protected GreaterThan (Expression leftOperand, Expression rightOperand)
  {
    super (">", leftOperand, rightOperand);
  } // ctor

  public Object evaluate () throws EvaluationException
  {
    try
    {
      Object left = left ().evaluate ();
      Object right = right ().evaluate ();
      if (left instanceof Boolean)
      {
        String[] parameters = {Util.getMessage ("EvaluationException.greaterThan"), left ().value ().getClass ().getName (), right ().value ().getClass ().getName ()};
        throw new EvaluationException (Util.getMessage ("EvaluationException.1", parameters));
      }
      else
      {
        Number l = (Number)left;
        Number r = (Number)right ().evaluate ();
        if (l instanceof Float || l instanceof Double || r instanceof Float || r instanceof Double)
          value (new Boolean (l.doubleValue () > r.doubleValue ()));
        else
          //daz          value (new Boolean (l.longValue () > r.longValue ()));
          value (new Boolean ( ((BigInteger)l).compareTo ((BigInteger)r) > 0));
      }
    }
    catch (ClassCastException e)
    {
      String[] parameters = {Util.getMessage ("EvaluationException.greaterThan"), left ().value ().getClass ().getName (), right ().value ().getClass ().getName ()};
      throw new EvaluationException (Util.getMessage ("EvaluationException.1", parameters));
    }
    return value ();
  } // evaluate
} // class GreaterThan
