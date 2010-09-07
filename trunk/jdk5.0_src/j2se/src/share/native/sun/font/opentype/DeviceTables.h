/*
 * @(#)DeviceTables.h	1.9 02/12/11
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#ifndef __DEVICETABLES_H
#define __DEVICETABLES_H

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "GlyphIterator.h"
#include "GlyphPositionAdjustments.h"

struct DeviceTable
{
    le_uint16  startSize;
    le_uint16  endSize;
    le_uint16  deltaFormat;
    le_uint16  deltaValues[ANY_NUMBER];

    le_int16   getAdjustment(le_uint16 ppem) const;

private:
    static const le_uint16 fieldMasks[];
    static const le_uint16 fieldSignBits[];
    static const le_uint16 fieldBits[];
};


#endif


