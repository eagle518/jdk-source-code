/* $XConsortium: sharedlib.c /main/5 1996/01/25 20:57:39 pascale $ */
/*
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 */
/*
 * HISTORY
 */

#include "XmP.h"
#include "ArrowBGP.h"
#include "ArrowBP.h"
#include "BaseClassP.h"
#include "BulletinBP.h"
#include "CacheP.h"
#include "CascadeBGP.h"
#include "CascadeBP.h"
#include "CommandP.h"
#include "DesktopP.h"
#include "DialogSEP.h"
#include "DialogSP.h"
#include "DisplayP.h"
#include "DragCP.h"
#include "DragIconP.h"
#include "DragOverSP.h"
#include "DrawP.h"
#include "DrawingAP.h"
#include "DrawnBP.h"
#include "DropSMgrP.h"
#include "DropTransP.h"
#include "ExtObjectP.h"
#include "FileSBP.h"
#include "FormP.h"
#include "FrameP.h"
#include "GadgetP.h"
#include "LabelGP.h"
#include "LabelP.h"
#include "ListP.h"
#include "MainWP.h"
#include "ManagerP.h"
#include "MenuShellP.h"
#include "MenuUtilP.h"
#include "MessageBP.h"
#include "PanedWP.h"
#include "PrimitiveP.h"
#include "ProtocolsP.h"
#include "PushBGP.h"
#include "PushBP.h"
#include "RowColumnP.h"
#include "SashP.h"
#include "ScaleP.h"
#include "ScreenP.h"
#include "ScrollBarP.h"
#include "ScrolledWP.h"
#include "SelectioBP.h"
#include "SeparatoGP.h"
#include "SeparatorP.h"
#include "ShellEP.h"
#include "TearOffBP.h"
#include "TearOffP.h"
#include "TextFP.h"
#include "TextInP.h"
#include "TextP.h"
#include "ToggleBGP.h"
#include "ToggleBP.h"
#include "TransltnsP.h"
#include "VaSimpleP.h"
#include "VendorSEP.h"
#include "VendorSP.h"

extern XmArrowButtonClassRec xmArrowButtonClassRec;
WidgetClass xmArrowButtonWidgetClass = (WidgetClass) &xmArrowButtonClassRec;

extern XmArrowButtonGadgetClassRec xmArrowButtonGadgetClassRec;
WidgetClass xmArrowButtonGadgetClass = (WidgetClass) &xmArrowButtonGadgetClassRec;

extern XmBulletinBoardClassRec xmBulletinBoardClassRec;
WidgetClass xmBulletinBoardWidgetClass = (WidgetClass) &xmBulletinBoardClassRec;

extern XmCascadeButtonClassRec xmCascadeButtonClassRec;
WidgetClass xmCascadeButtonWidgetClass = (WidgetClass) &xmCascadeButtonClassRec;

extern XmCascadeButtonGadgetClassRec xmCascadeButtonGadgetClassRec;
WidgetClass xmCascadeButtonGadgetClass = (WidgetClass) &xmCascadeButtonGadgetClassRec;

extern XmCommandClassRec xmCommandClassRec;
WidgetClass xmCommandWidgetClass = (WidgetClass) &xmCommandClassRec;

extern XmDesktopClassRec xmDesktopClassRec;
WidgetClass xmDesktopClass = (WidgetClass) &xmDesktopClassRec;

extern XmDialogShellClassRec xmDialogShellClassRec;
WidgetClass xmDialogShellWidgetClass = (WidgetClass) &xmDialogShellClassRec;

extern XmDialogShellExtClassRec xmDialogShellExtClassRec;
WidgetClass xmDialogShellExtObjectClass = (WidgetClass) &xmDialogShellExtClassRec;

extern XmDisplayClassRec xmDisplayClassRec;
WidgetClass xmDisplayClass = (WidgetClass) &xmDisplayClassRec;

extern XmDragContextClassRec xmDragContextClassRec;
WidgetClass xmDragContextClass = (WidgetClass) &xmDragContextClassRec;

extern XmDragIconClassRec xmDragIconClassRec;
WidgetClass xmDragIconObjectClass = (WidgetClass) &xmDragIconClassRec;

extern XmDragOverShellClassRec xmDragOverShellClassRec;
WidgetClass xmDragOverShellWidgetClass = (WidgetClass) &xmDragOverShellClassRec;

extern XmDrawingAreaClassRec xmDrawingAreaClassRec;
WidgetClass xmDrawingAreaWidgetClass = (WidgetClass) & xmDrawingAreaClassRec;

extern XmDrawnButtonClassRec xmDrawnButtonClassRec;
WidgetClass xmDrawnButtonWidgetClass = (WidgetClass) &xmDrawnButtonClassRec;

extern XmDropSiteManagerClassRec xmDropSiteManagerClassRec;
WidgetClass xmDropSiteManagerObjectClass = (WidgetClass) &xmDropSiteManagerClassRec;

extern XmDropTransferClassRec xmDropTransferClassRec;
WidgetClass xmDropTransferObjectClass = (WidgetClass) &xmDropTransferClassRec;

extern XmExtClassRec xmExtClassRec;
WidgetClass xmExtObjectClass = (WidgetClass) &xmExtClassRec;

extern XmFileSelectionBoxClassRec xmFileSelectionBoxClassRec;
WidgetClass xmFileSelectionBoxWidgetClass = (WidgetClass) &xmFileSelectionBoxClassRec;

