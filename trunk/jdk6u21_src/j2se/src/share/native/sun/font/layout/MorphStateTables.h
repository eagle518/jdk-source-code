/*
 * @(#)MorphStateTables.h	1.6 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __MORPHSTATETABLES_H
#define __MORPHSTATETABLES_H

#include "LETypes.h"
#include "LayoutTables.h"
#include "MorphTables.h"
#include "StateTables.h"

struct MorphStateTableHeader : MorphSubtableHeader
{
    StateTableHeader stHeader;
};

#endif
