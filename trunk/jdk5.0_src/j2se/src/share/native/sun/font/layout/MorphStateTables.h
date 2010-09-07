/*
 * @(#)MorphStateTables.h	1.5 01/10/09
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000 - All Rights Reserved
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
