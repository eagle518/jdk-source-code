#ifndef _XmXOC_h
#define _XmXOC_h

#include <X11/Intrinsic.h>	/* for String, Cardinal, etc... */
#include <X11/Xlibint.h>	/* for Xmalloc, Xfree */
#include <X11/Xlcint.h>

#include <sys/layout.h>
#include <dlfcn.h>

typedef String AttrObject;	/* This one ought to be in a header somewhere */

#ifndef ALLOCATE_LOCAL
#define ALLOCATE_LOCAL(size)	XtMalloc(size)
#define DEALLOCATE_LOCAL(ptr)	XtFree(ptr)
#endif /* ALLOCATE_LOCAL */

/* These defines would move to Xlib.h */
#define CTL_CACHE_SIZE		512
#define BYTE			unsigned char
#define MAX_PLANES      	32
#define AT_VISUAL_LINE_END	-1
#define AT_VISUAL_LINE_START	-2
#define LINE_END		-3
#define LINE_START		-4
#define CANT_DELETE		-5
#define NO_WORD			-6
#define NO_CELL			-7
#define LAST_WORD		-8
#define FIRST_WORD		-9
#define LAST_CELL		-10
#define LOGICAL			-11
#define VISUAL			-12

#define XNLayoutAttrObject	"layoutAttrObject"
#define XNLayoutModifier	"layoutModifier"

typedef Status (*XocTextPerCharExtentsProc)(
    XOC		        /* oc */,
    _Xconst void*	/* text */,
    Boolean             /* is_wchar */,
    int			/* text_len */,
    XSegment*		/* ink_extents_buffer */,
    XSegment*		/* logical_extents_buffer */,
    int			/* buffer_size */,
    int*		/* num_chars */,
    XSegment*		/* max_ink_extents */,
    XSegment*		/* max_logical_extents */
);

typedef struct {
    unsigned int fid;	 /* Font Id			*/
    char	 *fname; /* Human readable charset name	*/
    XFontStruct	 *mfont; /* XFontStruct */
} mfontRec;

typedef struct {
    mfontRec	MFontRec[MAX_PLANES];
    int		num_entries;
} MultiFont;

enum { COPYPARTIAL, COPYALL, COPYNONE }; /* Cache State Values */
/*
  This structure is used to cache Layout transformations in the XmXOC.
 */
typedef struct {
    /* These are the values passed in to XmXOC entry points */
    Boolean	is_wchar;
    void	*char_buf;
    size_t	char_buf_index; /* Maybe used one day */
    
    /* These are the values returned by transformations */
    int		ret;
    void	*GlyphBuf;
    size_t	BufSize; /* num of characters for which glyph is allocated */
    size_t	CharNum;
    size_t	GlyphNum;
    size_t	*CPos2GPos; /* 'C'haracter 'Pos'ition '2' 'G'lyph 'Pos'ition mapping */
    size_t	*GPos2CPos; /* 'G'lyph 'Pos'ition '2' 'C'haracter 'Pos'ition mapping */
    BYTE	*CharProps; /* Properties of the character String */
    XtEnum	cacheState; /* Are all cache values filled ? */
} XmXOCCacheRec, *XmXOCCache;

typedef struct {
    /* These are the PLS apis that are used frequently by XmXOC */
    void 	 *so;			/* liblayout.so */
    LayoutObject (*m_create_layout)();
    int		 (*m_destroy_layout)();
    int		 (*m_getvalues_layout)();
    int		 (*m_setvalues_layout)();
    int		 (*m_transform_layout)();
    int		 (*m_wtransform_layout)();
} XmXOCFuncRec;

