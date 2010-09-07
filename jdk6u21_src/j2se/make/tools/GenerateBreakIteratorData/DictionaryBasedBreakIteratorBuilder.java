/*
 * @(#)DictionaryBasedBreakIteratorBuilder.java	1.1 03/09/09
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.util.Hashtable;
import java.util.Vector;

/**
 * The Builder class for DictionaryBasedBreakIterator inherits almost all of
 * its functionality from RuleBasedBreakIteratorBuilder, but extends it with
 * extra logic to handle the "<dictionary>" token.
 */
class DictionaryBasedBreakIteratorBuilder extends RuleBasedBreakIteratorBuilder {

    /**
     * A list of flags indicating which character categories are contained in
     * the dictionary file (this is used to determine which ranges of characters
     * to apply the dictionary to)
     */
    private boolean[] categoryFlags;

    /**
     * A CharSet that contains all the characters represented in the dictionary
     */
    private CharSet dictionaryChars = new CharSet();
    private String dictionaryExpression = "";

    public DictionaryBasedBreakIteratorBuilder(String description) {
        super(description);
    }

    /**
     * We override handleSpecialSubstitution() to add logic to handle
     * the <dictionary> tag.  If we see a substitution named "<dictionary>",
     * parse the substitution expression and store the result in
     * dictionaryChars.
     */
    protected void handleSpecialSubstitution(String replace, String replaceWith,
                                             int startPos, String description) {
        super.handleSpecialSubstitution(replace, replaceWith, startPos, description);

        if (replace.equals("<dictionary>")) {
            if (replaceWith.charAt(0) == '(') {
                error("Dictionary group can't be enclosed in (", startPos, description);
            }
            dictionaryExpression = replaceWith;
            dictionaryChars = CharSet.parseString(replaceWith);
        }
    }

    /**
     * The other half of the logic to handle the dictionary characters happens
     * here. After the inherited builder has derived the real character
     * categories, we set up the categoryFlags array in the iterator. This array
     * contains "true" for every character category that includes a dictionary
     * character.
     */
    protected void buildCharCategories(Vector tempRuleList) {
        super.buildCharCategories(tempRuleList);

        categoryFlags = new boolean[categories.size()];
        for (int i = 0; i < categories.size(); i++) {
            CharSet cs = (CharSet)categories.elementAt(i);
            if (!(cs.intersection(dictionaryChars).empty())) {
                categoryFlags[i] = true;
            }
        }
    }

    // This function is actually called by
    // RuleBasedBreakIteratorBuilder.buildCharCategories(), which is called by
    // the function above. This gives us a way to create a separate character
    // category for the dictionary characters even when
    // RuleBasedBreakIteratorBuilder isn't making a distinction.
    protected void mungeExpressionList(Hashtable expressions) {
        expressions.put(dictionaryExpression, dictionaryChars);
    }

    void makeFile(String filename) {
        super.setAdditionalData(super.toByteArray(categoryFlags));
	super.makeFile(filename);
    }
}
