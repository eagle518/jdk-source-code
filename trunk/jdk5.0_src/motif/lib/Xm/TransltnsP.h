/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* $XConsortium: TransltnsP.h /main/13 1996/11/21 11:33:31 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmTransltnsP_h
#define _XmTransltnsP_h

#include <X11/Intrinsic.h>	/* for externalref */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _XmConst
#define _XmConst
#endif


externalref _XmConst char _XmArrowB_defaultTranslations[];
externalref _XmConst char _XmBulletinB_defaultTranslations[];
externalref _XmConst char _XmCascadeB_menubar_events[];
externalref _XmConst char _XmCascadeB_p_events[];
externalref _XmConst char _XmComboBox_defaultTranslations[];
externalref _XmConst char _XmComboBox_defaultAccelerators[];
externalref _XmConst char _XmComboBox_dropDownComboBoxAccelerators[];
externalref _XmConst char _XmComboBox_dropDownListTranslations[];
externalref _XmConst char _XmComboBox_textFocusTranslations[];
externalref _XmConst char _XmContainer_defaultTranslations[];
externalref _XmConst char _XmContainer_traversalTranslations[];
externalref _XmConst char _XmDisplay_baseTranslations[];
externalref _XmConst char _XmDragC_defaultTranslations[];
externalref _XmConst char _XmDrawingA_defaultTranslations[];
externalref _XmConst char _XmDrawingA_traversalTranslations[];
externalref _XmConst char _XmDrawnB_defaultTranslations[];
externalref _XmConst char _XmFrame_defaultTranslations[];
externalref _XmConst char _XmGrabShell_translations [];
externalref _XmConst char _XmLabel_defaultTranslations[];
externalref _XmConst char _XmLabel_menuTranslations[];
externalref _XmConst char _XmLabel_menu_traversal_events[];
externalref _XmConst char _XmList_ListXlations1[];
externalref _XmConst char _XmList_ListXlations2[];
externalref _XmConst char _XmManager_managerTraversalTranslations[];
externalref _XmConst char _XmManager_defaultTranslations[];
externalref _XmConst char _XmNotebook_manager_translations[];
externalref _XmConst char _XmNotebook_TabAccelerators[];
externalref _XmConst char _XmMenuShell_translations [];
externalref _XmConst char _XmPrimitive_defaultTranslations[];
externalref _XmConst char _XmPushB_defaultTranslations[];
externalref _XmConst char _XmPushB_menuTranslations[];
externalref _XmConst char _XmRowColumn_menu_traversal_table[];
externalref _XmConst char _XmRowColumn_bar_table[];
externalref _XmConst char _XmRowColumn_option_table[];
externalref _XmConst char _XmRowColumn_menu_table[];
externalref _XmConst char _XmSash_defTranslations[];
externalref _XmConst char _XmScrollBar_defaultTranslations[];
externalref _XmConst char _XmScrolledW_ScrolledWindowXlations[];
externalref _XmConst char _XmClipWindowTranslationTable[];
externalref _XmConst char _XmScrolledW_WorkWindowTranslationTable[];
externalref _XmConst char _XmSelectioB_defaultTextAccelerators[];
externalref _XmConst char _XmSpinB_defaultTranslations[];
externalref _XmConst char _XmSpinB_defaultAccelerators[];
externalref _XmConst char _XmTearOffB_overrideTranslations[];
externalref _XmConst char _XmTextF_EventBindings1[];
externalref _XmConst char _XmTextF_EventBindings2[]; 
externalref _XmConst char _XmTextF_EventBindings3[];
externalref _XmConst char _XmTextIn_XmTextEventBindings1[];
externalref _XmConst char _XmTextIn_XmTextEventBindings2[];
externalref _XmConst char _XmTextIn_XmTextEventBindings3[];
externalref _XmConst char _XmTextIn_XmTextVEventBindings[];
externalref _XmConst char _XmToggleB_defaultTranslations[];
externalref _XmConst char _XmToggleB_menuTranslations[];
externalref _XmConst char _XmVirtKeys_fallbackBindingString[];

/*
 * The following keybindings have been "grandfathered" 
 * for backward compatablility.
 */
externalref _XmConst char _XmVirtKeys_acornFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_apolloFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_dgFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_decFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_dblclkFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_hpFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_ibmFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_ingrFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_megatekFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_motorolaFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_sgiFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_siemensWx200FallbackBindingString[];
externalref _XmConst char _XmVirtKeys_siemens9733FallbackBindingString[];
externalref _XmConst char _XmVirtKeys_sunFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_tekFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_pcFallbackBindingString[];

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTransltnsP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
