/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  
 */

provider hs_private {
  probe hashtable__new_entry(void*, uintptr_t, void*); 
  probe safepoint__begin();
  probe safepoint__end();
  probe cms__initmark__begin();
  probe cms__initmark__end();
  probe cms__remark__begin();
  probe cms__remark__end();
};

#pragma D attributes Private/Private/Common provider hs_private provider
#pragma D attributes Private/Private/Unknown provider hs_private module
#pragma D attributes Private/Private/Unknown provider hs_private function
#pragma D attributes Private/Private/Common provider hs_private name
#pragma D attributes Private/Private/Common provider hs_private args

