/*
 * @(#)ConverterHelp_zh_CN.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;
import sun.plugin.converter.resources.ConverterHelpTemplates;

/**
 * Simplified Chinese version of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ConverterHelp_zh_CN extends ListResourceBundle {

    private static String newline = System.getProperty("line.separator");
    private static String fileSeparator = System.getProperty("file.separator");
    private static String j2seVersion = System.getProperty("java.version");

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "conhelp.file", newline +
      "Java(TM) \u63d2\u4ef6 HTML \u8f6c\u6362\u5668\u81ea\u8ff0\u6587\u4ef6" + newline + newline +
      "\u7248\u672c:  " + j2seVersion + newline + newline + newline +
      "*****   \u4f7f\u7528\u6b64\u5de5\u5177\u8f6c\u6362\u6587\u4ef6\u524d\u8bf7\u5148\u5907\u4efd\u6240\u6709\u8fd9\u4e9b\u6587\u4ef6\u3002" + newline +
      "*****   \u53d6\u6d88\u8f6c\u6362\u4e0d\u4f1a\u6062\u590d\u66f4\u6539\u3002" + newline +
      "*****   \u5c0f\u7a0b\u5e8f\u6807\u8bb0\u5185\u7684\u6ce8\u91ca\u5c06\u88ab\u5ffd\u7565\u3002" + newline + newline + newline +
      "\u76ee\u5f55:" + newline +
      "   1.  \u65b0\u589e\u529f\u80fd" + newline +
      "   2.  \u9519\u8bef\u66f4\u6b63" + newline +
      "   3.  \u5173\u4e8e Java(TM) \u63d2\u4ef6 HTML \u8f6c\u6362\u5668" + newline +
      "   4.  \u8f6c\u6362\u8fc7\u7a0b" + newline +
      "   5.  \u4ece\u6587\u4ef6\u5939\u4e2d\u9009\u62e9\u8981\u8f6c\u6362\u7684\u6587\u4ef6" + newline +
      "   6.  \u9009\u62e9\u5907\u4efd\u6587\u4ef6\u5939" + newline +
      "   7.  \u751f\u6210\u65e5\u5fd7\u6587\u4ef6" + newline +
      "   8.  \u9009\u62e9\u8f6c\u6362\u6a21\u677f" + newline +
      "   9.  \u8f6c\u6362" + newline +
      "  10.  \u7ee7\u7eed\u8f6c\u6362\u6216\u9000\u51fa" + newline +
      "  11.  \u6709\u5173\u6a21\u677f\u7684\u8be6\u7ec6\u4fe1\u606f" + newline +
      "  12.  \u8fd0\u884c HTML \u8f6c\u6362\u5668 (Windows \u548c Solaris)" + newline + newline +
      "1)  \u65b0\u589e\u529f\u80fd:" + newline + newline +
      "    o \u66f4\u65b0\u4e86\u6269\u5c55\u6a21\u677f\uff0c\u652f\u6301 Netscape 6\u3002" + newline +
      "    o \u66f4\u65b0\u4e86\u6240\u6709\u6a21\u677f\uff0c\u652f\u6301 Java \u63d2\u4ef6\u4e2d\u65b0\u7684\u591a\u7248\u672c\u529f\u80fd\u3002" + newline +
      "    o \u4f7f\u7528 Swing 1.1\u589e\u5f3a\u4e86\u7528\u6237\u754c\u9762\uff0c\u652f\u6301\u56fd\u9645\u5316\u3002" + newline +
      "    o \u589e\u5f3a\u4e86\u9ad8\u7ea7\u9009\u9879\u5bf9\u8bdd\u6846\u7684\u529f\u80fd\uff0c\u652f\u6301\u65b0\u7684 SmartUpdate \u548c" + newline +
      "      MimeType \u6a21\u677f\u6807\u8bb0\u3002" + newline +
      "    o \u589e\u5f3a\u4e86 HTML \u8f6c\u6362\u5668\uff0c\u53ef\u4ee5\u4e0e Java \u63d2\u4ef6 1.1.x\u3001" + newline +
      "      Java \u63d2\u4ef6 1.2.x\u3001Java \u63d2\u4ef6 1.3.x\u3001Java \u63d2\u4ef6 1.4.x" + newline +
      "      \u548c Java \u63d2\u4ef6 1.5.x\u3002" + newline +
      "    o \u5728\u6240\u6709\u8f6c\u6362\u6a21\u677f\u4e2d\uff0c\u589e\u5f3a\u4e86\u5bf9 SmartUpdate \u548c MimeType \u7684" + newline +
      "      \u652f\u6301\u3002" + newline +
      "    o \u5728\u6240\u6709\u6a21\u677f\u7684 OBJECT/EMBED \u6807\u8bb0\u4e2d\u6dfb\u52a0\u4e86\u201cscriptable=false\u201d\u3002" + newline + newline +
      "     \u5f53 Java \u63d2\u4ef6\u672a\u7528\u4e8e\u811a\u672c\u7f16\u8f91\u65f6\uff0c\u6b64\u529f\u80fd" + newline +
      "    \u53ef\u4ee5\u7981\u6b62\u751f\u6210 typelib\u3002" + newline + newline + newline +
      "2)  \u66f4\u6b63\u7684\u9519\u8bef:" + newline + newline +
      "    o \u589e\u5f3a\u4e86\u627e\u4e0d\u5230\u5c5e\u6027\u6587\u4ef6\u65f6\u7684\u9519\u8bef\u5904\u7406\u80fd\u529b\u3002" + newline +
      "    o \u589e\u5f3a\u4e86 HTML \u8f6c\u6362\u529f\u80fd\uff0c\u4ee5\u4fbf\u4ea7\u751f\u7684 EMBED/OBJECT \u6807\u8bb0\u53ef\u4ee5" + newline +
      "      \u5728 JDK 1.2.x \u7684 AppletViewer \u4e2d\u4f7f\u7528\u3002" + newline +
      "    o \u6e05\u9664\u4e86\u4f7f\u7528 HTML \u8f6c\u6362\u5668 1.1.x \u6240\u5bfc\u81f4\u7684\u4e0d\u5fc5\u8981\u6587\u4ef6\u3002" + newline +
      "    o \u751f\u6210\u7684 EMBED/OBJECT \u5177\u6709 CODE\u3001CODEBASE \u7b49\u5c5e\u6027\u540d\u79f0\uff0c" + newline +
      "      \u800c\u4e0d\u662f JAVA_CODE\u3001JAVA_CODEBASE \u7b49\u5c5e\u6027\u540d\u79f0\u3002\u8fd9\u4f7f" + newline +
      "      \u751f\u6210\u7684\u9875\u9762\u53ef\u5728 JDK 1.2.x AppletViewer \u4e2d\u4f7f\u7528\u3002" + newline +
      "    o \u5982\u679c MAYSCRIPT \u5b58\u5728\u4e8e APPLET \u6807\u8bb0\u4e2d\uff0c" + newline +
      "      \u5219\u652f\u6301 MAYSCRIPT \u8f6c\u6362\u3002" + newline + newline +
      "3)  \u5173\u4e8e Java(TM) \u63d2\u4ef6 HTML \u8f6c\u6362\u5668:" + newline + newline +
      "        Java(TM) \u63d2\u4ef6 HTML \u8f6c\u6362\u5668\u662f\u4e00\u79cd\u5b9e\u7528\u7a0b\u5e8f\uff0c\u53ef\u7528\u4e8e\u5c06\u4efb\u4e00\u5305\u542b\u5c0f\u7a0b\u5e8f\u7684 HTML \u9875\u9762" + newline +
      "        \u8f6c\u6362\u4e3a\u4f7f\u7528 Java(TM)" + newline +
      "        \u63d2\u4ef6\u7684\u683c\u5f0f\u3002" + newline + newline +
      "4)  \u8f6c\u6362\u8fc7\u7a0b:" + newline + newline +
      "        Java(TM) \u63d2\u4ef6 HTML \u8f6c\u6362\u5668\u5c06\u6240\u6709\u5305\u542b\u5c0f\u7a0b\u5e8f\u7684\u6587\u4ef6\u8f6c\u6362\u4e3a" + newline +
      "        \u53ef\u4ee5\u4e0e Java(TM) \u63d2\u4ef6\u914d\u5408\u4f7f\u7528\u7684\u683c\u5f0f\u3002" + newline + newline +
      "        \u8f6c\u6362\u6bcf\u4e2a\u6587\u4ef6\u7684\u8fc7\u7a0b\u5982\u4e0b:" + newline +
      "        \u9996\u5148\uff0c\u5c06\u4e0d\u5c5e\u4e8e\u5c0f\u7a0b\u5e8f\u7684 HTML \u4ece\u6e90\u6587\u4ef6\u4f20\u9001\u5230" + newline +
      "        \u4e34\u65f6\u6587\u4ef6\u4e2d\u3002\u5f53\u9047\u5230 <APPLET \u6807\u8bb0\u65f6\uff0c\u8f6c\u6362\u5668" + newline +
      "        \u5c06\u5206\u6790\u5c0f\u7a0b\u5e8f\u76f4\u5230\u7b2c\u4e00\u4e2a </APPLET \u6807\u8bb0\u5904 (\u4e0d\u5305\u542b\u5728\u5f15\u53f7\u4e2d)\uff0c" + newline +
      "        \u5e76\u5c06\u5c0f\u7a0b\u5e8f\u6570\u636e\u5408\u5e76\u5230\u6a21\u677f\u3002(\u53c2\u89c1\u4e0b\u9762\u6709\u5173\u6a21\u677f\u7684\u8be6\u7ec6\u4fe1\u606f)\u3002" + newline +
      "        \u5982\u679c\u6b64\u8fc7\u7a0b\u987a\u5229\u5b8c\u6210\uff0c\u539f\u59cb html \u6587\u4ef6\u5c06\u88ab\u79fb\u5230" + newline +
      "        \u5907\u4efd\u6587\u4ef6\u5939\uff0c\u4e34\u65f6\u6587\u4ef6\u5c06\u88ab\u91cd\u547d\u540d\u4e3a" + newline +
      "        \u539f\u59cb\u6587\u4ef6\u7684\u540d\u79f0\u3002\u8fd9\u6837\uff0c\u539f\u59cb\u6587\u4ef6\u6c38\u8fdc\u90fd\u4e0d\u4f1a\u88ab\u4ece\u78c1\u76d8\u4e0a\u5220\u9664\u3002" + newline + newline +
      "        \u6ce8\u610f\uff0c\u8f6c\u6362\u5668\u5c06\u5728\u539f\u5904\u9ad8\u6548\u8f6c\u6362\u6587\u4ef6\u3002\u6240\u4ee5\uff0c" + newline +
      "        \u4e00\u65e6\u60a8\u8fd0\u884c\u4e86\u8f6c\u6362\u5668\uff0c\u6587\u4ef6\u5c06\u88ab\u8bbe\u7f6e\u4e3a\u4f7f\u7528 Java(TM) \u63d2\u4ef6\u3002" + newline +


      "5)  \u4ece\u6587\u4ef6\u5939\u4e2d\u9009\u62e9\u8981\u8f6c\u6362\u7684\u6587\u4ef6:" + newline + newline +
      "       \u8981\u8f6c\u6362\u6587\u4ef6\u5939\u4e2d\u7684\u6240\u6709\u6587\u4ef6\uff0c\u53ef\u4ee5\u952e\u5165\u6587\u4ef6\u5939\u7684\u8def\u5f84\uff0c" + newline +
      "       \u6216\u9009\u62e9\u201c\u6d4f\u89c8\u201d\u6309\u94ae\u4ece\u5bf9\u8bdd\u6846\u4e2d\u9009\u62e9\u6587\u4ef6\u5939\u3002" + newline  +
      "       \u4e00\u65e6\u9009\u62e9\u4e86\u8def\u5f84\uff0c\u5c31\u53ef\u4ee5\u5728" + newline +
      "       \u201c\u5339\u914d\u6587\u4ef6\u540d\u201d\u4e2d\u8f93\u5165\u4efb\u610f\u6570\u91cf\u7684\u6587\u4ef6\u6307\u5b9a\u7b26\u3002\u5404\u4e2a\u6307\u5b9a\u7b26\u4e4b\u95f4\u5fc5\u987b\u4ee5\u9017\u53f7\u5206\u9694\u3002\u53ef\u4ee5\u4f7f\u7528 * \u4f5c\u4e3a" + newline +
      "       \u901a\u914d\u7b26\u3002\u5982\u679c\u8f93\u5165\u7684\u6587\u4ef6\u540d\u4e2d\u5e26\u6709\u901a\u914d\u7b26\uff0c\u5219\u53ea\u8f6c\u6362\u5355\u4e2a" + newline +
      "       \u6587\u4ef6\u3002\u6700\u540e\uff0c\u5982\u679c\u8981\u8f6c\u6362\u5d4c\u5957\u6587\u4ef6\u5939\u4e2d\u6240\u6709\u4e0e\u6587\u4ef6\u540d\u5339\u914d\u7684\u6587\u4ef6\uff0c" + newline +
      "       \u8bf7\u9009\u62e9\u201c\u5305\u542b\u5b50\u6587\u4ef6\u5939\u201d\u590d\u9009\u6846\u3002" + newline + newline +
      "6)  \u9009\u62e9\u5907\u4efd\u6587\u4ef6\u5939:" + newline +

      "       \u9ed8\u8ba4\u7684\u5907\u4efd\u6587\u4ef6\u5939\u8def\u5f84\u4e3a\u5728\u6e90\u8def\u5f84\u540d\u540e\u52a0\u4e0a" + newline +
      "       \u201c_BAK\u201d\uff0c\u5373\uff0c\u5982\u679c\u6e90\u8def\u5f84\u4e3a c:/html/applet.html (\u8f6c\u6362\u4e00\u4e2a\u6587\u4ef6)\uff0c" + newline +
      "       \u5219\u5907\u4efd\u8def\u5f84\u5e94\u4e3a c:/html_BAK\u3002\u5982\u679c\u6e90\u8def\u5f84" + newline +
      "       \u4e3a c:/html (\u8f6c\u6362\u8def\u5f84\u4e2d\u7684\u6240\u6709\u6587\u4ef6)\uff0c\u5219\u5907\u4efd\u8def\u5f84\u5e94\u4e3a" + newline +
      "       c:/html_BAK\u3002\u53ef\u4ee5\u901a\u8fc7\u5728\u201c\u5c06\u6587\u4ef6\u5907\u4efd\u5230\u6587\u4ef6\u5939:\u201d\u65c1\u7684\u5b57\u6bb5\u4e2d\u952e\u5165\u8def\u5f84\u6216" + newline +
      "       \u6d4f\u89c8\u5230\u6587\u4ef6\u5939\u6765\u66f4\u6539\u5907\u4efd\u8def\u5f84\u3002" + newline + newline +

      "       Unix (Solaris):" + newline +
      "       \u9ed8\u8ba4\u7684\u5907\u4efd\u6587\u4ef6\u5939\u8def\u5f84\u4e3a\u5728\u6e90\u8def\u5f84\u540d\u540e\u52a0\u4e0a" + newline +
      "       \u201c_BAK\u201d\uff0c\u5373\uff0c\u5982\u679c\u6e90\u8def\u5f84\u4e3a /home/user1/html/applet.html (\u8f6c\u6362\u4e00\u4e2a\u6587\u4ef6)\uff0c" + newline +
      "       \u5219\u5907\u4efd\u8def\u5f84\u5e94\u4e3a /home/user1/html_BAK\u3002\u5982\u679c\u6e90\u8def\u5f84" + newline +
      "       \u4e3a /home/user1/html (\u8f6c\u6362\u8def\u5f84\u4e2d\u7684\u6240\u6709\u6587\u4ef6)\uff0c\u5219\u5907\u4efd\u8def\u5f84\u5e94\u4e3a" + newline +
      "       /home/user1/html_BAK\u3002\u53ef\u4ee5\u901a\u8fc7\u5728\u201c\u5c06\u6587\u4ef6\u5907\u4efd\u5230\u6587\u4ef6\u5939:\u201d\u65c1\u7684\u5b57\u6bb5\u4e2d\u952e\u5165\u8def\u5f84\u6216" + newline +
      "       \u6d4f\u89c8\u5230\u6587\u4ef6\u5939\u6765\u66f4\u6539\u5907\u4efd\u8def\u5f84\u3002" + newline + newline +
      "7)  \u751f\u6210\u65e5\u5fd7\u6587\u4ef6:" + newline + newline +
      "       \u5982\u679c\u8981\u751f\u6210\u65e5\u5fd7\u6587\u4ef6\uff0c\u8bf7\u9009\u62e9\u590d\u9009\u6846" + newline +
      "       \u201c\u751f\u6210\u65e5\u5fd7\u6587\u4ef6\u201d\u3002\u53ef\u4ee5\u8f93\u5165\u8def\u5f84\u548c\u6587\u4ef6\u540d\u6216\u8fdb\u884c\u6d4f\u89c8" + newline +
      "       \u4ee5\u9009\u62e9\u6587\u4ef6\u5939\uff0c\u7136\u540e\u952e\u5165\u6587\u4ef6\u540d\u5e76\u9009\u62e9\u6253\u5f00\u6587\u4ef6\u3002" + newline +
      "       \u65e5\u5fd7\u6587\u4ef6\u4e2d\u5305\u542b\u4e0e\u8f6c\u6362\u8fc7\u7a0b\u6709\u5173\u7684" + newline +
      "       \u57fa\u672c\u4fe1\u606f\u3002" + newline + newline +
      "8)  \u9009\u62e9\u8f6c\u6362\u6a21\u677f:" + newline + newline +
      "       \u5982\u679c\u6ca1\u6709\u9009\u62e9\u6a21\u677f\uff0c\u5c06\u4f7f\u7528\u9ed8\u8ba4\u6a21\u677f\u3002\u6b64\u6a21\u677f\u5c06" + newline +
      "       \u751f\u6210\u7ecf\u8fc7\u8f6c\u6362\u7684 html \u6587\u4ef6\uff0c\u8fd9\u6837\u8be5\u6587\u4ef6\u4fbf\u53ef\u914d\u5408 IE \u548c Netscape \u4f7f\u7528\u3002" + newline  +
      "       \u5982\u679c\u8981\u4f7f\u7528\u5176\u5b83\u6a21\u677f\uff0c\u60a8\u53ef\u4ee5\u4ece\u4e3b\u5c4f\u5e55\u7684" + newline +
      "       \u83dc\u5355\u4e2d\u9009\u62e9\u3002\u5982\u679c\u9009\u62e9\u4e86\u201c\u5176\u5b83\u201d\uff0c\u5219\u5141\u8bb8\u60a8\u9009\u62e9\u4e00\u4e2a\u6587\u4ef6" + newline +
      "       \u4f5c\u4e3a\u6a21\u677f\u3002" + newline +
      "       \u5982\u679c\u9009\u62e9\u4e86\u4e00\u4e2a\u6587\u4ef6\uff0c\u8bf7\u786e\u4fdd\u5176\u4e3a\u6a21\u677f\u6587\u4ef6\u3002" + newline + newline +
      "9)  \u8f6c\u6362:" + newline + newline +
      "       \u5355\u51fb\u201c\u8f6c\u6362...\u201d\u6309\u94ae\u4ee5\u5f00\u59cb\u8f6c\u6362\u8fc7\u7a0b\u3002\u8fdb\u7a0b\u5bf9\u8bdd\u6846\u5c06" + newline +
      "       \u663e\u793a\u6b63\u5728\u5904\u7406\u7684\u6587\u4ef6\u3001\u5904\u7406\u6587\u4ef6\u6570\u3001" + newline +
      "       \u627e\u5230\u7684\u5c0f\u7a0b\u5e8f\u6570\u548c\u53d1\u73b0\u7684\u9519\u8bef\u6570\u3002" + newline + newline +
      "10) \u7ee7\u7eed\u8f6c\u6362\u6216\u9000\u51fa:" + newline + newline +
      "       \u8f6c\u6362\u7ed3\u675f\u540e\uff0c\u8fdb\u7a0b\u5bf9\u8bdd\u6846\u4e2d\u7684\u6309\u94ae\u5c06\u7531" + newline +
      "       \u201c\u53d6\u6d88\u201d\u66f4\u6539\u4e3a\u201c\u5b8c\u6210\u201d\u3002\u60a8\u53ef\u4ee5\u9009\u62e9\u201c\u5b8c\u6210\u201d\u4ee5\u5173\u95ed\u5bf9\u8bdd\u6846\u3002" + newline  +
      "       \u8fd9\u65f6\uff0c\u53ef\u4ee5\u9009\u62e9\u201c\u9000\u51fa\u201d\u4ee5\u5173\u95ed Java(TM) \u63d2\u4ef6 HTML \u8f6c\u6362\u5668\uff0c" + newline +
      "       \u4e5f\u53ef\u4ee5\u9009\u62e9\u53e6\u4e00\u7ec4\u8981\u8f6c\u6362\u7684\u6587\u4ef6\u5e76\u518d\u6b21\u9009\u62e9\u201c\u8f6c\u6362...\u201d\u3002" + newline + newline +
      "11)  \u6709\u5173\u6a21\u677f\u7684\u8be6\u7ec6\u4fe1\u606f:" + newline + newline +
      "       \u6a21\u677f\u6587\u4ef6\u662f\u8f6c\u6362\u5c0f\u7a0b\u5e8f\u7684\u57fa\u7840\u3002\u5b83\u53ea\u662f\u4e00\u4e2a\u6587\u672c\u6587\u4ef6\uff0c" + newline +
      "       \u5305\u542b\u8868\u793a\u90e8\u5206\u539f\u59cb\u5c0f\u7a0b\u5e8f\u7684\u6807\u8bb0\u3002" + newline +
      "       \u901a\u8fc7\u5728\u6a21\u677f\u6587\u4ef6\u4e2d\u6dfb\u52a0/\u5220\u9664/\u79fb\u52a8\u6807\u8bb0\uff0c\u60a8\u53ef\u4ee5\u6539\u53d8" + newline +
      "       \u5df2\u8f6c\u6362\u6587\u4ef6\u7684\u8f93\u51fa\u3002" + newline + newline +
      "       \u652f\u6301\u7684\u6807\u8bb0:" + newline + newline +
      "        $OriginalApplet$    \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u539f\u59cb\u5c0f\u7a0b\u5e8f\u7684\u5b8c\u6574\u6587\u672c\u66ff\u6362\u3002" + newline + newline +
      "        $AppletAttributes$    \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u6240\u6709\u5c0f\u7a0b\u5e8f\u5c5e\u6027(\u4ee3\u7801\u3001\u4ee3\u7801\u5e93\u3001\u5bbd\u5ea6\u3001\u9ad8\u5ea6\u7b49) \u66ff\u6362\u3002" + newline + newline +
      "        $ObjectAttributes$    \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u5bf9\u8c61\u6807\u8bb0\u8981\u6c42\u7684\u6240\u6709\u5c5e\u6027\u66ff\u6362\u3002" + newline + newline +
      "        $EmbedAttributes$    \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u5d4c\u5165\u6807\u8bb0\u8981\u6c42\u7684\u6240\u6709\u5c5e\u6027\u66ff\u6362\u3002" + newline + newline +
      "        $AppletParams$    \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u6240\u6709\u5c0f\u7a0b\u5e8f\u7684 <param ...> \u6807\u8bb0\u66ff\u6362" + newline + newline +
      "        $ObjectParams$    \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u5bf9\u8c61\u6807\u8bb0\u8981\u6c42\u7684\u6240\u6709 <param...> \u6807\u8bb0\u66ff\u6362\u3002" + newline + newline +
      "        $EmbedParams$     \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u683c\u5f0f\u4e3a NAME=VALUE \u7684\u5d4c\u5165\u6807\u8bb0\u8981\u6c42\u7684\u6240\u6709 <param...> \u6807\u8bb0\u66ff\u6362" + newline + newline +
      "        $AlternateHTML$  \u6b64\u6807\u8bb0\u5c06\u88ab" + newline +
      "        \u539f\u59cb\u5c0f\u7a0b\u5e8f\u7684\u4e0d\u652f\u6301\u5c0f\u7a0b\u5e8f\u533a\u57df\u4e2d\u7684\u6587\u672c\u66ff\u6362" + newline + newline +
      "        $CabFileLocation$   \u8fd9\u662f\u5e94\u8be5\u5728\u6bcf\u4e2a\u9488\u5bf9 IE \u7684\u6a21\u677f\u4e2d\u4f7f\u7528\u7684" + newline +
      "        cab \u6587\u4ef6\u7684 URL\u3002" + newline + newline +
      "        $NSFileLocation$   \u8fd9\u662f\u5728\u6bcf\u4e2a\u9488\u5bf9 Netscape \u7684\u6a21\u677f\u4e2d\u4f7f\u7528\u7684" + newline +
      "        Netscape \u63d2\u4ef6\u7684 URL\u3002" + newline + newline +
      "        $SmartUpdate$   \u8fd9\u662f\u5728\u6bcf\u4e2a\u9488\u5bf9 Netscape Navigator 4.0 \u6216\u66f4\u9ad8\u7248\u672c\u7684\u6a21\u677f\u4e2d\u4f7f\u7528\u7684" + newline +
      "        Netscape SmartUpdate \u7684 URL\u3002" + newline + newline +
      "        $MimeType$     \u8fd9\u662f Java \u5bf9\u8c61\u7684 MIME \u7c7b\u578b" + newline + newline +
      "      default.tpl (\u8f6c\u6362\u5668\u7684\u9ed8\u8ba4\u6a21\u677f) -- \u8f6c\u6362\u7684\u9875\u9762\u53ef\u7528\u4e8e" + newline +
      "      Windows \u4e2d\u7684 IE \u548c Navigator\uff0c\u4ee5\u8c03\u7528 Java(TM) \u63d2\u4ef6\u3002" + newline +
      "      \u6b64\u6a21\u677f\u4e5f\u53ef\u7528\u4e8e Unix (Solaris) \u4e2d\u7684 Netscape\u3002" + newline + newline +
      ConverterHelpTemplates.DEFAULT_TPL + newline + newline +
      "      ieonly.tpl -- \u8f6c\u6362\u7684\u9875\u9762\u53ea\u80fd\u7528\u4e8e\u5728 Windows \u4e2d\u7684 IE \u4e2d\u8c03\u7528 Java(TM) " + newline +
      "      \u63d2\u4ef6\u3002" + newline + newline +
      ConverterHelpTemplates.IEONLY_TPL + newline + newline +
      "      nsonly.tpl -- \u8f6c\u6362\u7684\u9875\u9762\u53ef\u7528\u4e8e\u5728 Windows \u548c Solaris \u4e2d\u7684 Navigator \u4e2d\u8c03\u7528 Java(TM)" + newline +
      "      \u63d2\u4ef6\u3002" + newline + newline +
      ConverterHelpTemplates.NSONLY_TPL + newline + newline +
      "      extend.tpl -- \u8f6c\u6362\u7684\u9875\u9762\u53ef\u7528\u4e8e\u4efb\u4f55\u5e73\u53f0\u548c\u4efb\u4f55\u6d4f\u89c8\u5668\u3002" + newline +
      "      \u5982\u679c\u6d4f\u89c8\u5668\u4e3a Windows \u4e2d\u7684 IE \u6216 Navigator (\u6216 Solaris \u4e2d\u7684 Navigator)\uff0cJava(TM)" + newline +
      "      \u63d2\u4ef6\u5c06\u88ab\u8c03\u7528\u3002\u5426\u5219\uff0c\u5c06\u4f7f\u7528\u6d4f\u89c8\u5668\u7684\u9ed8\u8ba4 JVM\u3002" + newline + newline +
      ConverterHelpTemplates.EXTEND_TPL + newline + newline +
      "12)  \u8fd0\u884c HTML \u8f6c\u6362\u5668:" + newline + newline +
      "      \u8fd0\u884c HTML \u8f6c\u6362\u5668\u7684 GUI \u7248\u672c" + newline + newline +
      "      HTML \u8f6c\u6362\u5668\u5305\u542b\u5728 JDK \u4e2d\uff0c\u800c\u4e0d\u662f JRE \u4e2d\u3002\u8981\u8fd0\u884c\u8f6c\u6362\u5668\uff0c\u8bf7\u627e\u5230" + newline +
      "      JDK \u5b89\u88c5\u76ee\u5f55\u7684 lib \u5b50\u76ee\u5f55\u3002\u4f8b\u5982\uff0c" + newline +
      "      \u5982\u679c JDK \u5b89\u88c5\u5728 Windows \u4e2d C:\\jdk" + j2seVersion + " \u76ee\u5f55\uff0c\u5219\u4f7f\u7528 cd \u547d\u4ee4\u8fdb\u5165" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\" + newline + newline +
      "      \u8f6c\u6362\u5668 (htmlconverter.jar) \u5305\u542b\u5728\u6b64\u76ee\u5f55\u4e0b\u3002" + newline + newline +
      "      \u8981\u542f\u52a8\u8f6c\u6362\u5668\uff0c\u8bf7\u952e\u5165:" + newline + newline +
      "            C:\\jdk" + j2seVersion + "\\lib\\..\\bin\\java -jar htmlconverter.jar -gui" + newline + newline +
      "      \u5728 UNIX/Linux \u4e2d\u542f\u52a8\u8f6c\u6362\u5668\u7684\u547d\u4ee4\u4e0e\u4ee5\u4e0a\u547d\u4ee4\u7c7b\u4f3c\u3002" + newline +
      "      \u4ee5\u4e0b\u662f\u542f\u52a8\u8f6c\u6362\u5668\u7684\u4e00\u4e9b\u5176\u5b83\u65b9\u6cd5" + newline + newline +
      "      \u5728 Windows \u4e2d" + newline +
      "      \u4f7f\u7528\u8d44\u6e90\u7ba1\u7406\u5668\u542f\u52a8\u8f6c\u6362\u5668\u3002" + newline +
      "      \u4f7f\u7528\u8d44\u6e90\u7ba1\u7406\u5668\u6d4f\u89c8\u5230\u4ee5\u4e0b\u76ee\u5f55\u3002" + newline + newline +
      "      C:\\jdk" + j2seVersion + "\\bin" + newline + newline +
      "      \u53cc\u51fb Html \u8f6c\u6362\u5668\u5e94\u7528\u7a0b\u5e8f\u3002" + newline + newline +
      "      Unix/Linux" + newline + newline +
      "      \u8fd0\u884c\u4ee5\u4e0b\u547d\u4ee4" + newline + newline +
      "      cd /jdk" + j2seVersion + "/bin" + newline +
      "      ./HtmlConverter -gui" + newline + newline +
      "      \u4ece\u547d\u4ee4\u884c\u8fd0\u884c\u8f6c\u6362\u5668:" + newline + newline +
      "      \u683c\u5f0f:" + newline + newline +
      "      java -jar htmlconverter.jar [-options1 value1 [-option2 value2" + newline +
      "      [...]]] [-simulate] [filespecs]" + newline + newline +
      "      filespecs:\u7528\u7a7a\u683c\u5206\u9694\u7684\u6587\u4ef6\u89c4\u683c\u5217\u8868\uff0c* \u4e3a\u901a\u914d\u7b26\u3002 " + newline +
      "      (*.html *.htm)" + newline + newline +
      "      \u9009\u9879:" + newline + newline +
      "       source:\u6587\u4ef6\u8def\u5f84\u3002(Windows \u4e2d\u4e3a c:\\htmldocs" + newline +
      "                  Unix \u4e2d\u4e3a /home/user1/htmldocs) \u9ed8\u8ba4\u503c:<userdir>" + newline +
      "                  \u5982\u679c\u662f\u76f8\u5bf9\u8def\u5f84\uff0c\u5219\u5047\u8bbe\u5176\u76f8\u5bf9\u4e8e" + newline +
      "                  HTML \u8f6c\u6362\u5668\u7684\u542f\u52a8\u76ee\u5f55\u3002" + newline + newline +
      "       backup:\u5199\u5165\u5907\u4efd\u6587\u4ef6\u7684\u8def\u5f84\u3002\u9ed8\u8ba4\u503c:" + newline +
      "                  <userdir>/<source>_bak" + newline +
      "                  \u5982\u679c\u662f\u76f8\u5bf9\u8def\u5f84\uff0c\u5219\u5047\u8bbe\u5176\u76f8\u5bf9\u4e8e" + newline +
      "                  HTML \u8f6c\u6362\u5668\u7684\u542f\u52a8\u76ee\u5f55\u3002" + newline + newline +
      "       subdirs:\u5b50\u76ee\u5f55\u4e2d\u7684\u6587\u4ef6\u5e94\u88ab\u5904\u7406\u3002 " + newline +
      "                  \u9ed8\u8ba4\u503c:FALSE" + newline + newline +
      "       template:\u6a21\u677f\u6587\u4ef6\u540d\u3002\u9ed8\u8ba4\u503c:default.tpl -  " + newline +
      "                  \u4ec5\u5bf9\u4e8e Windows \u548c Solaris \u4e3a\u6807\u51c6 (IE \u548c Navigator)\u3002\u5982\u679c\u4e0d\u80fd\u786e\u5b9a\uff0c\u8bf7\u4f7f\u7528\u9ed8\u8ba4\u503c\u3002" + newline + newline +
      "       log:\u5199\u65e5\u5fd7\u7684\u8def\u5f84\u548c\u6587\u4ef6\u540d\u3002(\u9ed8\u8ba4\u503c <userdir>/convert.log)" + newline + newline +
      "       progress:\u663e\u793a\u8f6c\u6362\u65f6\u6807\u51c6\u8f93\u51fa\u8fdb\u5ea6\u3002 " + newline +
      "                  \u9ed8\u8ba4\u503c:false" + newline + newline +
      "       simulate:\u663e\u793a\u5bf9\u8f6c\u6362\u7684\u8bf4\u660e\u800c\u4e0d\u8fdb\u884c\u8f6c\u6362\u3002" + newline +
      "                  \u5982\u679c\u4e0d\u80fd\u786e\u5b9a\u662f\u5426\u8fdb\u884c\u8f6c\u6362\uff0c\u8bf7\u4f7f\u7528\u6b64\u9009\u9879\u3002" + newline +
      "                  \u8fd9\u5c06\u7ed9\u51fa\u6709\u5173\u8f6c\u6362\u7684" + newline +
      "                  \u8be6\u7ec6\u4fe1\u606f\u5217\u8868\u3002" + newline + newline +
      "      \u5982\u679c\u53ea\u6307\u5b9a\u201cjava -jar htmlconverter.jar -gui\u201d(\u4ec5\u6307\u5b9a -gui \u9009\u9879\uff0c\u672a\u6307\u5b9a filespecs \u9009\u9879)\uff0c" + newline +
      "      \u5c06\u542f\u52a8\u8f6c\u6362\u5668\u7684 GUI \u7248\u672c\u3002" + newline  +
      "      \u5426\u5219\uff0cGUI \u5c06\u4e0d\u8d77\u4f5c\u7528\u3002" + newline + newline +
      "      \u6709\u5173\u8be6\u7ec6\u4fe1\u606f\uff0c\u8bf7\u8bbf\u95ee\u4ee5\u4e0b URL:" + newline + newline +
      "      http://java.sun.com/j2se/" + 
      (j2seVersion.indexOf('_') != -1 ? j2seVersion.substring(0,j2seVersion.indexOf('_')) : j2seVersion) +
      "/docs/guide/plugin/developer_guide/html_converter_more.html."}
};
}




