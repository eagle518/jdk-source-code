
/*
 * @(#)CompositeLayoutEngine.h	1.2 01/12/03 
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#ifndef __COMPOSITELAYOUTENGINE_H
#define __COMPOSITELAYOUTENGINE_H

#ifndef __LETYPES_H
#include "LETypes.h"
#endif

#include "LayoutEngine.h"

#include <string.h>

class LEFontInstance;
class LEGlyphFilter;
class FontInstanceAdapter;

/**
 * This is an instance of LayoutEngine which implements
 * 32 bit glyph codes. This is needed to handle composite
 * fonts, which use a 16 bit font relative glyph code ORed
 * with an 8 bit font index.
 *
 * This class explicitly references FontInstanceAdapter to
 * get access to 32 bit glyph codes, which aren't available
 * through LEFontInstance.
 *
 * Because this class is a direct descendent of LayoutEngine,
 * it only implements default processing.
 *
 * @see LayoutEngine
 * @see LEFontInstance
 * @see FontInstanceAdapter
 */
class CompositeLayoutEngine : public LayoutEngine
{
protected:
	/**
	 * The (wide) output glyph array
	 */
    le_uint32 *fWideGlyphs;

	/**
	 * The font instance adapter for the text font.
	 *
	 * @see FontInstanceAdapter
	 */
    const FontInstanceAdapter *fFontInstanceAdapter;

    /**
	 * This overrides the default no argument constructor to make it
	 * difficult for clients to call it. Clients are expected to call
	 * layoutEngineFactory.
	 */
    CompositeLayoutEngine();

	/**
	 * This method does the glyph processing. It converts an array of characters
	 * into an array of 32 bit glyph indices and character indices. The characters to be
	 * processed are passed in a surrounding context. The context is specified as
	 * a starting address and a maximum character count. An offset and a count are
	 * used to specify the characters to be processed.
	 *
	 * The default implementation of this method only does character to glyph mapping.
	 * Subclasses needing more elaborate glyph processing must override this method.
	 *
	 * Input parameters:
	 * @param chars - the character context
	 * @param offset - the offset of the first character to process
	 * @param count - the number of characters to process
	 * @param max - the number of characters in the context.
	 * @param rightToLeft - true if the text is in a right to left directional run
	 *
	 * Output parameters:
	 * @param glyphs - the 32 bit glyph index array
	 * @param charIndices - the character index array
	 * @param success - set to an error code if the operation fails
	 *
	 * @return the number of glyphs in the glyph index array
	 */
    virtual le_int32 computeGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft, le_uint32 *&glyphs, le_int32 *&charIndices, LEErrorCode &success);

	/**
	 * This method does basic glyph positioning. The default implementation positions
	 * the glyphs based on their advance widths. This is sufficient for most uses. It
	 * is not expected that many subclasses will override this method.
	 *
	 * Input parameters:
	 * @param glyphs - the input 32 bit glyph array
	 * @param glyphCount - the number of glyphs in the glyph array
	 * @param x - the starting X position
	 * @param y - the starting Y position
	 *
	 * Output parameters:
	 * @param positions - the output X and Y positions (two entries per glyph)
	 */
    virtual void positionGlyphs(const le_uint32 glyphs[], le_int32 glyphCount, float x, float y, float *&positions, LEErrorCode &success);

	/**
	 * This method does character to glyph mapping. The default implementation
	 * uses the font instance to do the mapping. It will allocate the glyph and
	 * character index arrays if they're not already allocated. If it allocates the
	 * character index array, it will fill it it.
	 *
	 * This method supports right to left
	 * text with the ability to store the glyphs in reverse order, and by supporting
	 * character mirroring, which will replace a character which has a left and right
	 * form, such as parens, with the opposite form before mapping it to a glyph index.
	 *
	 * Input parameters:
	 * @param chars - the input character context
	 * @param offset - the offset of the first character to be mapped
	 * @param count - the number of characters to be mapped
	 * @param reverse - if true, the output will be in reverse order
	 * @param mirror - if true, do character mirroring
	 *
	 * Output parameters:
	 * @param glyphs - the 32 bit glyph array
	 * @param charIndices - the character index array
	 * @param success - set to an error code if the operation fails
	 *
	 * @see LEFontInstance
	 */
    virtual void mapCharsToGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, le_bool mirror, le_uint32 *&glyphs, le_int32 *&charIndices, LEErrorCode &success);

public:
	/**
	 * This constructs an instance for a given font, script and language. Subclass constructors
	 * must call this constructor.
	 *
	 * @param fontInstance - the font for the text
	 * @param scriptCode - the script for the text
	 * @param langaugeCode - the language for the text
	 *
	 * @see LEFontInstance
	 * @see ScriptAndLanguageTags.h
	 */
    CompositeLayoutEngine(const FontInstanceAdapter *fontInstance, le_int32 scriptCode, le_int32 languageCode);

	/**
	 * The destructor. It will free any storage allocated for the
	 * glyph, character index and position arrays by calling the reset
	 * method. It is declared virtual so that it will be invoked by the
	 * subclass destructors.
	 */
    virtual ~CompositeLayoutEngine();

	/**
	 * This method will invoke the layout steps in their correct order by calling
	 * the 32 bit versions of the computeGlyphs and positionGlyphs methods.(It doesn't
	 * call the adjustGlyphPositions method because that doesn't apply for default
	 * processing.) It will compute the glyph, character index and position arrays.
	 *
	 * @param chars - the input character context
	 * @param offset - the offset of the first character to process
	 * @param count - the number of characters to process
	 * @param max - the number of characters in the input context
	 * @param rightToLeft - true if the characers are in a right to left directional run
	 * @param x - the initial X position
	 * @param y - the initial Y position
	 * @param success - output parameter set to an error code if the operation fails
	 *
	 * @return the number of glyphs in the glyph array
	 *
	 * Note; the glyph, character index and position array can be accessed
	 * using the getter method below.
	 */
    le_int32 layoutChars(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft, float x, float y, LEErrorCode &success);

	/**
	 * This method copies the glyph array into a caller supplied array,
	 * ORing in extra bits. (This functionality is needed by the JDK,
	 * which uses 32 bits pre glyph idex, with the high 16 bits encoding
	 * the composite font slot number)
	 *
	 * @param glyphs - the destination (32 bit) glyph array
	 * @param extraBits - this value will be ORed with each glyph index
	 * @param success - set to an error code if the operation fails
	 */
    void getGlyphs(le_uint32 glyphs[], le_uint32 extraBits, LEErrorCode &success) const;

	/**
	 * This method frees the glyph, character index and position arrays
	 * so that the LayoutEngine can be reused to layout a different
	 * characer array. (This method is also called by the destructor)
	 */
    virtual void reset();
    
};

#endif

