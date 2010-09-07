/*
 * @(#)WindowsJavaTrayIcon.java	1.8 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.image.*;
import java.net.URL;
import java.security.*;
import com.sun.deploy.resources.*;
import com.sun.deploy.config.Config;

class WindowsJavaTrayIcon extends JavaTrayIcon {
    static {
        Config.getInstance().loadDeployNativeLib();
    }

    private static final int IDM_SHOW_CONSOLE      = 3001;
    private static final int IDM_HIDE_CONSOLE      = 3002;
    private static final int IDM_ABOUT_PLUGIN      = 3003;
    private static final int IDM_EXIT              = 3004;
    private static final int IDI_JAVACONSOLE       = 3005;
    private static final int IDM_DISABLE           = 3006;
    private static final int IDM_SHOW_CONTROLPANEL = 3007;
    private static final int IDM_GOTO_JAVA         = 3008;

    // Some constants needed to interact with the Windows APIs
    private static final int MF_STRING             = 0x0;
    private static final int MF_SEPARATOR          = 0x0800;
    private static final int NIM_ADD               = 0x00;
    private static final int NIM_MODIFY            = 0x01;
    private static final int NIM_DELETE            = 0x02;
    private static final int NIF_MESSAGE           = 0x01;
    private static final int NIF_ICON              = 0x02;
    private static final int NIF_TIP               = 0x04;
    private static final int NIF_INFO              = 0x10;
    private static final int NIN_BALLOONUSERCLICK  = 0x0405;
    private static final int SW_HIDE               = 0;
    private static final int WM_DESTROY            = 0x02;
    private static final int WM_COMMAND            = 0x0111;
    private static final int WM_LBUTTONDBLCLK      = 0x0203;
    private static final int WM_RBUTTONUP          = 0x0205;

    private static final String JAVA_VERSION = (String) AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                return System.getProperty("java.version");
            }
        });
    private static final String ICON_NAME = "com/sun/deploy/resources/image/java-tray-icon.gif";
    private static final String JAVA_HOME_LINK = "http://www.java.com";
    private static final String REG_USER_JRE_PATH = "SOFTWARE\\JavaSoft\\Java Runtime Environment\\" + JAVA_VERSION;
    private static final String REG_USER_FIRSTRUN_KEY = "BalloonShown";

    private long hWnd;
    private long hIcon;
    private long hMenu;

    private boolean deletedIcon;

    private Image imageBeingProcessed;

    WindowsJavaTrayIcon(JavaTrayIconController controller) {
        super(controller);
    }

    protected boolean isEnabled() {
        return isEnabled0(JAVA_VERSION);
    }

    protected void installImpl() {
        final Object lock = new Object();
        synchronized(lock) {
            new Thread(new Runnable() {
                    public void run() {
                        String className = "JavaConsole " + JAVA_VERSION +
                            " " + getCurrentProcessId();
                        if (registerClass(className)) {
                            hWnd = createWindow(className);
                        }
                        if (hWnd != 0) {
                            showWindow(hWnd, SW_HIDE);
                            setUserData(hWnd);
                        }
                        synchronized(lock) {
                            lock.notifyAll();
                        }
                        mainLoop();
                    }
                }, "Windows Tray Icon Thread").start();
            try {
                lock.wait();
            } catch (InterruptedException e) {
            }
        }
        // Should be ready to go
        initIcon();
        createMenu();
        if (!hasBalloonTooltipShown()) {
            setBalloonTooltipShown(true);
            showBalloonTooltip();
        }
        Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    deleteIcon();
                }
            });
    }

    protected void notifyConsoleClosedImpl() {
        changeMenuToOpenConsole();
    }

    private void initIcon() {
        ClassLoader loader = getClass().getClassLoader();
        // Load the icon's resource
        URL resource = (loader == null ? ClassLoader.getSystemResource(ICON_NAME) : loader.getResource(ICON_NAME)); 
        Image image = null;
        try {
            image = ImageLoader.getInstance().loadImage(resource);
        } catch (java.io.IOException ioe) {
        }
        createIcon(image);
        addIcon();
    }

    private void createMenu() {
        hMenu = createPopupMenu();
        appendMenu(hMenu, MF_STRING, IDM_ABOUT_PLUGIN, ResourceManager.getMessage("systray.about.java"));
        appendMenu(hMenu, MF_STRING, IDM_GOTO_JAVA, ResourceManager.getMessage("systray.goto.java"));
        appendMenu(hMenu, MF_SEPARATOR, 0, null);
        appendMenu(hMenu, MF_STRING, IDM_SHOW_CONTROLPANEL, ResourceManager.getMessage("systray.open.controlpanel"));
        // Provide both show and hide options all the time for best utility
        Object[] args = new Object[] { JAVA_VERSION };
        
        // Get the console state from the Config file.
        if (Config.getProperty(Config.CONSOLE_MODE_KEY).
                equalsIgnoreCase(Config.CONSOLE_MODE_SHOW)){
            // If console is going to be open, add "Hide console" menu item
            appendMenu(hMenu, MF_STRING, IDM_HIDE_CONSOLE,
                       ResourceManager.getFormattedMessage(
                           "systray.hide.console", args));
        } else {
            //  If console is not going to be open, add "Show console" menu item
            appendMenu(hMenu, MF_STRING, IDM_SHOW_CONSOLE,
                       ResourceManager.getFormattedMessage(
                           "systray.open.console", args));
        }
        
        appendMenu(hMenu, MF_SEPARATOR, 0, null);
        appendMenu(hMenu, MF_STRING, IDM_DISABLE, ResourceManager.getMessage("systray.disable"));
        setMenuDefaultItem(hMenu, 0, true);
    }

    private long wndProc(long hWnd, int msg, long wParam, long lParam) {
        switch (msg) {
            case IDI_JAVACONSOLE:
                int mouseMsg = (int) lParam;
                switch (mouseMsg) {
                    // Show Floating popup menu
                    case WM_RBUTTONUP: {
                        showPopupMenu(hWnd, hMenu);
                        return 0;
                    }

                    // The user double clicked the plugin icon, open the java console
                    case WM_LBUTTONDBLCLK: {
                        UIFactory.showAboutJavaDialog();
                        return 0;
                    }

                    case NIN_BALLOONUSERCLICK: {
                        if (isBalloonClickInBounds()) {
			    Config.getInstance().showDocument(JAVA_HOME_LINK);
                        }
                        break;
                    }

                    default:
                        break;
                }
                break;

            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case IDM_SHOW_CONSOLE: {
                        controller.showJavaConsole(true);
                        // Change menu item for console from
                        // "Open console" to "Hide console"
                        modifyMenu( hMenu, IDM_SHOW_CONSOLE, IDM_HIDE_CONSOLE, 
                                ResourceManager.getFormattedMessage(
                                "systray.hide.console", 
                                new Object[] { JAVA_VERSION }));
                        return 0;
                    }

                    case IDM_GOTO_JAVA: {
			Config.getInstance().showDocument(JAVA_HOME_LINK);
                        return 0;
                    }

                    case IDM_HIDE_CONSOLE: {
                        controller.showJavaConsole(false);
                        changeMenuToOpenConsole();
                        return 0;
                    }

                    case IDM_ABOUT_PLUGIN: {
                        UIFactory.showAboutJavaDialog();
                        return 0;
                    }

                    case IDM_DISABLE: {
                        showSysTray(JAVA_VERSION, false);
                        deleteIcon();
                        return 0;
                    }

                    case IDM_SHOW_CONTROLPANEL: {
                        openControlPanel();
                        return 0;
                    }

                    default:
                        break;
                }

            case WM_DESTROY: {
                postQuitMessage(0);
                return 0;
            }
        }

        return defWindowProc(hWnd, msg, wParam, lParam);
    }

    //----------------------------------------------------------------------
    // Tray icon creation code borrowed from JDK 6 WTrayIconPeer
    //

    private static final int TRAY_ICON_WIDTH = 16;
    private static final int TRAY_ICON_HEIGHT = 16;
    private static final int TRAY_ICON_MASK_SIZE = (TRAY_ICON_WIDTH * TRAY_ICON_HEIGHT) / 8;

    private void createIcon(Image image) {
        imageBeingProcessed = image;
        BufferedImage bufImage = new BufferedImage(TRAY_ICON_WIDTH, TRAY_ICON_HEIGHT,
                                                   BufferedImage.TYPE_INT_ARGB);
        Graphics2D gr = bufImage.createGraphics();
        if (gr != null) {
            try {
                gr.setPaintMode();

                if (gr.drawImage(image, 0, 0, TRAY_ICON_WIDTH, TRAY_ICON_HEIGHT, null)) {
                    createNativeIcon(bufImage);
                }
            } finally {
                gr.dispose();                
            }
        }
    }

    void createNativeIcon(BufferedImage bimage) {
        Raster raster = bimage.getRaster();
        byte[] andMask = new byte[TRAY_ICON_MASK_SIZE];
        int  pixels[] = ((DataBufferInt)raster.getDataBuffer()).getData();
        int npixels = pixels.length;

        for (int i = 0; i < npixels; i++) {
            int ibyte = i / 8;
            int omask = 1 << (7 - (i % 8));

            if ((pixels[i] & 0xff000000) == 0) {
                // Transparent bit
                if (ibyte < andMask.length) {
                    andMask[ibyte] |= omask;
                }
            }
        }

        int ficW = getScanlineStride(raster.getSampleModel());
        if (ficW < 0) {
            ficW = raster.getWidth();
        }

        hIcon = createNativeIcon(((DataBufferInt)bimage.getRaster().getDataBuffer()).getData(),
                                 andMask, ficW, raster.getWidth(), raster.getHeight());
    }

    private static int getScanlineStride(SampleModel sm) {
        if (sm instanceof SinglePixelPackedSampleModel) {
            return ((SinglePixelPackedSampleModel)sm).getScanlineStride();
        } else if (sm instanceof MultiPixelPackedSampleModel) {
            return ((MultiPixelPackedSampleModel)sm).getScanlineStride();
        } else if (sm instanceof ComponentSampleModel) {
            return ((ComponentSampleModel)sm).getScanlineStride();
        }
        return -1;
    }

    //----------------------------------------------------------------------

    private void changeMenuToOpenConsole() {
        // Change menu item for console from
        // "Hide console" to "Open console"
        modifyMenu(hMenu, IDM_HIDE_CONSOLE, IDM_SHOW_CONSOLE,
                   ResourceManager.getFormattedMessage(
                       "systray.open.console",
                       new Object[] { JAVA_VERSION }));
    }

    //----------------------------------------------------------------------

    private void addIcon() {
        notifyShell(NIM_ADD,
                    hWnd,
                    0,
                    NIF_MESSAGE | NIF_ICON | NIF_TIP,
                    IDI_JAVACONSOLE,
                    hIcon,
                    ResourceManager.getMessage("systray.tooltip"),
                    null,
                    null,
                    0);
    }

    private void showBalloonTooltip() {
        notifyShell(NIM_MODIFY,
                    hWnd,
                    0,
                    NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO,
                    IDI_JAVACONSOLE,
                    hIcon,
                    ResourceManager.getMessage("systray.tooltip"),
                    ResourceManager.getMessage("systray.balloon.title"),
                    ResourceManager.getMessage("systray.balloon.tooltip"),
                    15000); // 15 seconds
    }

    private synchronized void deleteIcon() {
        if (!deletedIcon) {
            deletedIcon = true;
            notifyShell(NIM_DELETE,
                        hWnd,
                        0,
                        NIF_MESSAGE | NIF_ICON,
                        IDI_JAVACONSOLE,
                        hIcon,
                        null,
                        null,
                        null,
                        0);
            destroyIcon(hIcon);
            hIcon = 0;
        }
    }

    private static boolean hasBalloonTooltipShown() {
        return hasBalloonTooltipShown0(REG_USER_JRE_PATH,
                                       REG_USER_FIRSTRUN_KEY);
    }
    private static void setBalloonTooltipShown(boolean val) {
        setBalloonTooltipShown0(REG_USER_JRE_PATH,
                                REG_USER_FIRSTRUN_KEY,
                                val);
    }

    private static native boolean isEnabled0(String javaVersion);
    private static native long getCurrentProcessId();
    private static native boolean registerClass(String className);
    private static native long createWindow(String className);
    private static native void showWindow(long hWnd, int command);
    private        native void setUserData(long hWnd);
    private static native void mainLoop();
    private static native boolean hasBalloonTooltipShown0(String regKeyPath,
                                                          String firstRunKey);
    private static native void setBalloonTooltipShown0(String regKeyPath,
                                                       String firstRunKey,
                                                       boolean val);
    private static native long createNativeIcon(int[] intRasterData,
                                                byte[] andMask,
                                                int nScanStride,
                                                int width, int height);
    private static native void destroyIcon(long icon);
    private static native long createPopupMenu();
    private static native void appendMenu(long hMenu, int flags, long newItemID, String newItem);
    private static native void modifyMenu(long hMenu, int flags, long newItemID, String newItem);
    private static native boolean setMenuDefaultItem(long hMenu, int item, boolean byPos);
    private static native void showPopupMenu(long hWnd, long hMenu);
    private static native boolean isBalloonClickInBounds();
    private short LOWORD(long param) {
        return ((short) (param & 0x0000FFFF));
    }
    private static native void showSysTray(String javaVersion, boolean onOrOff);
    private static native void notifyShell(long message,
                                           long hWnd,
                                           int  id,
                                           int  flags,
                                           int  callbackMessage,
                                           long hIcon,
                                           String tooltip,
                                           String balloonTitle,
                                           String balloonText,
                                           int  timeout);
    private static native void openControlPanel();
    private static native void postQuitMessage(int exitCode);
    private static native long defWindowProc(long hWnd, int msg, long wParam, long lParam);
}
