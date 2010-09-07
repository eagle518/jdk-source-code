/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)PluginPrint.cpp	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "commonhdr.h"
#include "commandprotocol.h"
#include "remotejni.h"
#include "IPluginInstance.h"
#include "IJVMPluginInstance.h"
#include "JavaPluginInstance5.h"
#include "JavaPluginFactory5.h"
#include "JavaVM5.h"
#include "CWriteBuffer.h"
#include "PluginPrint.h"

PluginPrint::PluginPrint(JavaPluginInstance5 *inst, 
			 JDPluginPrint* printinfo)
  :mInst(inst), mPrintInfo(printinfo)
{
  mInst->AddRef();
}

/* Desctruction */
PluginPrint::~PluginPrint()
{
  if (mInst != NULL)
    mInst->Release();
  
}

/* Actual Printing method */
JD_IMETHODIMP PluginPrint::Print()
{
  if (mPrintInfo->mode == JDPluginMode_Full)
    {
      return FullPrint();
    }
  else 
    {
      return EmbedPrint();
    }
}


/* If the request was a fullPrint */
JD_IMETHODIMP PluginPrint::FullPrint()
{
  mPrintInfo->print.fullPrint.pluginPrinted=JD_FALSE;
  return JD_OK;
}

/*
 * Embed Print does the following:
 * 1) Send message to Plugin to print requested area
 * 2) Request is send and received ack that print pipe is ready
 * 3) Poll the printpipe untile you receive ok message
 * 
 *
 */
JD_IMETHODIMP PluginPrint::EmbedPrint()
{
  FILE *fp;
  JavaVM5 *jvm ;
  sendRequest();
  JDPluginEmbedPrint ep;
  ep = mPrintInfo->print.embedPrint;
  void *platformPrint = (JDPluginPrintCallbackStruct*)(ep.platformPrint);
  
  fp = ((JDPluginPrintCallbackStruct *)(platformPrint))->fp;
  jvm =mInst->GetPluginFactory()->GetJavaVM();
  
  if (jvm == NULL) return JD_ERROR_FAILURE;
  
  jvm->ReceivePrinting(fp);
  
  return JD_OK;

}

void PluginPrint::sendRequest()
{
  CWriteBuffer wb;
  JDPluginEmbedPrint embedPrint= mPrintInfo->print.embedPrint;
  JDPluginWindow window=embedPrint.window;
  wb.putInt(JAVA_PLUGIN_PRINT);
  wb.putInt( mInst->GetPluginNumber());
  wb.putInt(window.x);
  wb.putInt(window.y);
  wb.putInt(window.width);
  wb.putInt(window.height);
  mInst->GetPluginFactory()->SendRequest(wb, FALSE);
} 
