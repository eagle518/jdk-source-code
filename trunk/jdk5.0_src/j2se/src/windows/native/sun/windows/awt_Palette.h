/*
 * @(#)awt_Palette.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_PALETTE_H
#define AWT_PALETTE_H

#include "awt_Win32GraphicsDevice.h"

#define CMAPSIZE	256	// number of colors to use in default cmap

#define GS_NOTGRAY	0	// screen is not grayscale
#define GS_INDEXGRAY	1	// screen is 8-bit indexed with several
				//  gray colormap entries
#define GS_STATICGRAY	2	// screen is 8-bit with 256 gray values
				// from 0 to 255 (no index table used)
#define GS_NONLINGRAY   3       /* screen is 8-bit with 256 gray values
                                   in non-monotonic order */

class AwtWin32GraphicsDevice;

class AwtPalette {
    
public:
    HPALETTE		    Select(HDC hDC);

    void		    Realize(HDC hDC);

    HPALETTE		    GetPalette() { return logicalPalette; }

    static void		    DisableCustomPalette();

    static BOOL		    UseCustomPalette();

			    AwtPalette(AwtWin32GraphicsDevice *device);

    static int		    FetchPaletteEntries(HDC hDC, PALETTEENTRY* pPalEntries);
    int			    GetGSType(PALETTEENTRY* pPalEntries);

    BOOL		    Update();
    void		    UpdateLogical();

    unsigned int	    *GetSystemEntries() {return systemEntries; }
    unsigned int	    *GetLogicalEntries() {return logicalEntries; }
    unsigned char	    *GetSystemInverseLUT() { return systemInverseLUT; }

private:
    static BOOL		    m_useCustomPalette;

    unsigned int	    logicalEntries[256];
    unsigned int	    systemEntries[256];
    PALETTEENTRY	    systemEntriesWin32[256];  // cached to eliminate
					      // copying it when unnec.
    int			    numSystemEntries;
    HPALETTE		    logicalPalette;

    AwtWin32GraphicsDevice  *device;
    unsigned char	    *systemInverseLUT;

    /**
     * This custom palette is derived from the IE palette.
     * Previously, we used a custom palette that used a patented
     * algorithm for getting an evently distributed color space.
     * But given the realites of desktop and web graphics, it seems
     * more important to use a more standard palette, especially one
     * that agrees with the predominant browser.  The browser uses
     * a slightly modified 6x6x6 colorcube plus a gray ramp plus a
     * few other colors.  We still flash with Netscape, but we end
     * up using a very similar palette (Netscape uses a 6x6x6 color
     * cube as well); the entries are just in different places (thus
     * the flash).
     * Another possible solution to use a standard palette would be
     * to use the CreateHalftonePalette() call of win32.  This gives
     * us the IE palette on win98, but totally different palettes on
     * different versions of Windows.  We should at least use the same
     * colors on different flavors of the same platform...
     * The values coded below should be used for entries 10 through
     * 245 of our custom palette.  Entries 0-9 and 246-255 should be
     * retrieved from the current system palette, to ensure that we
     * are working well with the current desktop palette.
     *
     * The palette is initialized in awt_CustomPaletteDef.h
     */
    static PALETTEENTRY	    customPalette[236];
};



#endif AWT_PALETTE_H
