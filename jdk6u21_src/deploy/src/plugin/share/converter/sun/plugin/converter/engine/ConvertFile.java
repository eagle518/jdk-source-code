/*
 * @(#)ConvertFile.java	1.28 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import java.io.*;
import java.util.*;
import sun.plugin.converter.util.*;
import sun.plugin.converter.engine.*;

public class ConvertFile 
{
    private byte[] templateBuffer;
    private File source;
    private File destination;

    private int appletsFound = 0;
    private int errors = 0;

    private int token;
    private String upperWord;
    private String eolStr;
    private String inEncoding = null;
    private String outEncoding = null;
    private ParsingState pState = new ParsingState();
    private String cabFileLocation = null;
    private String nsFileLocation = null;
    private String smartUpdateLocation = null;
    private String mimeType = null;
    private String classId = null;
    private boolean stdout = false; 

    private static final String PARAMTAG = "APPLETPARAMS";
    private static final String EMBEDPARAMTAG = "EMBEDPARAMS";
    private static final String ALTERNATEHTMLTAG = "ALTERNATEHTML";
    private static final String APPLETATTRSTAG = "APPLETATTRIBUTES";
    private static final String OBJECTPARAMSTAG = "OBJECTPARAMS";
    private static final String OBJECTATTRIBUTES = "OBJECTATTRIBUTES";
    private static final String APPLETTEXTTAG = "ORIGINALAPPLET";
    private static final String EMBEDATTRIBUTESTAG = "EMBEDATTRIBUTES";
    private static final String CABFILELOCATIONTAG = "CABFILELOCATION";
    private static final String NSFILELOCATIONTAG = "NSFILELOCATION";
    private static final String SMARTUPDATETAG = "SMARTUPDATE";
    private static final String MIMETYPETAG = "MIMETYPE";
    private static final String CLASSID = "CLASSID";
	   
    private static final String conversionTagBegin = "<!--\"CONVERTED_APPLET\"-->";			
    private static final String conversionTagEnd = "<!--\"END_CONVERTED_APPLET\"-->";		

    public ConvertFile(InputStream is) 
		       throws IOException
    { 
	/*
	 * We need to sort of clone the input stream in order to
	 * determine the EOL character and use it for parsing.,
	 * to avoid any side-effect
	 */
	ByteArrayOutputStream bos = new ByteArrayOutputStream();
	byte[] buf = new byte[2048];		
	int len = 0;

	while ((len = is.read(buf, 0, 2048)) != -1)
	    bos.write(buf, 0, len);

	templateBuffer = bos.toByteArray();
    }

    public void setSource(File f) { source = f; }
    public File getSource() { return source; }
		
    public void setDestination(File f) { destination = f; }
    public File getDestination() { return destination; }

    public String getCabFileLocation(){
	return cabFileLocation;
    }
    public void setCabFileLocation(String location) {
	cabFileLocation = location;
    }
    public String getNSFileLocation(){
	return nsFileLocation;
    }
    public void setNSFileLocation(String location) {
	nsFileLocation = location;
    }
    public String getSmartUpdateLocation(){
	return smartUpdateLocation;
    }
    public void setSmartUpdateLocation(String location) {
	smartUpdateLocation = location;
    }
    public String getMimeType(){
	return mimeType;
    }
    public void setMimeType(String type) {
	mimeType = type;
    }
    public String getClassId() {
       return classId;
    }
    public void setClassId(String classId) {
       this.classId = classId;
    }

    public int getAppletsFound() { return appletsFound; }
    public int getErrors() { return errors; }
    
    public void setEncoding(String enc) {
	outEncoding = inEncoding = enc;
    }

    public void setStandardOutput(boolean std) {
	stdout = std;
    }

    /**
     * Returns the encoding value that has been previously set. 
     * PluginConverter.java uses this when it calls the countWords() method 
     * in the utils package
     * @return the value of the encoding that was set
     */
    public String getEncoding(){
	return inEncoding;
    }

    public String parseCharset(String content){
	int i;
	i = content.indexOf("charset");
	if (i < 0)
	    return null;
	String sub = content.substring(i);
	StringTokenizer st = new StringTokenizer(sub, " =\t\n\r");
	st.nextToken();
	return st.nextToken();
    }
    
    public void guessEncoding() {
	int state = 0;
	int c;

	try {
	    //This has been changed to use InputStreamReader because 
	    //BufferedReader reads all of the characters in the file 
	    //ahead and converts them from bytes to chars in advance. 
	    //We don't know the encoding yet so we only can read the top
	    //of the file. InputStreamReader doesn't convert the whole 
	    //file in advance.  We will only need to parse the all-english
	    //tags at the top, which will include the encoding.
   
            InputStreamReader inRead = new InputStreamReader(new BufferedInputStream(new FileInputStream(source)));
	    StreamTokenizer st = new StreamTokenizer(inRead);
	    st.wordChars('<', '>');
	    st.whitespaceChars('=','=');
	    st.lowerCaseMode(true);
	    while(true) {
		c = st.nextToken();
		switch(c) {
		case StreamTokenizer.TT_WORD:
		    if (st.sval.equals("</head>"))
			return;
		    switch (state) {
		    case 0:
			if (st.sval.equals("<head>"))
			    state = 1;
			break;
		    case 1:
			if (st.sval.equals("content")) {
			    st.nextToken();
			    setEncoding(parseCharset(st.sval));
			    return;
			}
			break;
		    }
		    break;
		case StreamTokenizer.TT_EOF:
                    return;
		}
	    }
	} catch (Exception e) {
	    System.err.println("Something bad happened: "+e);
	}
	return;
    }

    public boolean convert() throws Exception {
	
	AppletPieces pieces;

	//added getEncoding parameter because it needs to know encoding in 
	//order to parse
	eolStr = StdUtils.getEOLs(new FileInputStream(source), getEncoding());
	if ((inEncoding == null) && (outEncoding == null)) {
	    guessEncoding();
	}
        BufferedWriter outFile;
	BufferedReader inBuf;
  
        if (stdout) {
	   if (outEncoding == null)
	       outFile  = new BufferedWriter(new FileWriter(FileDescriptor.out));	
	   else
	       outFile  = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(FileDescriptor.out),outEncoding));	
        }
        else {
	   if (outEncoding == null)
	       outFile  = new BufferedWriter(new FileWriter(destination));	
	   else
	       outFile  = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(destination),outEncoding));
        }
        
        if (inEncoding == null) 
	      inBuf = new BufferedReader(new FileReader(source));
	else 
	      inBuf = new BufferedReader(new InputStreamReader(new FileInputStream(source),inEncoding));
 
	StreamTokenizer st = new StreamTokenizer(inBuf);

	setTagSearch(st);
		
	//  Cycle through the file, getting all tokens and ord chars for processing
	while((token = st.nextToken())!=StreamTokenizer.TT_EOF) {
		
	skip:
	    if(token==StreamTokenizer.TT_WORD) {	//  Token is a WORD

		upperWord = st.sval.toUpperCase().trim();  //  Get and upper the WORD

		if(pState.isPotentialTag()) {
		    if(upperWord.equals(pState.getTarget())) {  //  Found beginning of APPLET
			//  Parse the applet
			if((pieces=getAppletPieces(st))!=null) appletsFound++;

			String convertedApplet = convertToOBJECT(pieces);
			outFile.write(conversionTagBegin+eolStr);
			outFile.write(convertedApplet);
			outFile.write(conversionTagEnd+eolStr);

			//  Besure we are in no state
			setTagSearch(st);

			//  Burn the last > if needed.
			if((token = st.nextToken())!=StreamTokenizer.TT_EOF)
			    if((char)token != pState.getPotentialTagChar()) break skip;
		    }
		    else if(upperWord.equals("!--")) {
			boolean dontBreak = false;
			outFile.write(pState.getPotentialTagChar()+"!--" );
						
			setCommentScan(st);
		    stripped:	
			while((token = st.nextToken())!=StreamTokenizer.TT_EOF) {
			    if(token==StreamTokenizer.TT_WORD) {	//  Token is a WORD
								
				upperWord = st.sval.toUpperCase().trim();  //  Get and upper the WORD
				outFile.write(st.sval);
				if(upperWord.indexOf("-->")>=0)	if(!dontBreak) break stripped;  
			    }
			    else if(token == ((int)'"')) {
				upperWord = st.sval.toUpperCase().trim();  //  Get and upper the WORD
				if(upperWord.equals("CONVERTED_APPLET")) dontBreak = true;
				if(upperWord.equals("END_CONVERTED_APPLET")) dontBreak = false;
				outFile.write("\""+st.sval+"\"");
			    }
			    else outFile.write(String.valueOf((char)token));
			}
						
			setTagSearch(st);
		    }
		    else {	    
			if(pState.isPotentialTag()) outFile.write(pState.getPotentialTagChar()+st.sval);
			else outFile.write(st.sval);
		    }
		}
		else {  // False alarm, tag was not APPLET
		    if(pState.isPotentialTag()) outFile.write(pState.getPotentialTagChar()+st.sval);
		    else outFile.write(st.sval);
					
		}
		pState.clearPotentialTag();

	    }
	    else {			//  Token is an ordinary CHAR
		if((char)token == pState.getPotentialTagChar()) pState.setPotentialTag();
		else { 
		    if(pState.isPotentialTag()) {
			outFile.write(pState.getPotentialTagChar()+String.valueOf((char)token));
		    }
		    else {
			outFile.write(String.valueOf((char)token));
		    }

		    pState.clearPotentialTag();

		}
	    }
	}


	inBuf.close();
	outFile.flush();
	outFile.close();

	if(appletsFound>0) return true;
	else return false;
    }
    private void setCommentScan(StreamTokenizer st) {
	st.resetSyntax();
		
	st.wordChars('-','-');
	st.wordChars('>','>');
	st.quoteChar('"');
	pState.setCommentState();
    }
    private void setTagSearch(StreamTokenizer st) {
	st.resetSyntax();

	st.wordChars('A','A');
	st.wordChars('a','a');
	st.wordChars('P','P');
	st.wordChars('p','p');
	st.wordChars('L','L');
	st.wordChars('l','l');
	st.wordChars('E','E');
	st.wordChars('e','e');
	st.wordChars('T','T');
	st.wordChars('t','t');
	st.wordChars('-','-');
	st.wordChars('!','!');

	pState.setScanState();
    }
		
    private void setAppletScan(StreamTokenizer st) {
	st.resetSyntax();

	st.wordChars(33,255);
	st.whitespaceChars(0,' ');
	st.whitespaceChars(0,' ');
	st.ordinaryChar('>');
	st.ordinaryChar('=');
	st.quoteChar('"');

	pState.setAppletScanState();
    }
    private void setParamOrEndScan(StreamTokenizer st) {
	st.resetSyntax();
		
	st.wordChars('A','A');
	st.wordChars('a','a');
	st.wordChars('P','P');
	st.wordChars('p','p');
	st.wordChars('L','L');
	st.wordChars('l','l');
	st.wordChars('E','E');
	st.wordChars('e','e');
	st.wordChars('T','T');
	st.wordChars('t','t');
	st.wordChars('R','R');
	st.wordChars('r','r');
	st.wordChars('M','M');
	st.wordChars('m','m');
	st.wordChars('/','/');

	pState.setAltOrEndScanState();

    }
    private void setParamScan(StreamTokenizer st) {
	st.resetSyntax();

	st.wordChars(33,255);
	st.whitespaceChars(0,' ');
	st.ordinaryChar('>');
	st.whitespaceChars('=','=');
	st.quoteChar('"');
		
	pState.setParamScanState();
    }
		
    public void getAppletPortion(StreamTokenizer st, AppletPieces pieces) throws Exception{
		
	setAppletScan(st);
					
	while((token = st.nextToken())!=StreamTokenizer.TT_EOF) {

	    if(token==StreamTokenizer.TT_WORD) {

		upperWord = st.sval.toUpperCase().trim();
		if(upperWord.equals("CODEBASE")) {
		    pieces.setCODEBASE("CODEBASE = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("ARCHIVE")) {
		    pieces.setARCHIVE("ARCHIVE = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("CODE")) {
		    pieces.setCODE("CODE = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("OBJECT")) {
		    pieces.setOBJECT("OBJECT = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("ALT")) {
		    pieces.setALT("ALT = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("NAME")) {
		    pieces.setNAME("NAME = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("WIDTH")) {
		    pieces.setWIDTH("WIDTH = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("HEIGHT")) {
		    pieces.setHEIGHT("HEIGHT = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("ALIGN")) {
		    pieces.setALIGN("ALIGN = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("VSPACE")) {
		    pieces.setVSPACE("VSPACE = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("HSPACE")) {
		    pieces.setHSPACE("HSPACE = " + getAttributeValue(pState,st));
		}
		else if(upperWord.equals("MAYSCRIPT")) {
		    pieces.setMAYSCRIPT("MAYSCRIPT = " + getAttributeValueForMAYSCRIPT(pState,st));
		}

		if(pState.isStateClear()) return;
	    }
	    else {
		if((char)token == '>') { 
		    pState.clearScanState();
		    return;
		}
	    }					
	}
    }
		
    public AppletPieces getAppletPieces(StreamTokenizer st) throws Exception{
		
	AppletPieces pieces = new AppletPieces(eolStr);

	getAppletPortion(st,pieces);
	getParamsAndAlt(st,pieces);
		
	return pieces;
    }
	
    public void getParamsAndAlt(StreamTokenizer st, AppletPieces pieces) throws Exception {
	
	StringBuffer altText = new StringBuffer();
	boolean	     isComment = false;	
	setParamOrEndScan(st);

	while((token = st.nextToken())!=StreamTokenizer.TT_EOF) {
	    if(token!=StreamTokenizer.TT_WORD) {
		if((char)token == pState.getPotentialTagChar()) pState.setPotentialTag();
		else { 
		    if(pState.isPotentialTag()) {
			altText.append(pState.getPotentialTagChar()+String.valueOf((char)token));
		    }
		    else {
                        altText.append(String.valueOf((char)token));
                        if ( isComment ) {
                            if ( altText.indexOf("-->") >= 0 ) {
                                isComment = false;
                                altText = new StringBuffer("");
                            }
                        }
                        else {
                            if ( altText.indexOf("<!--") >= 0 ) {
                                isComment = true;
                                altText = new StringBuffer("");
                            }
                        }
		    }

		    pState.clearPotentialTag();
		}
	    }
	    else {

		upperWord = st.sval.toUpperCase().trim();
		// This will stop the comment
		if(upperWord.equals("-->") && isComment) {
		    isComment = false;
                    setParamOrEndScan(st);
		}
		//  Looking for /APPLET, if we find it, we are DONE!
		if(pState.isPotentialTag()) {
		    if(upperWord.equals(pState.getTarget())) {
			pState.clearScanState();
			pieces.setAlternateHTML(altText.toString().trim());
                        setTagSearch(st);
			return;
		    }
		    else if(upperWord.equals("PARAM")) {
			if(!isComment) {
			    pieces.addParam(getParamString(st,pState));
			    setParamOrEndScan(st);
			}
		    }
		    else if(upperWord.equals("!--")) {
			isComment = true;
                        setCommentScan(st);
		    }
		    else altText.append(pState.getPotentialTagChar()+st.sval);
		}
		else altText.append(st.sval);
				
		pState.clearPotentialTag();
	    }
	}
	return;
	
    }
		
    public String getAttributeValue(ParsingState pState, StreamTokenizer st) throws Exception{
	
	boolean nameNeedsQuotes = false;

	token = st.nextToken();  //  Burn the =
	token = st.nextToken();  //  get the value potentially
	String value;
	if(token=='"') {
	    nameNeedsQuotes = true;
	    value = st.sval;
	}
	else if(token==StreamTokenizer.TT_WORD) {
	    value =  st.sval;
	    if(value.endsWith(">")) {
		value = value.substring(0,value.length()-1);
		pState.clearScanState();
	    }
	}
	else value =  String.valueOf(token);
		
	if(nameNeedsQuotes) value = "\""+value+"\"";
		
	return value;
    }


    /**
     * getAttributeValueForMAYSCRIPT obtains the value for MAYSCRIPT 
     * from the APPLET tag.
     *
     * @param pState Parsing state.
     * @param st stream tokenizer.
     * @return Value for MAYSCRIPT
     *
     */
    public String getAttributeValueForMAYSCRIPT(ParsingState pState, StreamTokenizer st) 
		  throws Exception
    {
        String value;
	token = st.nextToken();	 // Check if the token is =

	if (token == '=')  
	{
    	    boolean nameNeedsQuotes = false;
	    token = st.nextToken();  //  get the value potentially
	    if(token=='"') {
		nameNeedsQuotes = true;
		value = st.sval;
	    }
	    else if(token==StreamTokenizer.TT_WORD) {
		value =  st.sval;
		if(value.endsWith(">")) {
		    value = value.substring(0,value.length()-1);
		    pState.clearScanState();
		}
	    }
	    else value =  String.valueOf(token);
		    
	    if(nameNeedsQuotes) value = "\""+value+"\"";

	    return value;
	}
	else if (token == '>')
	{
	    value = "true";
	    pState.clearScanState();
	}
	else
	{
	    // This is really important to push back the token here,
	    // because MAYSCRIPT can exist within value. In this case,
	    // it is defaulted to be "true".
	    st.pushBack();
	    value = "true";
	}

	return value;
    }

    public String getParamString(StreamTokenizer st, ParsingState pState) throws Exception {
					
	setParamScan(st);

	boolean nameNeedsQuotes = false;
	boolean valueNeedsQuotes = false;

	//  HANDLE SUDDEN ENDS TO STREAM ON THESE NEXT TOKEN CALLS!!
	token = st.nextToken();  // Burn NAME
	token = st.nextToken();	//  Get Value

	String value, pName, pValue;
		
	if(token=='"') {
	    value = st.sval;
	    nameNeedsQuotes = true;
	}
	else if(token==StreamTokenizer.TT_WORD) {
	    value =  st.sval;
	}
	else value =  String.valueOf(token);

	pName = value;

	//  HANDLE SUDDEN ENDS TO STREAM ON THESE NEXT TOKEN CALLS!!
	token = st.nextToken();  // Burn VALUE
		
	st.resetSyntax();
	StringBuffer valueBuf = new StringBuffer();
	boolean quoted = false;
	while( (token = st.nextToken()) != StreamTokenizer.TT_EOF)
	    {
		if(token == '>')
		    {
			if(!quoted) 
			    {
				break;
			    }
		    }
		if(token == '"')
		    {
			quoted = !quoted;
		    }
			
		valueBuf.append((char)token);
	    }
		
		
	pValue = valueBuf.toString();
			
	setParamScan(st);

	return "<PARAM NAME = "+(nameNeedsQuotes?"\"":"")+pName+(nameNeedsQuotes?"\"":"")+" VALUE"+pValue+">";
    }

    public String convertToOBJECT(AppletPieces pieces) throws Exception 
    {
	StreamTokenizer tSt = new StreamTokenizer(new BufferedReader(
				new InputStreamReader(new ByteArrayInputStream(templateBuffer))));
	tSt.resetSyntax();
	tSt.quoteChar('$');
			
	StringBuffer result = new StringBuffer();
	int retVal;
	String templateEOL = "";		
	int token;
	String tag = new String();
	String tab = "    ";
	String tabX3 = "            ";
        String newLine = "\n";
        String escape = " \\";
		
	templateEOL = StdUtils.getEOLs(new ByteArrayInputStream(templateBuffer), getEncoding());

	while((token = tSt.nextToken())!=StreamTokenizer.TT_EOF) 
	{
	    if((char)token=='$') {			
		tag = tSt.sval.toUpperCase().trim();	
		if(tag.equals(APPLETATTRSTAG)) result.append(pieces.getAttributes(false));
		else if(tag.equals(OBJECTPARAMSTAG)) result.append(pieces.getObjectTagParams(false));
		else if(tag.equals(OBJECTATTRIBUTES)) result.append(pieces.getObjectTagAttributes(false));
		else if(tag.equals(APPLETTEXTTAG)) result.append(pieces.getAppletText(false));
		else if(tag.equals(EMBEDATTRIBUTESTAG)) result.append(pieces.getEmbedTagAttributes());
		else if(tag.equals(ALTERNATEHTMLTAG)) result.append(pieces.getAlternateHTML());
		else if(tag.equals(CABFILELOCATIONTAG)) result.append(cabFileLocation);
		else if(tag.equals(NSFILELOCATIONTAG)) result.append(nsFileLocation);	
		else if(tag.equals(SMARTUPDATETAG)) result.append(smartUpdateLocation);
		else if(tag.equals(MIMETYPETAG)) result.append(mimeType);
		else if(tag.equals(CLASSID)) result.append(classId);
		else if(tag.equals(PARAMTAG)) {
		    Enumeration params = pieces.getParamEnumeration();
		    String aParam;
		    while(params.hasMoreElements()) {
			aParam = (String)params.nextElement();
			result.append(tab+aParam+eolStr);
		    }  
		}
		else if(tag.equals(EMBEDPARAMTAG)) {
		    Enumeration params = pieces.getParamEnumeration();
		    StringBuffer embedParams = new StringBuffer();
		    String aParam;
		    StringTokenizer pPieces;
		    String pToken;

		    //  Cycle through all the applet specific params
		    while(params.hasMoreElements()) {

			aParam = (String)params.nextElement();  //  Get the complete <PARAM ...> tag
			String cName = "";						//  will hold the name and value parsed
			String cValue = "";						//  will hold the name and value parsed
			pPieces = new StringTokenizer(aParam," \t\n\r=",false);  //  Tokenize the <PARAM ...> tag

			//  Parse for the name and value
			while(pPieces.hasMoreTokens()) {

			    pToken = pPieces.nextToken("\t\n\r=").trim();

			    //  If the token is name or value, store the attribute, else do nothing
			    if(pToken.toUpperCase().indexOf("NAME")>=0) {
				String ttoken = pPieces.nextToken(" ");

				if(ttoken.equals("="))
				    {
					ttoken = pPieces.nextToken(" ");
				    }
				else
				    {
					if(ttoken.charAt(0)=='=')
					    {
						ttoken = ttoken.substring(1,ttoken.length());
					    }
				    }
									
				cName = AppletPieces.convertEscapes(ttoken);
				if(cName.startsWith("\"")) cName = cName.substring(1,cName.length());
				if(cName.endsWith("\"")) cName = cName.substring(0,cName.length()-1);
			    }
			    else if(pToken.toUpperCase().equals("VALUE")) {
				cValue = AppletPieces.convertEscapes(setAttribute(pPieces));
			    }
			}
			if(cValue.endsWith(">")) cValue = cValue.substring(0,cValue.length()-1);
			//  Create the new tag				
			embedParams.append(escape+newLine+tabX3+cName+" ="+cValue);
		    }
							
		    result.append(new String(embedParams));

		}
		else { 
		    result.append(String.valueOf((char)token)); 
		}
	    }
	    else { 
		result.append(String.valueOf((char)token)); 
	    }
					
	}
					
	return result.toString();
    }		
    public String setAttribute(StringTokenizer appletTokens) 
    {
	String token = "";
	token = appletTokens.nextToken(">");
			
	while(appletTokens.hasMoreTokens()) 
	    {
		String temp = appletTokens.nextToken(">");
		if(temp.length()>0)
		    token += ">"+temp;
	    }

	token = token.trim();
	if(token.indexOf("=")==0)
	    token = token.substring(1,token.length());

	return token;
    }  
}
	

class ParsingState 
{
	//  STATES
	public static final int NOSTATE = 1;
	public static final int SCANNING = 2;
	public static final int APPLETSTATE = 3;
	public static final int PARAMSTATE = 4;
	public static final int ALTORENDSTATE = 5;
	public static final int COMMENTSTATE = 6;
	
		
	//  TARGETS
	public static final String APPLET_TARGET = "APPLET";
	public static final String END_APPLET_TARGET = "/APPLET";
	public static final String PARAM_TARGET = "PARAM";
	public static final String NO_TARGET = null;
	
	public int state = SCANNING;
	public String target = APPLET_TARGET;
	public boolean potentialTag = false;
	public char potentialTagChar = '<';

	public String toString() {
		return "";
		}
		
	public void setState(int aState) { state = aState; }
	public int getState() { return state; }
	public boolean isScanning() { return state==SCANNING; }
	public boolean isAppletScanning() { return state==APPLETSTATE; }
	public boolean isParamScanning() { return state==PARAMSTATE; }
	public boolean isAltOrEndScanning() { return state==ALTORENDSTATE; }
	public boolean isCommentState() { return state==COMMENTSTATE; }
	
	public void setTarget(String target) { this.target = target; }
	public String getTarget() { return target; }
	
	public void setPotentialTag() { potentialTag = true; }
	public boolean isPotentialTag() { return potentialTag; }
	public void clearPotentialTag() { potentialTag = false; }

	public void setPotentialTagChar(char c) { potentialTagChar = c; }
	public char getPotentialTagChar() { return potentialTagChar; }
	
	public void clearScanState() {
		state = NOSTATE;
		target = NO_TARGET;
		potentialTag = false;
		potentialTagChar = ' ';
		}
	public void setScanState() {
		state = SCANNING;
		target = APPLET_TARGET;
		potentialTag = false;
		potentialTagChar = '<';
		}
	public void setAppletScanState() {
		state = APPLETSTATE;
		target = NO_TARGET;
		potentialTag = false;
		potentialTagChar = ' ';
		}
	public void setParamScanState() {
		state = PARAMSTATE;
		target = NO_TARGET;
		potentialTag = false;
		potentialTagChar = ' ';
		}
	public void setAltOrEndScanState() {
		state = ALTORENDSTATE;
		target = END_APPLET_TARGET;
		potentialTag = false;
		potentialTagChar = '<';
		}
	public void setCommentState() {
		state = COMMENTSTATE;
		target = "--";
		potentialTag = false;
		potentialTagChar = ' ';
		}	
	public boolean isStateClear() { return state==NOSTATE; }
	
}