extern XmFormClassRec xmFormClassRec;
WidgetClass xmFormWidgetClass = (WidgetClass) &xmFormClassRec;

extern XmFrameClassRec xmFrameClassRec;
WidgetClass xmFrameWidgetClass = (WidgetClass) &xmFrameClassRec;

extern XmGadgetClassRec xmGadgetClassRec;
WidgetClass xmGadgetClass = (WidgetClass) &xmGadgetClassRec;

extern XmLabelClassRec xmLabelClassRec;
WidgetClass xmLabelWidgetClass = (WidgetClass) &xmLabelClassRec;

extern XmLabelGadgetClassRec xmLabelGadgetClassRec;
WidgetClass xmLabelGadgetClass = (WidgetClass) &xmLabelGadgetClassRec;

extern XmListClassRec xmListClassRec;
WidgetClass xmListWidgetClass = (WidgetClass) &xmListClassRec;

extern XmMainWindowClassRec xmMainWindowClassRec;
WidgetClass xmMainWindowWidgetClass = (WidgetClass) &xmMainWindowClassRec;

extern XmManagerClassRec xmManagerClassRec;
WidgetClass xmManagerWidgetClass = (WidgetClass) &xmManagerClassRec;

extern XmPanedWindowClassRec xmPanedWindowClassRec;
WidgetClass xmPanedWindowWidgetClass = (WidgetClass) &xmPanedWindowClassRec;

extern XmPrimitiveClassRec xmPrimitiveClassRec;
WidgetClass xmPrimitiveWidgetClass = (WidgetClass) &xmPrimitiveClassRec;

extern XmProtocolClassRec xmProtocolClassRec;
WidgetClass xmProtocolObjectClass = (WidgetClass) &xmProtocolClassRec;

extern XmPushButtonClassRec xmPushButtonClassRec;
WidgetClass xmPushButtonWidgetClass = (WidgetClass) &xmPushButtonClassRec;

extern XmPushButtonGadgetClassRec xmPushButtonGadgetClassRec;
WidgetClass xmPushButtonGadgetClass = (WidgetClass) &xmPushButtonGadgetClassRec;

extern XmRowColumnClassRec xmRowColumnClassRec;
WidgetClass xmRowColumnWidgetClass = (WidgetClass) &xmRowColumnClassRec;

extern XmSashClassRec xmSashClassRec;
WidgetClass xmSashWidgetClass = (WidgetClass) &xmSashClassRec;

extern XmScaleClassRec xmScaleClassRec;
WidgetClass xmScaleWidgetClass = (WidgetClass) &xmScaleClassRec;

extern XmScreenClassRec xmScreenClassRec;
WidgetClass xmScreenClass = (WidgetClass) &xmScreenClassRec;

extern XmScrollBarClassRec xmScrollBarClassRec;
WidgetClass xmScrollBarWidgetClass = (WidgetClass) &xmScrollBarClassRec;

extern XmScrolledWindowClassRec xmScrolledWindowClassRec;
WidgetClass xmScrolledWindowWidgetClass = (WidgetClass) &xmScrolledWindowClassRec;

extern XmSelectionBoxClassRec xmSelectionBoxClassRec;
WidgetClass xmSelectionBoxWidgetClass = (WidgetClass) &xmSelectionBoxClassRec;

extern XmSeparatorGadgetClassRec xmSeparatorGadgetClassRec;
WidgetClass xmSeparatorGadgetClass = (WidgetClass) &xmSeparatorGadgetClassRec;

extern XmSeparatorClassRec xmSeparatorClassRec;
WidgetClass xmSeparatorWidgetClass = (WidgetClass) &xmSeparatorClassRec;

extern XmShellExtClassRec xmShellExtClassRec;
WidgetClass xmShellExtObjectClass = (WidgetClass) &xmShellExtClassRec;

extern XmTearOffButtonClassRec xmTearOffButtonClassRec;
WidgetClass xmTearOffButtonWidgetClass = (WidgetClass) &xmTearOffButtonClassRec;

/*
extern XmTemplateBoxClassRec xmTemplateBoxClassRec;
WidgetClass xmTemplateBoxWidgetClass = (WidgetClass) &xmTemplateBoxClassRec;
*/

extern XmTextClassRec xmTextClassRec;
WidgetClass xmTextWidgetClass = (WidgetClass) &xmTextClassRec;

extern XmTextFieldClassRec xmTextFieldClassRec;
WidgetClass xmTextFieldWidgetClass = (WidgetClass) &xmTextFieldClassRec;

extern XmToggleButtonClassRec xmToggleButtonClassRec;
WidgetClass xmToggleButtonWidgetClass = (WidgetClass) &xmToggleButtonClassRec;

extern XmToggleButtonGadgetClassRec xmToggleButtonGadgetClassRec;
WidgetClass xmToggleButtonGadgetClass = (WidgetClass) &xmToggleButtonGadgetClassRec;

extern XmVendorShellExtClassRec xmVendorShellExtClassRec;
WidgetClass xmVendorShellExtObjectClass = (WidgetClass) &xmVendorShellExtClassRec;

WidgetClass xmDesktopObjectClass = (WidgetClass) &xmDesktopClassRec;
WidgetClass xmDisplayObjectClass = (WidgetClass) &xmDisplayClassRec;
WidgetClass xmScreenObjectClass = (WidgetClass) &xmScreenClassRec;

extern int _XmInheritClass = 0;
extern XrmQuark XmQmotif = NULLQUARK;
extern XmBaseClassExt * _Xm_fastPtr = NULL;


