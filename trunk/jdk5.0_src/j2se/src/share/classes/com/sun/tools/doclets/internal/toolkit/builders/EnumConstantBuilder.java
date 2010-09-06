/*
 * @(#)EnumConstantBuilder.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.builders;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;
import java.util.*;
import java.lang.reflect.*;

/**
 * Builds documentation for a enum constants.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */
public class EnumConstantBuilder extends AbstractMemberBuilder {

	/**
	 * The class whose enum constants are being documented.
	 */
	private ClassDoc classDoc;

	/**
	 * The visible enum constantss for the given class.
	 */
	private VisibleMemberMap visibleMemberMap;

	/**
	 * The writer to output the enum constants documentation.
	 */
	private EnumConstantWriter writer;

	/**
	 * The list of enum constants being documented.
	 */
	private List enumConstants;

	/**
	 * The index of the current enum constant that is being documented at this point 
	 * in time.
	 */
	private int currentEnumConstantsIndex;

	/**
	 * Construct a new EnumConstantsBuilder.
	 *
	 * @param configuration the current configuration of the
	 *                      doclet.
	 */
	private EnumConstantBuilder(Configuration configuration) {
		super(configuration);
	}

	/**
	 * Construct a new EnumConstantsBuilder.
	 *
	 * @param configuration the current configuration of the doclet.
	 * @param classDoc the class whoses members are being documented.
	 * @param writer the doclet specific writer.
	 */
	public static EnumConstantBuilder getInstance(
		Configuration configuration,
		ClassDoc classDoc,
		EnumConstantWriter writer) {
		EnumConstantBuilder builder = new EnumConstantBuilder(configuration);
		builder.classDoc = classDoc;
		builder.writer = writer;
		builder.visibleMemberMap =
			new VisibleMemberMap(
				classDoc,
				VisibleMemberMap.ENUM_CONSTANTS,
				configuration.nodeprecated);
		builder.enumConstants =
			new ArrayList(builder.visibleMemberMap.getMembersFor(classDoc));
		if (configuration.getMemberComparator() != null) {
			Collections.sort(
				builder.enumConstants,
				configuration.getMemberComparator());
		}
		return builder;
	}

	/**
	 * {@inheritDoc}
	 */
	public String getName() {
		return "EnumConstantDetails";
	}

	/**
	 * {@inheritDoc}
	 */
	public void invokeMethod(
		String methodName,
		Class[] paramClasses,
		Object[] params)
		throws Exception {
		if (DEBUG) {
			configuration.root.printError(
				"DEBUG: " + this.getClass().getName() + "." + methodName);
		}
		Method method = this.getClass().getMethod(methodName, paramClasses);
		method.invoke(this, params);
	}

	/**
	 * Returns a list of enum constants that will be documented for the given class.  
	 * This information can be used for doclet specific documentation 
	 * generation.
	 *
	 * @param classDoc the {@link ClassDoc} we want to check.
	 * @return a list of enum constants that will be documented.
	 */
	public List members(ClassDoc classDoc) {
		return visibleMemberMap.getMembersFor(classDoc);
	}

	/**
	 * Returns the visible member map for the enum constants of this class.
	 *
	 * @return the visible member map for the enum constants of this class.
	 */
	public VisibleMemberMap getVisibleMemberMap() {
		return visibleMemberMap;
	}

	/**
	 * summaryOrder.size()
	 */
	public boolean hasMembersToDocument() {
		return enumConstants.size() > 0;
	}

	/**
	 * Build the enum constant documentation.
	 *
	 * @param elements the XML elements that specify how to construct this 
	 *                documentation.
	 */
	public void buildEnumConstant(List elements) {
		if (writer == null) {
			return;
		}
		for (currentEnumConstantsIndex = 0;
			currentEnumConstantsIndex < enumConstants.size();
			currentEnumConstantsIndex++) {
			build(elements);
		}
	}

	/**
	 * Build the overall header.
	 */
	public void buildHeader() {
		writer.writeHeader(
			classDoc,
			configuration.getText("doclet.Enum_Constant_Detail"));
	}

	/**
	 * Build the header for the individual enum constants.
	 */
	public void buildEnumConstantHeader() {
		writer.writeEnumConstantHeader(
			(FieldDoc) enumConstants.get(currentEnumConstantsIndex),
			currentEnumConstantsIndex == 0);
	}

	/**
	 * Build the signature.
	 */
	public void buildSignature() {
		writer.writeSignature(
			(FieldDoc) enumConstants.get(currentEnumConstantsIndex));
	}

	/**
	 * Build the deprecation information.
	 */
	public void buildDeprecationInfo() {
		writer.writeDeprecated(
			(FieldDoc) enumConstants.get(currentEnumConstantsIndex));
	}

	/**
	 * Build the comments for the enum constant.  Do nothing if 
	 * {@link Configuration#nocomment} is set to true.
	 */
	public void buildEnumConstantComments() {
		if (!configuration.nocomment) {
			writer.writeComments(
				(FieldDoc) enumConstants.get(currentEnumConstantsIndex));
		}
	}

	/**
	 * Build the tag information.
	 */
	public void buildTagInfo() {
		writer.writeTags(
			(FieldDoc) enumConstants.get(currentEnumConstantsIndex));
	}

	/**
	 * Build the footer for the individual enum constants.
	 */
	public void buildEnumConstantFooter() {
		writer.writeEnumConstantFooter();
	}

	/**
	 * Build the overall footer.
	 */
	public void buildFooter() {
		writer.writeFooter(classDoc);
	}

	/**
	 * Return the enum constant writer for this builder.
	 *
	 * @return the enum constant writer for this builder.
	 */
	public EnumConstantWriter getWriter() {
		return writer;
	}
}
