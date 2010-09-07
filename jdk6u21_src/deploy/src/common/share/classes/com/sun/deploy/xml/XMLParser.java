/*
 * @(#)XMLParser.java	1.27 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.xml;

import java.io.IOException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


public class XMLParser {
    public XMLNode _root;
    public String _source;
    public String _current;
    public int    _tokenType;
    public String _tokenData;
    public static final int TOKEN_EOF = 1;			//  null
    public static final int TOKEN_END_TAG = 2;			//  </
    public static final int TOKEN_BEGIN_TAG = 3;		//  <
    public static final int TOKEN_CLOSE_TAG = 4;		//  >
    public static final int TOKEN_EMPTY_CLOSE_TAG = 5;		//  />
    public static final int TOKEN_PCDATA = 6;			//  qwerty

    private BadTokenException _savedBte;
    /**
     * This array contains the predefined XML character entity references that
     * all XML parsers are required to recognize.
     */
    public static final GeneralEntity [] REQUIRED_CHARACTER_REFERENCES =
        {
            new GeneralEntity("quot", "\""),
            new GeneralEntity("amp",  "&"),
            new GeneralEntity("apos", "'"),
            new GeneralEntity("lt",   "<"),
            new GeneralEntity("gt",   ">")
        };

    /**
     * Construct an <code>XMLParser</code>.
     *
     * @param source  - the source text to parse.
     */
    public XMLParser(String source) {
	Trace.println("new XMLParser with source:", TraceLevel.TEMP);
	Trace.println(source, TraceLevel.TEMP);

        _source    = source;
        _current   = source;
        _root      = null;
        _tokenData = null;

        _savedBte = null;
    }

    public BadTokenException getSavedException() {
        return _savedBte;
    }

    public XMLNode parse() throws BadTokenException {
        try {
            nextToken(_current);
            _root = parseXMLElement();
	} catch (NullPointerException npe) {

	    Trace.println("NULL Pointer Exception: "+npe, TraceLevel.TEMP);
            throw npe;

        } catch (BadTokenException bte) {

	    Trace.println("JNLP Parse Exception: "+bte, TraceLevel.TEMP);
            throw bte;
	}

        if (Trace.isTraceLevelEnabled(TraceLevel.TEMP)) {
            Trace.println("\n\nreturning ROOT as follows:\n"+
        		_root, TraceLevel.TEMP);
        }
        return _root;
    }


    private void nextToken(String source) throws BadTokenException {

        _current = skipFilling(source);

        if (_current == null) {				// TOKEN_EOF
            _tokenType = TOKEN_EOF;
        } else if (_current.startsWith(CDStart)) {      // TOKEN_PCDATA
            _tokenType = TOKEN_PCDATA;
	    _current = skipPCData(source, '<');         // skip till next "<"
        } else if (_current.startsWith("</")) {		// TOKEN_END_TAG
            _tokenType = TOKEN_END_TAG;
            _current = skipXMLName(skipForward(_current,2,0));
        } else if (_current.startsWith("<")) {		// TOKEN_BEGIN_TAG
            _tokenType = TOKEN_BEGIN_TAG;
            _current = skipXMLName(skipForward(_current,1,0));
        } else if (_current.startsWith(">")) { 		// TOKEN_CLOSE_TAG
            _tokenType = TOKEN_CLOSE_TAG;
            _current = skipForward(_current,1,0);
        } else if (_current.startsWith("/>")) {		// TOKEN_EMPTY_CLOSE_TAG
            _tokenType = TOKEN_EMPTY_CLOSE_TAG;
            _current = skipForward(_current,2,0);
        } else {					// TOKEN_PCDATA
            _tokenType = TOKEN_PCDATA;
	    _current = skipPCData(source, '<');         // skip till next "<"
	}
    }

    private static final String CDStart = "<![CDATA[";
    private static final String CDEnd   = "]]>";


    /**
     * This method skips over any parsed character data (PCDATA).  It returns a
     * string that is a substring of the complete XML source, and begins with
     * the first character that is not part of the PCDATA.
     * <br />
     * PCDATA is the text within an element or an attribute value.  As it is
     * parsed, general entities are replaced by their replacement text. However,
     * any unparsed character data (CDATA) is left alone.
     * <br />
     * After this method completes, the parsed character data is stored in the
     * member variable <code>_tokenData</code>.
     * <br />
     * Note: This parser currently only recognizes the required character
     *       reference entities.  All other entities will be left as is.
     *
     * @param source       unparsed XML source that is assumed to start with
     *                     PCDATA.
     * @param delimitChar  character that marks the end of the current PCDATA
     *                     and the start of the next markup element.
     *
     * @return The remaining, unparsed, XML source that follows the PCDATA text.
     *
     * @throws BadTokenException  if the end of the parsed character data isn't
     *                            found, or if an incomplete CDATA element is
     *                            embedded inside the PCDATA.
     */
    private String skipPCData(String source, char delimitChar) throws BadTokenException
    {
        int    index  = source.indexOf(delimitChar);
        String retval = null;

        if (index >= 0)
        {
            // look for PCDATA
            int pcdi = source.indexOf(CDStart);
            if ((pcdi >= 0) && (pcdi <= index)) {
                // This CDATA also contains PCDATA.  The XML parser needs to
                // keep the text inside the PCDATA element as-is.  In other
                // words, it shouldn't do any entity replacement, or complain
                // if there are any characters that aren't normally allowed in
                // a CDATA element.
                String pre    = parseCharacterReferences(source.substring(0, pcdi));
                String remain = source.substring(pcdi + CDStart.length());
                int    end    = remain.indexOf(CDEnd);
                if (end >= 0) {
                    retval     = skipPCData(remain.substring(end + CDEnd.length()),
                                            delimitChar);
                    _tokenData = pre + remain.substring(0, end) + _tokenData;
                }
                else {
                    // Failed to find the end of the PCDATA element.  The stream
                    // is corrupt or the JNLP isn't valid XML.  Set _current to
                    // a point as close to the the problem as possible.
                    _current = remain;
                    throw (new BadTokenException("Found the start of a PCDATA element with no end marker.",
                                                 _source, getLineNumber()));
                }
            }
            else {
                // Everything up to the delimiter is CDATA, which needs to be
                // checked for general entities.  Any entities found should be
                // substituted with the entity replacement text.  Currently,
                // this parser only supports character reference entities.
                //
                // Everything after the delimiter is unparsed XML.
                _tokenData = parseCharacterReferences(source.substring(0, index));
                retval     = source.substring(index);
            }
        }
        else if (source.trim().length() != 0) {
            // Failed to find the end of the CDATA element.  The stream is
            // corrupt or the JNLP isn't valid XML.  Set _current to a point
            // as close to the the problem as possible.
            _current = source;
            throw (new BadTokenException("Failed to find the '" + delimitChar +
                                         "' charater that marks the end of a CDATA element.",
                                         _source, getLineNumber()));
        }

        return (retval);
    }

    private XMLNode parseXMLElement() throws BadTokenException {
        XMLNode self, child, sibling;
        XMLAttribute attribute, nextAttribute, firstAttribute;
        String name;
        int openTagLineNumber;

        if (_tokenType == TOKEN_BEGIN_TAG) {
            name = _tokenData;
            openTagLineNumber = getLineNumber();

            // Parse attributes. This section eats all input until
            // an EOF, a > or a />
            firstAttribute = parseXMLAttribute(_current);
            attribute = firstAttribute;
            while (attribute != null) {
                nextAttribute = parseXMLAttribute(_current);
		attribute.setNext(nextAttribute);
		attribute = nextAttribute;
            }

            // Create node for new element tag
            self =  new XMLNode(name, firstAttribute);


            // This will eihter be a TOKEN_EOF, TOKEN_CLOSE_TAG, or a
            // TOKEN_EMPTY_CLOSE_TAG
            nextToken(_current);
            if ((_tokenType != TOKEN_EMPTY_CLOSE_TAG) &&
                (_tokenType != TOKEN_CLOSE_TAG) &&
                (_tokenType != TOKEN_EOF)) {
                throw new BadTokenException(_source, getLineNumber());
            }

            if (_tokenType == TOKEN_EMPTY_CLOSE_TAG) {
                nextToken(_current);
                // We are done with the sublevel - fall through to
                // continue parsing tags at the same level */
            } else if (_tokenType == TOKEN_CLOSE_TAG) {
                nextToken(_current);

                // Parse until end tag if found
                child = parseXMLElement();
                if (child != null) {
                    self.setNested(child);
                    child.setParent(self);
                }

                if (_tokenType == TOKEN_END_TAG) {
                    String closeTokenName = _tokenData;
                    if (!name.equals(closeTokenName)) {
                        // set only the first exception in the bteThreadLocal
                        if (_savedBte == null) {
                            _savedBte = new BadTokenException("WARNING: <" + name + "> tag is not closed correctly",
                                    _source, openTagLineNumber);
                        }

                        Trace.println("<" + name + "> tag at line number " + openTagLineNumber +
                                " is not closed correctly", TraceLevel.TEMP);
                    }
                    // Find closing bracket '>' for end tag
                    do {
                        nextToken(_current);
                    } while ((_tokenType != TOKEN_EOF) &&
                            (_tokenType != TOKEN_CLOSE_TAG));
                    nextToken(_current);
                }
            }
            // Continue parsing rest on same level
            if (_tokenType != TOKEN_EOF) {
                // Parse rest of stream at same level
                sibling = parseXMLElement();
                self.setNext(sibling);
            }
            return self;

        } else if (_tokenType == TOKEN_PCDATA) {
            // Create node for pcdata
            self = new XMLNode(_tokenData);
            nextToken(_current);
            return self;
        }

	return null;

    }

    private XMLAttribute parseXMLAttribute(String source)
                         throws BadTokenException {
        if (source == null) {
            return null;
        }
        _current = skipFilling(source);

        if ((_current == null) || _current.startsWith(">") ||
            _current.startsWith("/>")) {
            return null;
        }
        // extract name
        _current = skipAttributeName(_current);
        String name = _tokenData;
        _current = skipFilling(_current);
        if (!_current.startsWith("=")) {
            // This is really an error. We ignore this, and just try
            // to parse an attribute out of the rest of the string
            if (source.equals(_current)) {
                // avoid infinate loop - move foward 1 ...
                _current = skipForward(_current, 1, 0);
            }
            return parseXMLAttribute(_current);
        }
        _current = skipForward(_current, 1, 0);
        _current = skipWhitespace(_current);

        String value;
        if ((_current.startsWith("\"")) || (_current.startsWith("\'"))) {
	    char quote = _current.charAt(0);
            _current = skipForward(_current, 1, 0);  // past quote
	    _current = skipPCData(_current, quote);  // past PCData
	    value = _tokenData;
            _current = skipForward(_current, 1, 0);  // past endquote
        } else {
            _current = skipNonSpace(_current);
            value = _tokenData;
        }
	if (value != null) {
	    value = value.trim();
	}
        return new XMLAttribute(name, value);
    }

    /**
     * Parses a string, replacing any recognized character reference entities
     * with the proper substitution values.
     * <br />
     * Currently, this parser only recognizes the 5 character reference entities
     * that all XML parsers are required to recognize (<code>&quot;</code>,
     * <code>&amp;</code>, <code>&apos;</code>, <code>&lt;</code>,
     * <code>&gt;</code>), plus character references to characters in the unicode
     * sequence in either decimal or hexadecimal format.
     * <br />
     * If a source string contains an unrecognized entity, then it is silently
     * ingnored, and left unchanged in the result.
     *
     * @param source  the source string to parse.
     *
     * @return The source string with all recognized character reference
     *         entities replaced with the appropriate replacement character.
     */
    private String parseCharacterReferences(String source)
    {
        String result = source;

        int start = source.indexOf("&");
        if (start >= 0) {
            String pre    = source.substring(0, start);
            String subst  = "&";    // be forgiving
            String remain = source.substring(start + 1);
            int    end    = source.indexOf(";", start);

            if (end > start) {
                // the character reference contains at least one character
                boolean known = false;

                subst  = source.substring(start + 1, end);
                remain = source.substring(end + 1);

                if (subst.startsWith("#")) {
                    try {
                        int     radix   = 10;
                        int     index   = 1;
                        char [] unicode = { '\0' };

                        if (subst.startsWith("#x"))
                        {
                            // this is a hexadecimal unicode value
                            radix = 16;
                            index = 2;
                        }
                        // no else required; this is a decimal unicode value

                        unicode[0] = (char) Integer.parseInt(subst.substring(index), radix);
                        subst      = new String(unicode);
                        known      = true;

                    }
                    catch (NumberFormatException nfe) {
                        // Don't need to do anything here.  Since this is not a
                        // known character reference entity, it will be preserved
                        // as is, and logged below.
                    }
                }
                else {
                    for (int i = 0; i < REQUIRED_CHARACTER_REFERENCES.length; i++) {
                        if (REQUIRED_CHARACTER_REFERENCES[i].equals(subst)) {
                            subst = REQUIRED_CHARACTER_REFERENCES[i].getValue();
                            known = true;
                            break;
                        }
                        // no else required; keep searching
                    }
                }

                if (known == false) {
                    subst = "&" + subst + ";";
                    Trace.println("Unrecognized character entity reference: " + subst,
                                  TraceLevel.BASIC);
                }
                // no else required; was a known entity
            }
            // no else required; this is an invalid character reference, but
            // don't let that stop further parsing

            result = pre + subst + parseCharacterReferences(remain);
        }
        // no else required; no character references

        return (result);
    }

    private String skipForward(String source, int index, int length) {
        if (index < 0 || (index+length) >= source.length()) return null;
        return source.substring(index+length);
    }

    private String skipNonSpace(String source) throws BadTokenException {
        int index = 0;
        if (source == null) {
            return null;
        }
        int length = source.length();
        while ((index < length) &&
               !Character.isWhitespace(source.charAt(index))) {
            index++;
        }
	return skipPCData(source, source.charAt(index));
    }

    private String skipWhitespace(String source) {
        int index = 0;
        if (source == null) {
            return null;
        }
        while ((index < source.length()) &&
	        Character.isWhitespace(source.charAt(index))) {
            ++index;
        }
        return source.substring(index);
    }

    private boolean legalTokenStartChar(char c) {
        return ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c == '_') || (c == ':' ));
    }

    private boolean legalTokenChar(char c) {
        return ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                (c == '_') || (c == ':' ) ||
                (c == '.') || (c == '-'));
    }

    private String skipAttributeName(String source) {

        if (source == null) {
            return null;
        }
        int index = source.indexOf("=");
        if (index >= 0) {
            _tokenData = source.substring(0, index);
	    if (_tokenData != null) {
		_tokenData = _tokenData.trim();
	    }
        } else {
            _tokenData = null;
        }
        return skipForward(source, index, 0);
    }

    private String skipXMLName(String source) {
        int index = 0;
        if (source == null) {
            return null;
        }
        if (legalTokenStartChar(source.charAt(0))) {
            index = 1;
            while ((index < source.length()) &&
                   legalTokenChar(source.charAt(index))) {
                index++;
            }
        }
        _tokenData = source.substring(0, index);
        if (_tokenData != null) {
	    _tokenData = _tokenData.trim();
        }
        return skipForward(source, index, 0);
    }

    private String skipXMLComment(String source) {
        if ((source != null) && (source.startsWith("<!--"))) {
            int index = source.indexOf("-->", 4);
            return skipForward(source, index, 3);
        }
        return source;
    }

    private String skipXMLDocType(String source) {
        // make sure we don't skip PCData
        if ((source != null) && (source.startsWith("<!")) &&
                source.startsWith(CDStart) == false) {
            int index = source.indexOf(">",2);
            return skipForward(source, index, 1);
        }
        return source;
    }

    private String skipXMLProlog(String source) {
        if ((source != null) && (source.startsWith("<?"))) {
            int index = source.indexOf("?>",2);
            return skipForward(source, index, 2);
        }
        return source;
    }

    private String skipFilling(String source) {
        String next, prev;
        next = source;
        do {
            prev = next;
            next = skipWhitespace(next);
            next = skipXMLComment(next);
            next = skipXMLDocType(next);
            next = skipXMLProlog(next);
        } while (next != prev);
        return next;
    }

    private int getLineNumber() {
        int end, lineCount;
        if (_current == null) {
            end = _source.length();
        } else {
            end = _source.indexOf(_current);
        }
        lineCount = 0;
        int index = 0;
        while ((index < end) && (index != -1)) {
            index = _source.indexOf("\n", index);
            if (index >= 0) {
		index++;
                lineCount++;
	    }
        }
        return lineCount;
    }
}
