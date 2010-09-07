/*
 * @(#)awt_DesktopProperties.cpp	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "stdhdrs.h"
#include "mmsystem.h"
#include "jlong.h"
#include "awt.h"
#include "awt_DesktopProperties.h"
#include "awt_dlls.h"
#include "sun_awt_windows_WDesktopProperties.h"
#include "java_awt_Font.h"
#include "awtmsg.h"
#include "Zmouse.h"
#include "shellapi.h"

// WDesktopProperties fields
jfieldID AwtDesktopProperties::pDataID = 0;
jmethodID AwtDesktopProperties::setBooleanPropertyID = 0;
jmethodID AwtDesktopProperties::setIntegerPropertyID = 0;
jmethodID AwtDesktopProperties::setStringPropertyID = 0;
jmethodID AwtDesktopProperties::setColorPropertyID = 0;
jmethodID AwtDesktopProperties::setFontPropertyID = 0;
jmethodID AwtDesktopProperties::setSoundPropertyID = 0;

typedef VOID (WINAPI *SHGetSettingsType)(LPSHELLFLAGSTATE, DWORD);
static HMODULE libShell32 = NULL;
static SHGetSettingsType fn_SHGetSettings;

AwtDesktopProperties::AwtDesktopProperties(jobject self) {
    this->self = GetEnv()->NewGlobalRef(self);
    GetEnv()->SetLongField( self, AwtDesktopProperties::pDataID,
			    ptr_to_jlong(this) );
}

AwtDesktopProperties::~AwtDesktopProperties() {
    GetEnv()->DeleteGlobalRef(self);
}

//
// Reads Windows parameters and sets the corresponding values
// in WDesktopProperties
//
void AwtDesktopProperties::GetWindowsParameters() {    
    if (GetEnv()->EnsureLocalCapacity(MAX_PROPERTIES) < 0) {
	DASSERT(0);
	return;
    }
    // this number defines the set of properties available, it is incremented
    // whenever more properties are added (in a public release of course)
    // for example, version 1 defines the properties available in Java SDK version 1.3.
    SetIntegerProperty( TEXT("win.properties.version"), AWT_DESKTOP_PROPERTIES_VERSION);
    GetNonClientParameters();
    GetIconParameters();
    GetColorParameters();
    GetOtherParameters();
    GetSoundEvents();
    GetSystemProperties();
    if (IS_WINXP) {
	GetXPStyleProperties();
    }
}

void AwtDesktopProperties::GetSystemProperties() {
    HDC dc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

    if (dc != NULL) {
        SetFontProperty(dc, ANSI_FIXED_FONT, TEXT("win.ansiFixed.font"));
        SetFontProperty(dc, ANSI_VAR_FONT, TEXT("win.ansiVar.font"));
        SetFontProperty(dc, DEVICE_DEFAULT_FONT, TEXT("win.deviceDefault.font"));
        SetFontProperty(dc, DEFAULT_GUI_FONT, TEXT("win.defaultGUI.font"));
        SetFontProperty(dc, OEM_FIXED_FONT, TEXT("win.oemFixed.font"));
        SetFontProperty(dc, SYSTEM_FONT, TEXT("win.system.font"));
        SetFontProperty(dc, SYSTEM_FIXED_FONT, TEXT("win.systemFixed.font"));
        DeleteDC(dc);
    }
}


// Determines what the font MS Shell Dlg maps to.
// Note that it uses malloc() and returns the pointer to allocated
// memory, so remember to use free() when you are done with its
// result.
static LPTSTR resolveShellDialogFont() {
    LPTSTR subKey = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes");

    HKEY handle;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &handle) != 0) {
	return NULL;
    }
    LPTSTR fontName = TEXT("MS Shell Dlg");
    // valueSize is in bytes, while valueChar is in characters.
    DWORD valueType, valueSize;
    if (RegQueryValueEx((HKEY)handle, fontName, NULL,
			&valueType, NULL, &valueSize) != 0) {
        // Couldn't find MS Shell Dlg
        RegCloseKey(handle);
        return NULL;
    }
    if (valueType != REG_SZ) {
        RegCloseKey(handle);
        return NULL;
    }
    LPTSTR buffer = (LPTSTR)safe_Malloc(valueSize);
    if (RegQueryValueEx((HKEY)handle, fontName, NULL,
			&valueType, (unsigned char *)buffer, &valueSize) != 0) {
	free(buffer);
        RegCloseKey(handle);
	return NULL;
    }
    RegCloseKey(handle);
    return buffer;
}

// Local function for getting values from the Windows registry
// Note that it uses malloc() and returns the pointer to allocated
// memory, so remember to use free() when you are done with its
// result.
static LPTSTR getXPStylePropFromReg(LPTSTR valueName) {
    LPTSTR subKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager");

    HKEY handle;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &handle) != 0) {
	return NULL;
    }
    // valueSize is in bytes, while valueChar is in characters.
    DWORD valueType, valueSize, valueChar;
    if (RegQueryValueEx((HKEY)handle, valueName, NULL,
			&valueType, NULL, &valueSize) != 0) {
        RegCloseKey(handle);
	return NULL;
    }
    LPTSTR buffer = (LPTSTR)safe_Malloc(valueSize);
    if (RegQueryValueEx((HKEY)handle, valueName, NULL,
			&valueType, (unsigned char *)buffer, &valueSize) != 0) {
	free(buffer);
        RegCloseKey(handle);
	return NULL;
    }
    RegCloseKey(handle);

    if (valueType == REG_EXPAND_SZ) {  
	// Pending: buffer must be null-terminated at this point
	valueChar = ExpandEnvironmentStrings(buffer, NULL, 0);
	LPTSTR buffer2 = (LPTSTR)safe_Malloc(valueChar*sizeof(TCHAR));
	ExpandEnvironmentStrings(buffer, buffer2, valueChar);
	free(buffer);
	return buffer2;
    } else if (valueType == REG_SZ) {  
	return buffer;
    } else {
	free(buffer);
	return NULL;
    }
}

// Used in AwtMenuItem to determine the color of top menus,
// since they depend on XP style. ThemeActive property is
// '1' for XP Style, '0' for Windows classic style.
BOOL AwtDesktopProperties::IsXPStyle() {
    LPTSTR style = getXPStylePropFromReg(TEXT("ThemeActive"));
    BOOL result = (style != NULL && *style == _T('1'));
    free(style);
    return result;
}

void AwtDesktopProperties::GetXPStyleProperties() {
    LPTSTR value;

    value = getXPStylePropFromReg(TEXT("ThemeActive"));
    SetBooleanProperty(TEXT("win.xpstyle.themeActive"), (value != NULL && *value == _T('1')));
    if (value != NULL) {
	free(value);
    }
    value = getXPStylePropFromReg(TEXT("DllName"));
    if (value != NULL) {
	SetStringProperty(TEXT("win.xpstyle.dllName"), value);
	free(value);
    }
    value = getXPStylePropFromReg(TEXT("SizeName"));
    if (value != NULL) {
	SetStringProperty(TEXT("win.xpstyle.sizeName"), value);
	free(value);
    }
    value = getXPStylePropFromReg(TEXT("ColorName"));
    if (value != NULL) {
	SetStringProperty(TEXT("win.xpstyle.colorName"), value);
	free(value);
    }
}


void AwtDesktopProperties::GetNonClientParameters() {
    //
    // general window properties
    //
    NONCLIENTMETRICS	ncmetrics;

    ncmetrics.cbSize = sizeof(ncmetrics);
    VERIFY( SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncmetrics.cbSize, &ncmetrics, FALSE) );

    SetFontProperty( TEXT("win.frame.captionFont"), ncmetrics.lfCaptionFont );
    SetIntegerProperty( TEXT("win.frame.captionHeight"), ncmetrics.iCaptionHeight );
    SetIntegerProperty( TEXT("win.frame.captionButtonWidth"), ncmetrics.iCaptionWidth );
    SetIntegerProperty( TEXT("win.frame.captionButtonHeight"), ncmetrics.iCaptionHeight );
    SetFontProperty( TEXT("win.frame.smallCaptionFont"), ncmetrics.lfSmCaptionFont );
    SetIntegerProperty( TEXT("win.frame.smallCaptionHeight"), ncmetrics.iSmCaptionHeight );
    SetIntegerProperty( TEXT("win.frame.smallCaptionButtonWidth"), ncmetrics.iSmCaptionWidth );
    SetIntegerProperty( TEXT("win.frame.smallCaptionButtonHeight"), ncmetrics.iSmCaptionHeight );
    SetIntegerProperty( TEXT("win.frame.sizingBorderWidth"), ncmetrics.iBorderWidth );
    
    // menu properties
    SetFontProperty( TEXT("win.menu.font"), ncmetrics.lfMenuFont );
    SetIntegerProperty( TEXT("win.menu.height"), ncmetrics.iMenuHeight );
    SetIntegerProperty( TEXT("win.menu.buttonWidth"), ncmetrics.iMenuWidth );
    
    // scrollbar properties
    SetIntegerProperty( TEXT("win.scrollbar.width"), ncmetrics.iScrollWidth );
    SetIntegerProperty( TEXT("win.scrollbar.height"), ncmetrics.iScrollHeight );
    
    // status bar and tooltip properties
    SetFontProperty( TEXT("win.status.font"), ncmetrics.lfStatusFont );
    SetFontProperty( TEXT("win.tooltip.font"), ncmetrics.lfStatusFont );

    // message box properties
    SetFontProperty( TEXT("win.messagebox.font"), ncmetrics.lfMessageFont );
}

void AwtDesktopProperties::GetIconParameters() {
    //
    // icon properties
    //
    ICONMETRICS	iconmetrics;

    iconmetrics.cbSize = sizeof(iconmetrics);
    VERIFY( SystemParametersInfo(SPI_GETICONMETRICS, iconmetrics.cbSize, &iconmetrics, FALSE) );

    SetIntegerProperty(TEXT("win.icon.hspacing"), iconmetrics.iHorzSpacing);
    SetIntegerProperty(TEXT("win.icon.vspacing"), iconmetrics.iVertSpacing);
    SetBooleanProperty(TEXT("win.icon.titleWrappingOn"), iconmetrics.iTitleWrap != 0);
    SetFontProperty(TEXT("win.icon.font"), iconmetrics.lfFont);
}

void AwtDesktopProperties::GetColorParameters() {

    if (IS_WIN98 || IS_WIN2000) {
        SetColorProperty(TEXT("win.frame.activeCaptionGradientColor"), 
                              GetSysColor(COLOR_GRADIENTACTIVECAPTION));
        SetColorProperty(TEXT("win.frame.inactiveCaptionGradientColor"), 
                              GetSysColor(COLOR_GRADIENTINACTIVECAPTION));
        SetColorProperty(TEXT("win.item.hotTrackedColor"), 
                              GetSysColor(COLOR_HOTLIGHT));
    }    
    SetColorProperty(TEXT("win.3d.darkShadowColor"), GetSysColor(COLOR_3DDKSHADOW));
    SetColorProperty(TEXT("win.3d.backgroundColor"), GetSysColor(COLOR_3DFACE));
    SetColorProperty(TEXT("win.3d.highlightColor"), GetSysColor(COLOR_3DHIGHLIGHT));
    SetColorProperty(TEXT("win.3d.lightColor"), GetSysColor(COLOR_3DLIGHT));
    SetColorProperty(TEXT("win.3d.shadowColor"), GetSysColor(COLOR_3DSHADOW));
    SetColorProperty(TEXT("win.button.textColor"), GetSysColor(COLOR_BTNTEXT));
    SetColorProperty(TEXT("win.desktop.backgroundColor"), GetSysColor(COLOR_DESKTOP));
    SetColorProperty(TEXT("win.frame.activeCaptionColor"), GetSysColor(COLOR_ACTIVECAPTION));
    SetColorProperty(TEXT("win.frame.activeBorderColor"), GetSysColor(COLOR_ACTIVEBORDER));
    
    // ?? ?? ??
    SetColorProperty(TEXT("win.frame.color"), GetSysColor(COLOR_WINDOWFRAME)); // ?? WHAT THE HECK DOES THIS MEAN ??
    // ?? ?? ??
    
    SetColorProperty(TEXT("win.frame.backgroundColor"), GetSysColor(COLOR_WINDOW));
    SetColorProperty(TEXT("win.frame.captionTextColor"), GetSysColor(COLOR_CAPTIONTEXT));
    SetColorProperty(TEXT("win.frame.inactiveBorderColor"), GetSysColor(COLOR_INACTIVEBORDER));
    SetColorProperty(TEXT("win.frame.inactiveCaptionColor"), GetSysColor(COLOR_INACTIVECAPTION));
    SetColorProperty(TEXT("win.frame.inactiveCaptionTextColor"), GetSysColor(COLOR_INACTIVECAPTIONTEXT));
    SetColorProperty(TEXT("win.frame.textColor"), GetSysColor(COLOR_WINDOWTEXT));
    SetColorProperty(TEXT("win.item.highlightColor"), GetSysColor(COLOR_HIGHLIGHT));
    SetColorProperty(TEXT("win.item.highlightTextColor"), GetSysColor(COLOR_HIGHLIGHTTEXT));
    SetColorProperty(TEXT("win.mdi.backgroundColor"), GetSysColor(COLOR_APPWORKSPACE));
    SetColorProperty(TEXT("win.menu.backgroundColor"), GetSysColor(COLOR_MENU));
    SetColorProperty(TEXT("win.menu.textColor"), GetSysColor(COLOR_MENUTEXT));
    // COLOR_MENUBAR is only defined on WindowsXP. Our binaries are
    // built on NT, hence the below ifdef.
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR 30
#endif
    SetColorProperty(TEXT("win.menubar.backgroundColor"),
				GetSysColor(IS_WINXP ? COLOR_MENUBAR : COLOR_MENU));
    SetColorProperty(TEXT("win.scrollbar.backgroundColor"), GetSysColor(COLOR_SCROLLBAR));
    SetColorProperty(TEXT("win.text.grayedTextColor"), GetSysColor(COLOR_GRAYTEXT));
    SetColorProperty(TEXT("win.tooltip.backgroundColor"), GetSysColor(COLOR_INFOBK));
    SetColorProperty(TEXT("win.tooltip.textColor"), GetSysColor(COLOR_INFOTEXT));
}

void AwtDesktopProperties::GetOtherParameters() {
    // TODO BEGIN: On NT4, some setttings don't trigger WM_SETTINGCHANGE --
    // check whether this has been fixed on Windows 2000 and Windows 98
    // ECH 10/6/2000 seems to be fixed on NT4 SP5, but not on 98
    SetBooleanProperty(TEXT("win.frame.fullWindowDragsOn"), GetBooleanParameter(SPI_GETDRAGFULLWINDOWS));
    SetBooleanProperty(TEXT("win.text.fontSmoothingOn"), GetBooleanParameter(SPI_GETFONTSMOOTHING));
    // TODO END

    if (IS_WINXP) {
        SetIntegerProperty(TEXT("win.text.fontSmoothingType"), 
                           GetIntegerParameter(SPI_GETFONTSMOOTHINGTYPE));
        SetIntegerProperty(TEXT("win.text.fontSmoothingContrast"), 
                           GetIntegerParameter(SPI_GETFONTSMOOTHINGCONTRAST));
    }

    int cxdrag = GetSystemMetrics(SM_CXDRAG);
    int cydrag = GetSystemMetrics(SM_CYDRAG);
    SetIntegerProperty(TEXT("win.drag.width"), cxdrag);
    SetIntegerProperty(TEXT("win.drag.height"), cydrag);
    SetIntegerProperty(TEXT("DnD.gestureMotionThreshold"), max(cxdrag, cydrag)/2);
    SetIntegerProperty(TEXT("awt.mouse.numButtons"), GetSystemMetrics(SM_CMOUSEBUTTONS));
    SetIntegerProperty(TEXT("awt.multiClickInterval"), GetDoubleClickTime());

    // BEGIN cross-platform properties
    // Note that these are cross-platform properties, but are being stuck into
    // WDesktopProperties.  WToolkit.lazilyLoadDesktopProperty() can find them,
    // but if a Toolkit subclass uses the desktopProperties
    // member, these properties won't be there. -bchristi, echawkes
    // This property is called "win.frame.fullWindowDragsOn" above
    // This is one of the properties that don't trigger WM_SETTINGCHANGE
    SetBooleanProperty(TEXT("awt.dynamicLayoutSupported"), GetBooleanParameter(SPI_GETDRAGFULLWINDOWS));

    // 95 MouseWheel support
    // More or less copied from the MSH_MOUSEWHEEL MSDN entry
    if (IS_WIN95 && !IS_WIN98) {
        HWND hdlMSHWHEEL = NULL;
        UINT msgMSHWheelSupported = NULL;
        BOOL wheelSupported = FALSE;

        msgMSHWheelSupported = RegisterWindowMessage(MSH_WHEELSUPPORT);
        hdlMSHWHEEL = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
        if (hdlMSHWHEEL && msgMSHWheelSupported) {
            wheelSupported = (BOOL)::SendMessage(hdlMSHWHEEL,
                                                 msgMSHWheelSupported, 0, 0);
        }
        SetBooleanProperty(TEXT("awt.wheelMousePresent"), wheelSupported);
    }
    else {
        SetBooleanProperty(TEXT("awt.wheelMousePresent"),
                           ::GetSystemMetrics(SM_MOUSEWHEELPRESENT));
    }

    // END cross-platform properties

    if (IS_WIN98 || IS_WIN2000) {
      //DWORD	menuShowDelay;
        //SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &menuShowDelay, 0);
	// SetIntegerProperty(TEXT("win.menu.showDelay"), menuShowDelay);
        SetBooleanProperty(TEXT("win.frame.captionGradientsOn"), GetBooleanParameter(SPI_GETGRADIENTCAPTIONS));
        SetBooleanProperty(TEXT("win.item.hotTrackingOn"), GetBooleanParameter(SPI_GETHOTTRACKING));
    }

    if (IS_WIN2000) {
        SetBooleanProperty(TEXT("win.menu.keyboardCuesOn"), GetBooleanParameter(SPI_GETKEYBOARDCUES));
    }

    // High contrast accessibility property
    HIGHCONTRAST contrast;
    contrast.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST),
			     &contrast, 0) != 0 &&
	      (contrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON) {
      SetBooleanProperty(TEXT("win.highContrast.on"), TRUE);
    }
    else {
      SetBooleanProperty(TEXT("win.highContrast.on"), FALSE);
    }
    
    if (fn_SHGetSettings != NULL) {
        SHELLFLAGSTATE sfs;
        fn_SHGetSettings(&sfs, SSF_SHOWALLOBJECTS | SSF_SHOWATTRIBCOL);
        if (sfs.fShowAllObjects) {
            SetBooleanProperty(TEXT("awt.file.showHiddenFiles"), TRUE);
        }
        else {
            SetBooleanProperty(TEXT("awt.file.showHiddenFiles"), FALSE);
        }
        if (sfs.fShowAttribCol) {
            SetBooleanProperty(TEXT("awt.file..showAttribCol"), TRUE);
        }
        else {
            SetBooleanProperty(TEXT("awt.file.showAttribCol"), FALSE);
        }
    }
}

void AwtDesktopProperties::GetSoundEvents() {
    /////
    SetSoundProperty(TEXT("win.sound.default"), TEXT(".Default"));
    SetSoundProperty(TEXT("win.sound.close"), TEXT("Close"));
    SetSoundProperty(TEXT("win.sound.maximize"), TEXT("Maximize"));
    SetSoundProperty(TEXT("win.sound.minimize"), TEXT("Minimize"));
    SetSoundProperty(TEXT("win.sound.menuCommand"), TEXT("MenuCommand"));
    SetSoundProperty(TEXT("win.sound.menuPopup"), TEXT("MenuPopup"));
    SetSoundProperty(TEXT("win.sound.open"), TEXT("Open"));
    SetSoundProperty(TEXT("win.sound.restoreDown"), TEXT("RestoreDown"));
    SetSoundProperty(TEXT("win.sound.restoreUp"), TEXT("RestoreUp"));
    /////
    SetSoundProperty(TEXT("win.sound.asterisk"), TEXT("SystemAsterisk"));
    SetSoundProperty(TEXT("win.sound.exclamation"), TEXT("SystemExclamation"));
    SetSoundProperty(TEXT("win.sound.exit"), TEXT("SystemExit"));
    SetSoundProperty(TEXT("win.sound.hand"), TEXT("SystemHand"));
    SetSoundProperty(TEXT("win.sound.question"), TEXT("SystemQuestion"));
    SetSoundProperty(TEXT("win.sound.start"), TEXT("SystemStart"));
}

BOOL AwtDesktopProperties::GetBooleanParameter(UINT spi) {
    BOOL	flag;
    SystemParametersInfo(spi, 0, &flag, 0);
    DASSERT(flag == TRUE || flag == FALSE); // should be simple boolean value
    return flag;
}

UINT AwtDesktopProperties::GetIntegerParameter(UINT spi) {
    UINT retValue;
    SystemParametersInfo(spi, 0, &retValue, 0);
    return retValue;
}

void AwtDesktopProperties::SetStringProperty(LPCTSTR propName, LPTSTR value) {
    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setStringPropertyID,
			     key, JNU_NewStringPlatform(GetEnv(), value));
    GetEnv()->DeleteLocalRef(key);
}

void AwtDesktopProperties::SetIntegerProperty(LPCTSTR propName, int value) {
    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setIntegerPropertyID,
			     key, (jint)value);
    GetEnv()->DeleteLocalRef(key);
}

void AwtDesktopProperties::SetBooleanProperty(LPCTSTR propName, BOOL value) {
    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setBooleanPropertyID,
			     key, value ? JNI_TRUE : JNI_FALSE);
    GetEnv()->DeleteLocalRef(key);
}

void AwtDesktopProperties::SetColorProperty(LPCTSTR propName, DWORD value) {    
    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setColorPropertyID,
			     key, GetRValue(value), GetGValue(value),
			     GetBValue(value));
    GetEnv()->DeleteLocalRef(key);
}

void AwtDesktopProperties::SetFontProperty(HDC dc, int fontID,
                                           LPCTSTR propName) {
    HGDIOBJ font = GetStockObject(fontID);
    if (font != NULL && SelectObject(dc, font) != NULL) {
        int length = GetTextFace(dc, 0, NULL);

        if (length > 0) {
            LPTSTR face = new TCHAR[length];

            if (GetTextFace(dc, length, face) > 0) {
                TEXTMETRIC metrics;

                if (GetTextMetrics(dc, &metrics) > 0) {
                    jstring fontName = NULL;
                    if (!wcscmp(face, L"MS Shell Dlg")) {
                        // MS Shell Dlg is an indirect font name, find the
                        // real face name from the registry.
                        LPTSTR shellDialogFace = resolveShellDialogFont();
                        if (shellDialogFace != NULL) {
                            fontName = JNU_NewStringPlatform(GetEnv(),
                                                             shellDialogFace);
                            free(shellDialogFace);
                        }
                        else {
                            // Couldn't determine mapping for MS Shell Dlg,
                            // fall back to Microsoft Sans Serif
                            fontName = JNU_NewStringPlatform(GetEnv(),
                                                    L"Microsoft Sans Serif");
                        }
                    }
                    else {
                        fontName = JNU_NewStringPlatform(GetEnv(), face);
                    }
                    jint pointSize = metrics.tmHeight -
		                     metrics.tmInternalLeading;
                    jint style = java_awt_Font_PLAIN;

                    if (metrics.tmWeight >= FW_BOLD) {
                        style =  java_awt_Font_BOLD;
                    }
                    if (metrics.tmItalic ) {
                        style |= java_awt_Font_ITALIC;
                    }

                    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
                    GetEnv()->CallVoidMethod(self,
                              AwtDesktopProperties::setFontPropertyID,
                              key, fontName, style, pointSize);
                    GetEnv()->DeleteLocalRef(fontName);
                    GetEnv()->DeleteLocalRef(key);
                }
            }
            delete[] face;
        }
    }
}

void AwtDesktopProperties::SetFontProperty(LPCTSTR propName, const LOGFONT & font) {
    jstring fontName;
    jint pointSize;
    jint style;

    fontName = JNU_NewStringPlatform(GetEnv(), font.lfFaceName);

#if 0
    HDC		hdc;
    int		pixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    // convert font size specified in pixels to font size in points
    hdc = GetDC(NULL);
    pointSize = (-font.lfHeight)*72/pixelsPerInch;
    ReleaseDC(NULL, hdc);
#endif    
    // Java uses point sizes, but assumes 1 pixel = 1 point
    pointSize = -font.lfHeight;

    // convert Windows font style to Java style
    style = java_awt_Font_PLAIN;
    DTRACE_PRINTLN1("weight=%d", font.lfWeight);
    if ( font.lfWeight >= FW_BOLD ) {
	style =  java_awt_Font_BOLD;
    }
    if ( font.lfItalic ) {
	style |= java_awt_Font_ITALIC;
    }

    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
    GetEnv()->CallVoidMethod(self, AwtDesktopProperties::setFontPropertyID,
			     key, fontName, style, pointSize);

    GetEnv()->DeleteLocalRef(fontName);
    GetEnv()->DeleteLocalRef(key);
}

void AwtDesktopProperties::SetSoundProperty(LPCTSTR propName, LPCTSTR winEventName) {
    jstring key = JNU_NewStringPlatform(GetEnv(), propName);
    jstring event = JNU_NewStringPlatform(GetEnv(), winEventName);
    GetEnv()->CallVoidMethod(self,
			     AwtDesktopProperties::setSoundPropertyID,
			     key, event);

    GetEnv()->DeleteLocalRef(key);
    GetEnv()->DeleteLocalRef(event);
}

void AwtDesktopProperties::PlayWindowsSound(LPCTSTR event) {    
    // stop any currently playing sounds
    AwtWinMM::PlaySoundWrapper(NULL, NULL, SND_PURGE);
    // play the sound for the given event name
    AwtWinMM::PlaySoundWrapper(event, NULL, SND_ASYNC|SND_ALIAS|SND_NODEFAULT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static AwtDesktopProperties * GetCppThis(JNIEnv *env, jobject self) {
    jlong longProps = env->GetLongField(self, AwtDesktopProperties::pDataID);
    AwtDesktopProperties * props =
        (AwtDesktopProperties *)jlong_to_ptr(longProps);
    DASSERT( !IsBadReadPtr(props, sizeof(*props)) );
    return props;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtDesktopProperties::pDataID = env->GetFieldID(cls, "pData", "J");
    DASSERT(AwtDesktopProperties::pDataID != 0);

    AwtDesktopProperties::setBooleanPropertyID = env->GetMethodID(cls, "setBooleanProperty", "(Ljava/lang/String;Z)V");
    DASSERT(AwtDesktopProperties::setBooleanPropertyID != 0);
    
    AwtDesktopProperties::setIntegerPropertyID = env->GetMethodID(cls, "setIntegerProperty", "(Ljava/lang/String;I)V");
    DASSERT(AwtDesktopProperties::setIntegerPropertyID != 0);

    AwtDesktopProperties::setStringPropertyID = env->GetMethodID(cls, "setStringProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    DASSERT(AwtDesktopProperties::setStringPropertyID != 0);

    AwtDesktopProperties::setColorPropertyID = env->GetMethodID(cls, "setColorProperty", "(Ljava/lang/String;III)V");
    DASSERT(AwtDesktopProperties::setColorPropertyID != 0);

    AwtDesktopProperties::setFontPropertyID = env->GetMethodID(cls, "setFontProperty", "(Ljava/lang/String;Ljava/lang/String;II)V");
    DASSERT(AwtDesktopProperties::setFontPropertyID != 0);

    AwtDesktopProperties::setSoundPropertyID = env->GetMethodID(cls, "setSoundProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    DASSERT(AwtDesktopProperties::setSoundPropertyID != 0);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_init(JNIEnv *env, jobject self) {
    TRY;

    // Open shell32.dll, get the symbol for SHGetSettings
    libShell32 = LoadLibrary(TEXT("shell32.dll"));
    if (libShell32 == NULL) {
        fn_SHGetSettings = NULL;
    }
    else {
        fn_SHGetSettings = (SHGetSettingsType)GetProcAddress(
                libShell32, "SHGetSettings");
    }

    new AwtDesktopProperties(self);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_finalize(JNIEnv *env, jobject self) {
    TRY_NO_VERIFY;

    AwtDesktopProperties *props = GetCppThis(env, self);
    DASSERT(props != NULL);

    env->SetLongField(self, AwtDesktopProperties::pDataID,
		      ptr_to_jlong(NULL));
    delete props;

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_getWindowsParameters(JNIEnv *env, jobject self) {
    TRY;

    GetCppThis(env, self)->GetWindowsParameters();

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WDesktopProperties_playWindowsSound(JNIEnv *env, jobject self, jstring event) {
    TRY;

    LPCTSTR winEventName;
    winEventName = JNU_GetStringPlatformChars(env, event, NULL);
    if ( winEventName == NULL ) {
	return;
    }
    GetCppThis(env, self)->PlayWindowsSound(winEventName);
    JNU_ReleaseStringPlatformChars(env, event, winEventName);

    CATCH_BAD_ALLOC;
}

