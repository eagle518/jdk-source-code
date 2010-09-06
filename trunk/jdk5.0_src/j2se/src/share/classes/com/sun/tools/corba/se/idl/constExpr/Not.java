/*
 * @(#)Not.java	1.12 03/12/19
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
 * @(#)Not.java	1.12 03/12/19
 */

package com.sun.tools.corba.se.idl.constExpr;

// NOTES:

import com.sun.tools.corba.se.idl.Util;
import java.math.BigInteger;

public class Not extends UnaryExpr
{
  protected Not (Expression operand)
  {
    super ("~", operand);
  } // ctor

  public Object evaluate () throws EvaluationException
  {
    try
    {
      Number op = (Number)operand ().evaluate ();

      if (op instanceof Float || op instanceof Double)
      {
        String[] parameters = {Util.getMessage ("EvaluationException.not"), operand ().value ().getClass ().getName ()};
        throw new EvaluationException (Util.getMessage ("EvaluationException.2", parameters));
      }
      else
      {
        // Complement (~)
        //daz        value (new Long (~op.longValue ()));
        BigInteger b = (BigInteger)coerceToTarget((BigInteger)op);

        // Compute according to CORBA 2.1 specifications for specified type.
        if (type ().equals ("short") || type ().equals ("long") || type ().equals ("long long"))
          value (b.add (one).multiply (negOne));
        else if (type ().equals("unsigned short"))
          // "short" not CORBA compliant, but necessary for logical operations--size matters!
          value (twoPow16.subtract (one).subtract (b));
        else if (type ().equals ("unsigned long"))
          value (twoPow32.subtract (one).subtract (b));
        else if (type ().equals ("unsigned long long"))
          value (twoPow64.subtract (one).subtract (b));
        else
          value (b.not ());  // Should never execute...
      }
    }
    catch (ClassCastException e)
    {
      String[] parameters = {Util.getMessage ("EvaluationException.not"), operand ().value ().getClass ().getName ()};
      throw new EvaluationException (Util.getMessage ("EvaluationException.2", parameters));
    }
    return value ();
  } // evaluate
} // class Not
