/*
 * @(#)UserDialogHelper.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;


/** 
 * This class is a repository for all the dialog strings. Currently, 
 * it hard-codes the US-English strings, but it should eventually be linked 
 * to the Netscape XP strings library. Note that there are no public methods
 * here, although maybe all these methods should be public. 
 *
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class UserDialogHelper
{
    public UserDialogHelper()
    {
    }

    public final static String targetRiskLow()
    {
	return "Risk Low";
    }

    public final static String targetRiskColorLow()
    {
	return "#0000FF";
    }

    public final static String targetRiskMedium()
    {
	return "Risk Medium";
    }

    public final static String targetRiskColorMedium()
    {
	return "#00FF00";
    }

    public final static String targetRiskHigh()
    {
	return "Risk High";
    }

    public final static String targetRiskColorHigh()
    {
	return "#FF0000";
    }
}
