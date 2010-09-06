/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)PluginPrint.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef PLUGINPRINT_H
#define PLUGINPRINT_H

#include "commonhdr.h"

class JavaPluginInstance5;

class PluginPrint{
 public:
  /* Construction of PluginPrint */
  PluginPrint(JavaPluginInstance5 *inst, JDPluginPrint* printinfo);
  
  /* Desctruction */
  virtual ~PluginPrint(void);

  /* Actual Printing method */
  JD_IMETHOD Print();
  
  /* If the request was a fullPrint */
  JD_IMETHOD FullPrint();

  /* For the request of embedPrint */
  JD_IMETHOD EmbedPrint();

protected:
  void sendRequest();
  
  /*Pointer to JavaPluginInstance */
  JavaPluginInstance5 *mInst;

  /* pointer to print info */
  JDPluginPrint *mPrintInfo;

};

#endif /*PLUGINPRINT_H */
  
