/*
 * @(#)GlyphDefinitionTables.cpp	1.9 05/05/11
 *
 * (C) Copyright IBM Corp. 1998 - 2004 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "GlyphDefinitionTables.h"
#include "LESwaps.h"

const GlyphClassDefinitionTable *GlyphDefinitionTableHeader::getGlyphClassDefinitionTable() const
{
    return (const GlyphClassDefinitionTable *) ((char *) this + SWAPW(glyphClassDefOffset));
}

const AttachmentListTable *GlyphDefinitionTableHeader::getAttachmentListTable() const
{
    return (const AttachmentListTable *) ((char *) this + SWAPW(attachListOffset));
}

const LigatureCaretListTable *GlyphDefinitionTableHeader::getLigatureCaretListTable() const
{
    return (const LigatureCaretListTable *) ((char *) this + SWAPW(ligCaretListOffset));
}

const MarkAttachClassDefinitionTable *GlyphDefinitionTableHeader::getMarkAttachClassDefinitionTable() const
{
    return (const MarkAttachClassDefinitionTable *) ((char *) this + SWAPW(MarkAttachClassDefOffset));
}