/* All of this below would migrate into a new "class" of Xlib XOC */
typedef struct {
    XOCRec	  rec;			/* copy of xoc */
    XOC		  xoc;			/* pointer to standard Xlib XOC that Xm will wrap */
    AttrObject    layout_attr_object;	/* resourced as XNLayoutAttrObject (CG) */
    String        layout_modifier;	/* resourced as XNLayoutModifier (CSG) */
    LayoutObject  layout_object;
    size_t        layout_max_expand;
    int		  layout_shape_charset_size;
    XmXOCCacheRec layout_cache;
    Boolean       layout_active;
    Boolean	  ule_active;		/* Check for MultiScript Locale */
    XmXOCFuncRec  *fnRec;		/* All frequently used PLS apis */
    MultiFont     *uleFonts;		/* Holds FontTable in MultiScript Locale */
    XocTextPerCharExtentsProc   xoc_extents_per_char;
} XmXOCRec, *XmXOC;

typedef enum {XocCONST_POS, XocRELATIVE_POS} XocPosSelectionType;

/* A couple of utility functions */
extern XmXOC
XmCreateXmXOC(XOC xoc, String modifier, XmXOCFuncRec *fnRec);
extern void 
_XRectangleToXSegment(Boolean left_to_right, XSegment *seg, XRectangle *rect);
extern void 
_XSegmentToXRectangle(XRectangle *rect, XSegment *seg);
extern Status 
_XFontStructTextPerCharExtents(XFontStruct	*font,
			       char		*str,
			       int		length,
			       XRectangle	*ink_array_return,
			       XRectangle	*logical_array_return,
			       int		array_size,
			       int		*num_chars_return,
			       XRectangle	*overall_ink_return,
			       XRectangle	*overall_logical_return);
extern Status 
_XFontStruct16TextPerCharExtents(XFontStruct	*font,
				 wchar_t	*str,
				 int		length,
				 XRectangle	*ink_array_return,
				 XRectangle	*logical_array_return,
				 int		array_size,
				 int		*num_chars_return,
				 XRectangle	*overall_ink_return,
				 XRectangle	*overall_logical_return);
extern Status XocTextPerCharExtents(XOC		oc,
				 _Xconst void	*string,
				 Boolean	is_wchar, 
				 int		num_chars,
				 XSegment	*ink_array_return,
				 XSegment	*logical_array_return,
				 int		array_size,
				 int		*num_chars_return,
				 XSegment	*overall_ink_return,
				 XSegment	*overall_logical_return);
extern int XocTextInfo(		XOC           	oc,
		       		_Xconst void	*string,
		       		Boolean      	is_wchar, 
		       		int		num_chars,
				unsigned char	*char_props,
				XmTextPosition	*text_i2o,
				XmTextPosition	*text_02i);
extern Status XocVisualScan(XFontSet		fontset,
			    char		*string,
			    Boolean		is_wchar, 
			    int			num_chars,
			    int			position,
			    XocPosSelectionType	pos_sel_type,
			    XmTextScanType	stype,
			    XmTextScanDirection	dir,
			    Boolean		include_ws,
			    XmTextPosition	*new_pos);
extern Status 
XocVisualCharDelInfo(XFontSet			fontset,
		     char			*string,
		     Boolean			is_wchar, 
		     int			num_chars,
		     int			position,
		     XmTextScanDirection	dir,
		     XmTextPosition		*del_cursor_position,
		     XmTextPosition		*new_pos);
extern int 
XocFindVisualWord(XFontSet			fontset,
		  char				*string,
		  Boolean			is_wchar, 
		  int				str_len,
		  XocPosSelectionType		pos_sel_type,
		  int				position,
		  XmTextScanDirection		dir,
		  XmTextPosition		*word_char_list, 
		  int				*num_chars,
		  XmTextPosition		*new_pos);
extern int XocCellScan(XFontSet			fontset,
		       char			*string,
		       Boolean			is_wchar, 
		       int			num_chars,
		       XmTextPosition		position,
		       XmTextScanDirection	dir,
		       XmTextPosition		*start_pos);
extern int XocFindCell(XFontSet			fontset,
		       char			*string,
		       Boolean			is_wchar, 
		       int			num_chars,
		       XmTextPosition		position,
		       XmTextScanDirection	dir,
		       XmTextPosition		*start_pos,
		       XmTextPosition		*end_pos);
#endif /* _XmXOC_h */
