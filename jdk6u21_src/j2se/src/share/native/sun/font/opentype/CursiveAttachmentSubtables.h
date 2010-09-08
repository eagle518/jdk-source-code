/*
 * @(#)CursiveAttachmentSubtables.h	1.3 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2005 - All Rights Reserved
 *
 */

#ifndef __CURSIVEATTACHMENTSUBTABLES_H
#define __CURSIVEATTACHMENTSUBTABLES_H

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "GlyphPositioningTables.h"

class LEFontInstance;
class GlyphIterator;

struct EntryExitRecord
{
    Offset entryAnchor;
    Offset exitAnchor;
};

struct CursiveAttachmentSubtable : GlyphPositioningSubtable
{
    le_uint16 entryExitCount;
    EntryExitRecord entryExitRecords[ANY_NUMBER];

    le_uint32  process(GlyphIterator *glyphIterator, const LEFontInstance *fontInstance) const;
};

#endif

