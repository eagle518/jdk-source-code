<?xml version="1.0"?> 
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="jvmtiLib.xsl"/>

<xsl:output method="html" indent="yes" 
  doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN" 
  doctype-system="http://www.w3.org/TR/html4/loose.dtd"/>

<xsl:param name="development"></xsl:param>

<xsl:template match="specification">
  <html>
  <head>
        <title>
          <xsl:value-of select="@label"/>
          <xsl:text> </xsl:text>
          <xsl:call-template name="showversion"/>
        </title>
        <style type="text/css">
          td.tableHeader {font-size: larger}
        </style>
  </head>
  <body>
    <table border="0" width="100%">
      <tr>
        <td align="center">
          <xsl:apply-templates select="title"/>
        </td>
      </tr>
    </table>
    <ul>
      <li>
        <a href="#SpecificationIntro"><b>Introduction</b></a>
        <ul>
          <xsl:for-each select="intro">
            <li>
              <a>
                <xsl:attribute name="href">#<xsl:value-of select="@id"/>
                </xsl:attribute>
                <b><xsl:value-of select="@label"/></b>
              </a>
            </li>
          </xsl:for-each>
        </ul>
      </li>
      <li>
        <a href="#FunctionSection"><b>Functions</b></a>
        <ul>
          <xsl:for-each select="functionsection/intro">
            <li>
              <a>
                <xsl:attribute name="href">#<xsl:value-of select="@id"/>
                </xsl:attribute>
                <b><xsl:value-of select="@label"/></b>
              </a>
            </li>
          </xsl:for-each>
          <li>
            <a href="#FunctionIndex"><b>Function Index</b></a>
            <ul>
              <xsl:for-each select="functionsection/category">
                <li>
                  <a>
                    <xsl:attribute name="href">#<xsl:value-of select="@id"/>
                    </xsl:attribute>
                    <b><xsl:value-of select="@label"/></b>
                  </a>
                </li>
              </xsl:for-each>
            </ul>
          </li>
          <li>
            <a href="#ErrorSection"><b>Error Codes</b></a>
          </li>
        </ul>
      </li>
      <li>
        <a href="#EventSection"><b>Events</b></a>
        <ul>
          <li>
            <a href="#EventIndex"><b>Event Index</b></a>
          </li>
        </ul>
      </li>
      <li>
        <a href="#DataSection"><b>Data Types</b></a>
        <ul>
          <xsl:for-each select="//basetypes">   
          <li>
            <a>
              <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
              <b>
                <xsl:value-of select="@label"/>
              </b>
            </a>
          </li>
          </xsl:for-each>
          <li>
            <a href="#StructureTypeDefinitions"><b>Structure Type Definitions</b></a>
          </li>
          <li>
            <a href="#FunctionTypeDefinitions"><b>Function Type Definitions</b></a>
          </li>
          <li>
            <a href="#EnumerationDefinitions"><b>Enumeration Definitions</b></a>
          </li>
        </ul>
      </li>
      <li>
        <a href="#ConstantIndex"><b>Constant Index</b></a>
      </li>
      <xsl:if test="$development = 'Show'">
        <li>
          <a href="#SpecificationIssues"><b>Issues</b></a>
          <ul>
            <xsl:for-each select="issuessection/intro">
              <li>
                <a>
                  <xsl:attribute name="href">#<xsl:value-of select="@id"/>
                  </xsl:attribute>
                  <b><xsl:value-of select="@label"/></b>
                </a>
              </li>
            </xsl:for-each>
          </ul>
        </li>
      </xsl:if>
      <li>
        <a href="#ChangeHistory"><b>Change History</b></a>
      </li>
    </ul>
    <!-- end table of contents, begin body -->
    <p/>
    <hr noshade="noshade" size="3"/>
    <p/>
    <p id="SpecificationIntro"/>
      <xsl:apply-templates select="intro"/>
    <p id="FunctionSection"/>
      <xsl:apply-templates select="functionsection"/>
    <p id="ErrorSection"/>
      <xsl:apply-templates select="errorsection"/>
    <p id="DataSection"/>
      <xsl:apply-templates select="datasection"/>
    <p id="EventSection"/>
      <xsl:apply-templates select="eventsection"/>
    <p id="ConstantIndex"/>
      <p/>
      <hr noshade="noshade" size="3"/>
      <h2>
        Constant Index
      </h2>
      <blockquote>
        <xsl:apply-templates select="//constant" mode="index">
          <xsl:sort select="@id"/>
        </xsl:apply-templates>
      </blockquote>
    <xsl:if test="$development = 'Show'">
      <p id="SpecificationIssues"/>
      <p/>
      <hr noshade="noshade" size="3"/>
      <h2>
        <xsl:value-of select="issuessection/@label"/>
      </h2>
      <xsl:apply-templates select="issuessection/intro"/>
    </xsl:if>
    <p id="ChangeHistory"/>
      <xsl:apply-templates select="changehistory"/>
  </body>
</html>
</xsl:template>

<xsl:template match="title">
    <h1>
      <xsl:apply-templates/>
    </h1>
    <h3>
      <xsl:value-of select="@subtitle"/>
      <xsl:text> </xsl:text>
      <xsl:call-template name="showbasicversion"/>
    </h3>
</xsl:template>

<xsl:template match="functionsection">
  <p/>
  <hr noshade="noshade" size="3"/>
  <h2>
    <xsl:value-of select="@label"/>
  </h2>
  <xsl:apply-templates select="intro"/>
  <h3 id="FunctionIndex">Function Index</h3>
  <ul>
    <xsl:apply-templates select="category" mode="index"/>
  </ul>
  <xsl:apply-templates select="category" mode="body"/>
</xsl:template>

<xsl:template match="category" mode="index">
  <li>
    <a>
      <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
      <b>
        <xsl:value-of select="@label"/>
      </b>
    </a>
    <ul>
      <xsl:apply-templates select="function[count(@hide)=0]" mode="index"/>
    </ul>
  </li>
</xsl:template>

<xsl:template match="function|callback" mode="index">
  <li>
    <a>
      <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
      <xsl:apply-templates select="synopsis" mode="index"/>
    </a>
  </li>
</xsl:template>

<xsl:template match="synopsis" mode="index">
    <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="category" mode="body">
  <p>
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </p>
  <hr noshade="noshade" size="3"/>
  <h2 align="center"><xsl:value-of select="@label"/></h2>
  <xsl:value-of select="@label"/> functions:
  <ul>
    <xsl:apply-templates select="function[count(@hide)=0]" mode="index"/>
  </ul>
  <xsl:variable name="calltypes" select="callback"/>
  <xsl:if test="count($calltypes)!=0">
    <xsl:value-of select="@label"/> function types:
    <ul>
      <xsl:apply-templates select="$calltypes" mode="index"/>
    </ul>
  </xsl:if>
  <xsl:variable name="cattypes" 
    select="(descendant::typedef|descendant::capabilitiestypedef)"/>
  <xsl:if test="count($cattypes)!=0">
    <xsl:value-of select="@label"/> types:
    <ul>
      <xsl:for-each select="$cattypes">
        <li>
          <a>
            <xsl:attribute name="href">
              <xsl:text>#</xsl:text>
              <xsl:value-of select="@id"/>
            </xsl:attribute>
            <code><xsl:value-of select="@id"/></code>
          </a>
          <xsl:text> - </xsl:text>
          <xsl:value-of select="@label"/>
        </li>
      </xsl:for-each>
    </ul>    
  </xsl:if>
  <xsl:apply-templates select="intro|typedef|capabilitiestypedef"/>
  <xsl:apply-templates select="typedef|capabilitiestypedef" mode="body"/>
  <p/>
  <xsl:apply-templates select="function[count(@hide)=0]|callback" mode="body"/>
</xsl:template>

<xsl:template match="function" mode="body">
  <hr noshade="noshade" width="100%" size="1">
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
    
  </hr>
  <xsl:apply-templates select="synopsis" mode="body"/>
  <table cellpadding="0" cellspacing="0" border="0" width="90%" align="center"><tr><td>
  <blockquote>
    <xsl:apply-templates select="typedef"/>
    <xsl:apply-templates select="descendant::constants[@kind='enum']" mode="signature"/>
    <pre>
      <xsl:text>jvmtiError
</xsl:text>
      <xsl:value-of select="@id"/>(jvmtiEnv* env<xsl:apply-templates select="parameters" mode="signature"/>)</pre>
  </blockquote>
  <xsl:apply-templates select="description"/>
  <xsl:apply-templates select="." mode="phaseinfo"/>
  <xsl:apply-templates select="." mode="callbacksafeinfo"/>
  <xsl:apply-templates select="capabilities|eventcapabilities"/>
  <xsl:apply-templates select="typedef" mode="body"/>
  <xsl:apply-templates select="parameters" mode="body"/>
  <xsl:apply-templates select="." mode="errors"/>
</td></tr></table>
</xsl:template>

<xsl:template match="function" mode="phaseinfo">
  This function may
  <xsl:choose>
    <xsl:when test="count(@phase) = 0 or @phase = 'live'">
      only be called during the live
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="@phase = 'onload'">
          only be called during the OnLoad or the live
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="@phase = 'any'">
              be called during any
            </xsl:when>
            <xsl:otherwise>
              <xsl:choose>
                <xsl:when test="@phase = 'start'">
                  only be called during the start or the live
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="@phase = 'onloadOnly'">
                      only be called during the OnLoad
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:message terminate="yes">
                        bad phase - <xsl:value-of select="@phase"/>
                      </xsl:message>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:otherwise>
            </xsl:choose>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
  <a href="#jvmtiPhase">phase</a>.
  <p/>
</xsl:template>


<xsl:template match="event" mode="phaseinfo">
  This event is sent
  <xsl:choose>
    <xsl:when test="count(@phase) = 0 or @phase = 'live'">
      only during the live
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="@phase = 'any'">
          during the primordial, start or live
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="@phase = 'start'">
              during the start or live
            </xsl:when>
            <xsl:otherwise>
              <xsl:message terminate="yes">
                bad phase - <xsl:value-of select="@phase"/>
              </xsl:message>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
  <a href="#jvmtiPhase">phase</a>.
  <p/>
</xsl:template>


<xsl:template match="function" mode="callbacksafeinfo">
  <xsl:if test="contains(@callbacksafe,'safe')">
    This function may be called from the callbacks to the
    <a href="#Heap">Heap</a> iteration functions, or from the
    event handles for the 
    <a href="#GarbageCollectionStart"><code>GarbageCollectionStart</code></a>,
    <a href="#GarbageCollectionFinish"><code>GarbageCollectionFinish</code></a>,
    and <a href="#ObjectFree"><code>ObjectFree</code></a> events.
  </xsl:if>
</xsl:template>


<xsl:template match="callback" mode="body">
  <hr noshade="noshade" width="100%" size="1">
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </hr>
  <xsl:apply-templates select="synopsis" mode="body"/>
  <table cellpadding="0" cellspacing="0" border="0" width="90%" align="center"><tr><td>
  <blockquote>
    <pre>
      <xsl:text>typedef </xsl:text>
      <xsl:apply-templates select="child::*[position()=1]" mode="signature"/>
      <xsl:text> (JNICALL *</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>)
    (</xsl:text>
      <xsl:for-each select="parameters">
        <xsl:apply-templates select="param[position()=1]" mode="signature"/>
        <xsl:for-each select="param[position()>1]">
          <xsl:text>, 
     </xsl:text>
          <xsl:apply-templates select="." mode="signature"/>
        </xsl:for-each>
      </xsl:for-each>
      <xsl:text>);</xsl:text>
    </pre>
  </blockquote>
  <xsl:apply-templates select="description"/>
  <xsl:apply-templates select="parameters" mode="body"/>
</td></tr></table>
</xsl:template>

<xsl:template match="synopsis" mode="body">
  <h3><xsl:value-of select="."/></h3>
</xsl:template>

<xsl:template match="eventsection">
  <p/>
  <hr noshade="noshade" size="3"/>
  <h2>
    <xsl:value-of select="@label"/>
  </h2>
  <xsl:apply-templates select="intro"/>
  <blockquote>
  <pre>
  <xsl:text>
typedef struct {
</xsl:text>
  <xsl:call-template name="eventStruct">
    <xsl:with-param name="events" select="event"/>
    <xsl:with-param name="index" select="0"/>
    <xsl:with-param name="started" select="false"/>
    <xsl:with-param name="comment" select="'No'"/>
  </xsl:call-template>
  <xsl:text>} jvmtiEventCallbacks;
</xsl:text>
  </pre>
  </blockquote>
  <p/>
  <hr noshade="noshade" width="100%" size="1"/>
  <h3 id="EventIndex">Event Index</h3>
  <ul>
    <xsl:apply-templates select="event" mode="index">
      <xsl:sort select="@label"/>
    </xsl:apply-templates>
  </ul>
  <xsl:apply-templates select="event" mode="body"/>
</xsl:template>

<xsl:template match="event" mode="index">
  <li>
    <a>
      <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
      <b>
        <xsl:value-of select="@label"/>
      </b>
    </a>
  </li>
</xsl:template>

<xsl:template match="event" mode="body">
  <p>
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </p>
  <hr noshade="noshade" size="3"/>
  <h2><xsl:value-of select="@label"/></h2>
  <p/>
  <blockquote>
    <xsl:apply-templates select="typedef"/>
    <pre>
<xsl:text>void JNICALL
</xsl:text>
      <xsl:value-of select="@id"/>(jvmtiEnv *jvmti_env<xsl:apply-templates select="parameters" mode="signature"/>)</pre>
  </blockquote>
  <xsl:apply-templates select="description"/>
  <xsl:apply-templates select="." mode="phaseinfo"/>
  <b>Event ID:</b>
  <blockquote>
    <code><xsl:value-of select="@const"/> = <xsl:value-of select="@num"/></code>
  </blockquote>
  <p/>
  <b>Enabling:</b>
  <blockquote>
        All events are initially disabled. Enable globally with
          <pre>
            <a href="#SetEventNotificationMode">SetEventNotificationMode</a>(JVMTI_ENABLE, 
        <xsl:value-of select="@const"/>, NULL)
          </pre>
  </blockquote>
  <xsl:apply-templates select="typedef" mode="body"/>
  <xsl:apply-templates select="capabilities"/>
  <xsl:apply-templates select="parameters" mode="body"/>
</xsl:template>

<xsl:template match="capabilitiestypedef">
  <blockquote>
    <pre>
      <xsl:apply-templates select="." mode="genstruct"/>
    </pre>
  </blockquote>
</xsl:template>

<xsl:template match="typedef">
  <pre>
  <xsl:call-template name="gentypedef">
    <xsl:with-param name="tdef" select="."/>
  </xsl:call-template>
  </pre>
</xsl:template>

<xsl:template match="constants" mode="signature">
  <pre>
  <xsl:apply-templates select="." mode="enum"/>
  </pre>
</xsl:template>

<xsl:template match="typedef" mode="body">
  <p>
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </p>
    <table border="1" cellpadding="3" cellspacing="0" width="100%">
      <tr bgcolor="#CCCCFF">
        <td colspan="3" align="center" class="tableHeader">
          <code><xsl:value-of select="@id"/></code> - <xsl:value-of select="@label"/>
        </td>
      </tr>
      <tr bgcolor="#EEEEFF">
        <td>
          Field
        </td>
        <td>
          Type
        </td>
        <td>
          Description
        </td>
      </tr>
      <xsl:apply-templates select="field" mode="body"/>
    </table>
</xsl:template>

<xsl:template match="capabilitiestypedef" mode="body">
  <p>
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </p>
    <table border="1" cellpadding="3" cellspacing="0" width="100%">
      <tr bgcolor="#CCCCFF">
        <td colspan="3" align="center" class="tableHeader">
          <code><xsl:value-of select="@id"/></code> - <xsl:value-of select="@label"/>
        </td>
      </tr>
      <tr bgcolor="#EEEEFF">
        <td colspan="3">
          All types are <code>unsigned int : 1</code>
        </td>
      </tr>
      <tr bgcolor="#EEEEFF">
        <td>
          Field
        </td>
        <td>
          Description
        </td>
        <td>
        </td>
      </tr>
      <xsl:apply-templates select="capabilityfield" mode="body"/>
    </table>
</xsl:template>

<xsl:template match="typedef|capabilitiestypedef|constants" mode="tableentry">
  <tr>
    <td>
      <a>
        <xsl:attribute name="href">
          <xsl:text>#</xsl:text>
          <xsl:value-of select="@id"/>
        </xsl:attribute>
        <code><xsl:value-of select="@id"/></code>
      </a>
    </td>
    <td>
      <xsl:value-of select="@label"/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="field" mode="body">
  <tr>
    <td>
      <code>
        <xsl:attribute name="id">
          <xsl:value-of select="../@id"/>.<xsl:value-of select="@id"/>
        </xsl:attribute>
        <xsl:value-of select="@id"/>
      </code>
    </td>
    <td>
      <code>
        <xsl:apply-templates select="child::*[position()=1]" mode="link"/>
      </code>
    </td>
    <td>
      <xsl:apply-templates select="description" mode="brief"/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="capabilityfield" mode="body">
  <tr>
    <td>
      <code>
        <xsl:choose>
          <xsl:when test="@disp1!=''">
            <xsl:value-of select="@disp1"/>
            <br></br>
            <xsl:value-of select="@disp2"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@id"/>
          </xsl:otherwise>
        </xsl:choose>
      </code>
    </td>
    <td>
      <a>
        <xsl:attribute name="name">
          <xsl:value-of select="../@id"/>.<xsl:value-of select="@id"/>
        </xsl:attribute>
      </a>
      <xsl:apply-templates select="description" mode="brief"/>
    </td>
    <td>
    </td>
  </tr>
</xsl:template>

<xsl:template match="callback" mode="tableentry">
  <tr>
    <td>
      <a>
        <xsl:attribute name="href">
          <xsl:text>#</xsl:text>
          <xsl:value-of select="@id"/>
        </xsl:attribute>
        <code>
          <xsl:value-of select="@id"/>
        </code>
      </a>
    </td>
    <td>
      <xsl:apply-templates select="synopsis" mode="index"/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="constants">
  <blockquote>
  <a>
    <xsl:attribute name="name">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </a>
    <table border="1" cellpadding="3" cellspacing="0">
      <tr bgcolor="#CCCCFF">
        <td colspan="3" align="center" class="tableHeader">
            <xsl:value-of select="@label"/>
            <xsl:if test="@kind='enum'">
              <xsl:text> (</xsl:text>
              <code>
                <xsl:value-of select="@id"/>
              </code>
              <xsl:text>)</xsl:text>
            </xsl:if>
        </td>
      </tr>
      <tr bgcolor="#EEEEFF">
        <td>
          Constant
        </td>
        <td>
          Value
        </td>
        <td>
          Description
        </td>
      </tr>
      <xsl:apply-templates select="constant" mode="body"/>
    </table>
  </blockquote>
</xsl:template>

<xsl:template match="constant" mode="index">
  <a>
    <xsl:attribute name="href">#<xsl:value-of select="@id"/>
    </xsl:attribute>
    <code>
      <xsl:value-of select="@id"/>
    </code>
  </a>
  <br/>
</xsl:template>

<xsl:template match="constant" mode="body">
  <tr>
    <td>
      <code>
        <xsl:attribute name="id">
          <xsl:value-of select="@id"/>
        </xsl:attribute>
        <xsl:value-of select="@id"/>
      </code>
    </td>
    <td align="right">
      <xsl:value-of select="@num"/>
    </td>
    <td>
      <xsl:apply-templates/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="basetypes">
  <p>
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
  </p>
    <table border="1" cellpadding="3" cellspacing="0" width="100%">
      <tr bgcolor="#CCCCFF">
        <td colspan="2" align="center" class="tableHeader">
          <xsl:value-of select="@label"/>
        </td>
      </tr>
      <tr bgcolor="#EEEEFF">
        <td>
          Type
        </td>
        <td>
          Description
        </td>
      </tr>
      <xsl:apply-templates select="basetype" mode="body"/>
    </table>
</xsl:template>

<xsl:template match="basetype" mode="body">
  <xsl:choose>
    <xsl:when test="count(definition)=0">
      <tr>
        <td>
          <code>
            <xsl:value-of select="@id"/>
          </code>
        </td>
        <td>
          <a>
            <xsl:attribute name="name">
              <xsl:value-of select="@id"/>
            </xsl:attribute>
          </a>
          <xsl:apply-templates select="description" mode="brief"/>
        </td>
      </tr>      
    </xsl:when>
    <xsl:otherwise>
      <tr>
        <td rowspan="2">
          <code>
            <xsl:value-of select="@id"/>
          </code>
        </td>
        <td>
          <a>
            <xsl:attribute name="name">
              <xsl:value-of select="@id"/>
            </xsl:attribute>
          </a>
          <xsl:apply-templates select="description" mode="brief"/>
        </td>
      </tr>      
      <tr>
        <td>
          <pre>
            <xsl:apply-templates select="definition"/>
          </pre>          
        </td>
      </tr>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="description">
  <xsl:apply-templates/>
  <p/>
</xsl:template>

<xsl:template match="description" mode="brief">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="fieldlink">
  <a>
    <xsl:attribute name="href">#<xsl:value-of select="@struct"/>.<xsl:value-of select="@id"/></xsl:attribute>
    <xsl:choose>
      <xsl:when test=".=''">
        <code>
          <xsl:value-of select="@id"/>
        </code>        
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
  </a>
</xsl:template>

<xsl:template match="paramlink">
  <a>
    <xsl:attribute name="href">#<xsl:value-of select="ancestor::function/@id|ancestor::event/@id"/>.<xsl:value-of select="@id"/>
    </xsl:attribute>
    <xsl:choose>
      <xsl:when test=".=''">
        <code>
          <xsl:value-of select="@id"/>
        </code>        
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
  </a>
</xsl:template>

<xsl:template match="eventlink|errorlink|typelink|datalink|functionlink">
  <a>
    <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
    <xsl:choose>
      <xsl:when test=".=''">
        <code>
          <xsl:value-of select="@id"/>
        </code>        
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
  </a>
</xsl:template>

<xsl:template match="functionphaselist">
  <xsl:variable name="phase" select="@phase"/>
  <ul>
    <xsl:for-each select="/specification/functionsection/category/function[@phase=$phase and count(@hide)=0]">   
      <li>
        <a>
          <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
          <b>
            <xsl:value-of select="@id"/>
          </b>
        </a>
      </li>
    </xsl:for-each>
  </ul>
</xsl:template>

<xsl:template match="eventphaselist">
  <xsl:variable name="phase" select="@phase"/>
  <ul>
    <xsl:for-each select="//eventsection/event[@phase=$phase]">   
      <li>
        <a>
          <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
          <b>
            <xsl:value-of select="@id"/>
          </b>
        </a>
      </li>
    </xsl:for-each>
  </ul>
</xsl:template>

<xsl:template match="externallink">
  <a>
    <xsl:attribute name="href">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
    <xsl:value-of select="."/>
  </a>
</xsl:template>

<xsl:template match="internallink">
  <a>
    <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
    <xsl:value-of select="."/>
  </a>
</xsl:template>

<xsl:template match="parameters" mode="body">
  <p/>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="3" align="center" class="tableHeader">
        Parameters
      </td>
    </tr>
    <tr bgcolor="#EEEEFF">
      <td>
        Name
      </td>
      <td>
        Type
      </td>
      <td>
        Description
      </td>
    </tr>
    <xsl:apply-templates select="param[count(jclass/@method)=0]" mode="body"/>
  </table>
</xsl:template>

<xsl:template match="param" mode="body">
  <tr>
    <td>
      <code>
        <xsl:attribute name="id">
          <xsl:value-of select="../../@id"/>.<xsl:value-of select="@id"/>
        </xsl:attribute>
        <xsl:value-of select="@id"/>
      </code>
    </td>
    <td>
      <code>
        <xsl:apply-templates select="child::*[position()=1]" mode="link"/>
      </code>
    </td>
    <td>
      <xsl:apply-templates select="description" mode="brief"/>
      <xsl:if test="count(ancestor::function)=1">
        <xsl:apply-templates select="child::*[position()=1]" mode="funcdescription"/>
      </xsl:if>
    </td>
  </tr>
</xsl:template>

<xsl:template match="capabilities">
  <p/>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center" class="tableHeader">
        Capabilities
      </td>
    </tr>
    <xsl:choose>
      <xsl:when test="count(required)=0">
        <tr>
          <td colspan="2">
            <b>Required Functionality</b>
          </td>
        </tr>
      </xsl:when>
      <xsl:otherwise>
        <tr>
          <td colspan="2">
            <b>Optional Functionality:</b> might not be implemented for all
            virtual machines. 
            <xsl:choose>
              <xsl:when test="count(required)=1">
                The following capability 
              </xsl:when>
              <xsl:otherwise>
                One of the following capabilities
              </xsl:otherwise>
            </xsl:choose>
            (as returned by 
            <a href="#GetCapabilities"><code>GetCapabilities</code></a>)
            must be true to use this      
            <xsl:choose>
              <xsl:when test="ancestor::function">
                function.
              </xsl:when>
              <xsl:otherwise>
                event.
              </xsl:otherwise>
            </xsl:choose>
          </td>
        </tr>
        <tr bgcolor="#EEEEFF">
          <td >
            Capability
          </td>
          <td>
            Effect
          </td>
        </tr>
        <xsl:apply-templates select="required"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:if test="count(capability)!=0">
      <tr bgcolor="#CCCCFF">
        <td colspan="2" align="center">
          Optional Features
        </td>
      </tr>
      <xsl:if test="count(required)=0">
        <tr bgcolor="#EEEEFF">
          <td >
            Capability
          </td>
          <td>
            Effect
          </td>
        </tr>
      </xsl:if>
      <xsl:apply-templates select="capability"/>
    </xsl:if>
  </table>
</xsl:template>

<xsl:template match="eventcapabilities">
  <p/>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center" class="tableHeader">
        Capabilities
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Required Functionality</b>
      </td>
    </tr>
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center">
        Event Enabling Capabilities
      </td>
    </tr>
    <tr bgcolor="#EEEEFF">
      <td >
        Capability
      </td>
      <td>
        Events
      </td>
    </tr>
    <xsl:for-each select="//capabilityfield">
      <xsl:variable name="capa" select="@id"/>
      <xsl:variable name="events" select="//event[capabilities/required/@id=$capa]"/>
      <xsl:if test="count($events)">
        <tr>
          <td>
            <a>
              <xsl:attribute name="href">#jvmtiCapabilities.<xsl:value-of select="@id"/>
              </xsl:attribute>
              <code>
                <xsl:value-of select="@id"/>
              </code>
            </a>
          </td>
          <td>
            <xsl:for-each select="$events">
              <a>
                <xsl:attribute name="href">#<xsl:value-of select="@id"/>
                </xsl:attribute>
                <code>
                  <xsl:value-of select="@id"/>
                </code>
              </a>
              <br/>
            </xsl:for-each>
          </td>
        </tr>
      </xsl:if>
    </xsl:for-each>
  </table>
</xsl:template>

<xsl:template match="capability|required">
  <tr>
    <td>
      <a>
        <xsl:attribute name="href">#jvmtiCapabilities.<xsl:value-of select="@id"/>
        </xsl:attribute>
        <code>
          <xsl:value-of select="@id"/>
        </code>
      </a>
    </td>
    <td>
      <xsl:choose>
        <xsl:when test=".=''">
          <xsl:variable name="desiredID" select="@id"/>
          <xsl:for-each select="//capabilityfield[@id=$desiredID]">
            <xsl:apply-templates select="description" mode="brief"/>
          </xsl:for-each>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates/>
        </xsl:otherwise>
      </xsl:choose>
    </td>
  </tr>
</xsl:template>

<xsl:template match="function" mode="errors">
  <xsl:variable name="haserrors">
    <xsl:apply-templates select="capabilities/required" mode="haserrors"/>
    <xsl:apply-templates select="errors/error" mode="haserrors"/>
    <xsl:apply-templates select="parameters/param" mode="haserrors"/>
  </xsl:variable>
  <p/>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center" class="tableHeader">
        Errors
      </td>
    </tr>
    <xsl:choose>
      <xsl:when test="contains($haserrors,'yes')">
        <tr>
          <td colspan="2">
            This function returns either a 
            <a href="#universal-error">universal error</a> 
            or one of the following errors
          </td>
        </tr>
        <tr bgcolor="#EEEEFF">
          <td>
            Error
          </td>
          <td>
            Description
          </td>
        </tr>
        <xsl:apply-templates select="capabilities/required" mode="errors"/>
        <xsl:apply-templates select="errors/error"/>
        <xsl:apply-templates select="parameters/param" mode="errors"/>
      </xsl:when>
      <xsl:otherwise>
        <tr>
          <td colspan="2">
            This function returns a 
            <a href="#universal-error">universal error</a>
          </td>
        </tr>
      </xsl:otherwise>
    </xsl:choose>
  </table>
</xsl:template>

<xsl:template match="required" mode="haserrors">
  yes
</xsl:template>

<xsl:template match="required" mode="errors">
  <tr>
    <td>
      <a href="#JVMTI_ERROR_MUST_POSSESS_CAPABILITY">
        <code>
          JVMTI_ERROR_MUST_POSSESS_CAPABILITY
        </code>
      </a>
    </td>
    <td>
      The environment does not possess the capability
      <a>
        <xsl:attribute name="href">#jvmtiCapabilities.<xsl:value-of select="@id"/></xsl:attribute>
        <code>
          <xsl:value-of select="@id"/>
        </code>        
      </a>.
      Use <a href="#AddCapabilities"><code>AddCapabilities</code></a>.
    </td>
  </tr>
</xsl:template>

<xsl:template match="param" mode="haserrors">
  <xsl:apply-templates mode="haserrors"/>
</xsl:template>

<xsl:template match="param" mode="errors">
  <xsl:apply-templates select="." mode="errors1"/>
  <xsl:apply-templates select="." mode="errors2"/>
</xsl:template>

<xsl:template match="param" mode="errors1">
  <xsl:variable name="haserrors">
    <xsl:apply-templates mode="haserrors"/>
  </xsl:variable>
  <xsl:if test="contains($haserrors,'yes')!=0">
    <xsl:variable name="erroridraw">
      <xsl:apply-templates mode="errorid"/>
    </xsl:variable>
    <xsl:variable name="errorid" select="normalize-space($erroridraw)"/>
    <tr>
      <td>
        <a>
          <xsl:attribute name="href">#<xsl:value-of select="$errorid"/></xsl:attribute>
          <code>
            <xsl:value-of select="$errorid"/>
          </code>
        </a>
      </td>
      <td>
        <xsl:apply-templates mode="errordesc">
          <xsl:with-param name="id" select="@id"/>
        </xsl:apply-templates>
      </td>
    </tr>
  </xsl:if>
</xsl:template>
 
<xsl:template match="param" mode="errors2">
  <xsl:variable name="haserrors2">
    <xsl:apply-templates mode="haserrors2"/>
  </xsl:variable>
  <xsl:if test="contains($haserrors2,'yes')!=0">
    <xsl:variable name="erroridraw2">
      <xsl:apply-templates mode="errorid2"/>
    </xsl:variable>
    <xsl:variable name="errorid2" select="normalize-space($erroridraw2)"/>
    <tr>
      <td>
        <a>
          <xsl:attribute name="href">#<xsl:value-of select="$errorid2"/></xsl:attribute>
          <code>
            <xsl:value-of select="$errorid2"/>
          </code>
        </a>
      </td>
      <td>
        <xsl:apply-templates mode="errordesc2">
          <xsl:with-param name="id" select="@id"/>
        </xsl:apply-templates>
      </td>
    </tr>
  </xsl:if>
</xsl:template>
 
<xsl:template match="description" mode="haserrors">
</xsl:template>

<xsl:template match="description" mode="errorid">
</xsl:template>

<xsl:template match="description" mode="errordesc">
</xsl:template>

<xsl:template match="jmethodID|jfieldID|jframeID|jrawMonitorID|jthread|jthreadGroup|jobject|enum|jlocation" mode="haserrors">
  yes
</xsl:template>

<xsl:template match="jclass" mode="haserrors">
  <xsl:if test="count(@method)=0">
    yes
  </xsl:if>
</xsl:template>

<xsl:template match="description|jclass|jfieldID|jrawMonitorID|
                    jthreadGroup|jobject|enum|jlocation|jvalue|jint|jlong|jfloat|jdouble|jboolean|
                    char|uchar|void|varargs|struct|
                    ptrtype|outptr|allocbuf|allocallocbuf|inptr|inbuf|outbuf|vmbuf|agentbuf" mode="haserrors2">
</xsl:template>

<xsl:template match="jmethodID" mode="haserrors2">
  <xsl:if test="count(@native)=1 and contains(@native,'error')">
    yes
  </xsl:if>
</xsl:template>

<xsl:template match="jthread" mode="haserrors2">
  <xsl:if test="count(@started)=0 or contains(@started,'yes') or @started=''">
    yes
  </xsl:if>
</xsl:template>

<xsl:template match="jframeID" mode="haserrors2">
    yes
</xsl:template>

<xsl:template match="description" mode="errorid2">
</xsl:template>

<xsl:template match="description" mode="errordesc2">
</xsl:template>

<xsl:template match="jmethodID" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_METHODID</xsl:text>
</xsl:template>

<xsl:template match="jmethodID" mode="errorid2">
    <xsl:text>JVMTI_ERROR_NATIVE_METHOD</xsl:text>
</xsl:template>

<xsl:template match="jmethodID" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a jmethodID.</xsl:text>
</xsl:template>

<xsl:template match="jmethodID" mode="errordesc2">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is a native method.</xsl:text>
</xsl:template>

<xsl:template match="jfieldID" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_FIELDID</xsl:text>
</xsl:template>

<xsl:template match="jfieldID" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a jfieldID.</xsl:text>
</xsl:template>

<xsl:template match="jframeID" mode="errorid">
  <xsl:text>JVMTI_ERROR_ILLEGAL_ARGUMENT</xsl:text>
</xsl:template>

<xsl:template match="jframeID" mode="errorid2">
  <xsl:text>JVMTI_ERROR_NO_MORE_FRAMES</xsl:text>
</xsl:template>

<xsl:template match="jframeID" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is less than zero.</xsl:text>
</xsl:template>

<xsl:template match="jframeID" mode="errordesc2">
  <xsl:param name="id"/>
  <xsl:text>There are no stack frames at the specified </xsl:text>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text>.</xsl:text>
</xsl:template>

<xsl:template match="jrawMonitorID" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_MONITOR</xsl:text>
</xsl:template>

<xsl:template match="jrawMonitorID" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a jrawMonitorID.</xsl:text>
</xsl:template>

<xsl:template match="jclass" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_CLASS</xsl:text>
</xsl:template>

<xsl:template match="jclass" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a class object or the class has been unloaded.</xsl:text>
</xsl:template>

<xsl:template match="jthread" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_THREAD</xsl:text>
</xsl:template>

<xsl:template match="jthread" mode="errorid2">
  <xsl:text>JVMTI_ERROR_THREAD_NOT_ALIVE</xsl:text>
</xsl:template>

<xsl:template match="jthread" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a thread object.</xsl:text>
</xsl:template>

<xsl:template match="jthread" mode="errordesc2">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not live (has not been started or is now dead).</xsl:text>
</xsl:template>

<xsl:template match="jthreadGroup" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_THREAD_GROUP</xsl:text>
</xsl:template>

<xsl:template match="jthreadGroup" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a thread group object.</xsl:text>
</xsl:template>

<xsl:template match="jobject" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_OBJECT</xsl:text>
</xsl:template>

<xsl:template match="jobject" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not an object.</xsl:text>
</xsl:template>

<xsl:template match="enum" mode="errorid">
  <xsl:choose>
    <xsl:when test=".='jvmtiEvent'">
      <xsl:text>JVMTI_ERROR_INVALID_EVENT_TYPE</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>JVMTI_ERROR_ILLEGAL_ARGUMENT</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="enum" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a </xsl:text>
  <xsl:value-of select="."/>
  <xsl:text>.</xsl:text>
</xsl:template>

<xsl:template match="jlocation" mode="errorid">
  <xsl:text>JVMTI_ERROR_INVALID_LOCATION</xsl:text>
</xsl:template>

<xsl:template match="jlocation" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is not a valid location.</xsl:text>
</xsl:template>

<xsl:template match="jint" mode="haserrors">
  <xsl:if test="count(@min)=1">
    yes
  </xsl:if>
</xsl:template>

<xsl:template match="jint" mode="errorid">
  <xsl:text>JVMTI_ERROR_ILLEGAL_ARGUMENT</xsl:text>
</xsl:template>

<xsl:template match="jint" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is less than </xsl:text>
  <code><xsl:value-of select="@min"/></code>
  <xsl:text>.</xsl:text>
</xsl:template>

<xsl:template match="jvalue|jlong|jfloat|jdouble|jboolean|char|uchar|void|varargs|struct" mode="haserrors">
</xsl:template>

<xsl:template match="jvalue|jlong|jfloat|jdouble|jboolean|char|uchar|void|varargs|struct" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:message terminate="yes">
    attempt to get error description for <xsl:apply-templates select="." mode="paramlink"/>
  </xsl:message>
</xsl:template>

<xsl:template match="ptrtype|outptr|allocbuf|allocallocbuf|inptr|inbuf|outbuf|vmbuf|agentbuf" mode="haserrors">
  <xsl:if test="count(nullok)=0">
    yes
  </xsl:if>
</xsl:template>

<xsl:template match="ptrtype|outptr|allocbuf|allocallocbuf|inptr|inbuf|outbuf|vmbuf|agentbuf" mode="errorid">
  <xsl:text>JVMTI_ERROR_NULL_POINTER</xsl:text>
</xsl:template>

<xsl:template match="ptrtype|outptr|allocbuf|allocallocbuf|inptr|inbuf|outbuf|vmbuf|agentbuf" mode="errordesc">
  <xsl:param name="id"/>
  <xsl:if test="count(nullok)=1">
    <xsl:message terminate="yes">
      attempt to get error description in null ok case for <xsl:apply-templates select="." mode="paramlink"/>
    </xsl:message>
  </xsl:if>
  <xsl:apply-templates select="." mode="paramlink"/>
  <xsl:text> is </xsl:text>
  <code>NULL</code>
  <xsl:text>.</xsl:text>
</xsl:template>

<xsl:template match="jmethodID|jfieldID|jframeID|jrawMonitorID|jint|jclass|jthread|jthreadGroup|jobject|enum|jlocation|ptrtype|outptr|allocbuf|allocallocbuf|inptr|inbuf|outbuf|vmbuf|agentbuf" mode="paramlink">
  <a>
    <xsl:attribute name="href">#<xsl:value-of select="ancestor::function/@id|ancestor::event/@id"/>.<xsl:value-of select="ancestor::param/@id"/>
    </xsl:attribute>
    <code>
      <xsl:value-of select="ancestor::param/@id"/>
    </code>        
  </a>
</xsl:template>

<xsl:template match="error" mode="haserrors">
  yes
</xsl:template>

<xsl:template match="error">
  <tr>
    <td>
      <a>
        <xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
        <code>
          <xsl:value-of select="@id"/>
        </code>
      </a>
    </td>
    <td>
      <xsl:apply-templates/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="errorsection">
  <p/>
  <hr noshade="noshade" size="3"/>
  <h2>
    Errors
  </h2>
  <p/>
  <xsl:apply-templates select="intro"/>
  <p/>
  <xsl:apply-templates select="errorcategory"/>
  <p/>
</xsl:template>

<xsl:template match="datasection">
  <p/>
  <hr noshade="noshade" size="3"/>
  <h2>
    Data Types
  </h2>
  <p/>
  <xsl:apply-templates select="intro"/>
  <xsl:apply-templates select="basetypes"/>
  <p/>
  <a name="StructureTypeDefinitions"></a>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center" class="tableHeader">
        Structure Type Definitions
      </td>
    </tr>
    <tr bgcolor="#EEEEFF">
      <td>
        Type
      </td>
      <td>
        Description
      </td>
    </tr>
    <xsl:apply-templates select="//typedef|//capabilitiestypedef" mode="tableentry">
      <xsl:sort select="@id"/>
    </xsl:apply-templates>
  </table>
  <p/>
  <a name="FunctionTypeDefinitions"></a>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center" class="tableHeader">
        Function Type Definitions
      </td>
    </tr>
    <tr bgcolor="#EEEEFF">
      <td>
        Type
      </td>
      <td>
        Description
      </td>
    </tr>
    <xsl:apply-templates select="//callback" mode="tableentry">
      <xsl:sort select="@id"/>
    </xsl:apply-templates>
  </table>
  <p/>
  <a name="EnumerationDefinitions"></a>
  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr bgcolor="#CCCCFF">
      <td colspan="2" align="center" class="tableHeader">
        Enumeration Definitions
      </td>
    </tr>
    <tr bgcolor="#EEEEFF">
      <td>
        Type
      </td>
      <td>
        Description
      </td>
    </tr>
    <xsl:apply-templates select="//constants[@kind='enum']" mode="tableentry">
      <xsl:sort select="@id"/>
    </xsl:apply-templates>
  </table>
  <p/>
</xsl:template>

<xsl:template match="errorcategory">
  <h3>
    <xsl:attribute name="id">
      <xsl:value-of select="@id"/>
    </xsl:attribute>
    <xsl:value-of select="@label"/>
  </h3>
  <xsl:apply-templates select="intro"/>
  <p/>
  <dl>
    <xsl:apply-templates select="errorid"/>
  </dl>
  <p/>
</xsl:template>

<xsl:template match="errorid">
  <dt>
    <code>
      <xsl:attribute name="id">
        <xsl:value-of select="@id"/>
      </xsl:attribute>
      <xsl:value-of select="@id"/> (<xsl:value-of select="@num"/>)
    </code>
  </dt>
  <dd>
    <xsl:apply-templates/>
    <p/>
  </dd>
</xsl:template>

<xsl:template match="changehistory">
    <p/><hr noshade="noshade" size="3"/>
    <h2>Change History</h2>
    Last update: <xsl:value-of select="@update"/><br/>
    File version: <xsl:value-of select="@version"/><br/>
    Version: <xsl:call-template name="showversion"/>
    <p/>
    <table border="1" cellpadding="3" cellspacing="0" width="100%">
      <tr bgcolor="#EEEEFF">
        <td>
          <b>Version</b><br/>
          <b>Date</b>
        </td>
        <td>
          <b>Changes</b>
        </td>
      </tr>
      <xsl:apply-templates select="change"/>
    </table>
</xsl:template>

<xsl:template match="change">
  <tr>
    <td>
      <xsl:if test="count(@version)">
        <b>
          <xsl:value-of select="@version"/>
        </b>
        <br/>
      </xsl:if>
      <xsl:value-of select="@date"/>
    </td>
    <td>
      <xsl:apply-templates/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="intro">
  <xsl:if test="@id!=''">
    <xsl:choose>
      <xsl:when test="@label!=''">
        <h4>
          <xsl:attribute name="id">
            <xsl:value-of select="@id"/>
          </xsl:attribute>
          <xsl:value-of select="@label"/>
        </h4>
      </xsl:when>
      <xsl:otherwise>
        <a>
          <xsl:attribute name="name">
            <xsl:value-of select="@id"/>
          </xsl:attribute>
        </a>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="jvmdi">
  <xsl:if test=".!='' and $development = 'Show'">
    <p style="color: green">
      JVMDI difference:
      <xsl:apply-templates/>
    </p>
  </xsl:if>
</xsl:template>

<xsl:template match="issue">
  <xsl:if test="$development = 'Show'">
    <p style="color: red">
    To be resolved:
      <xsl:apply-templates/>
    </p>
  </xsl:if>
</xsl:template>

<xsl:template match="rationale">
  <p style="color: purple">
  Rationale:
      <xsl:apply-templates/>
  </p>
</xsl:template>

<xsl:template match="todo">
  <xsl:if test="$development = 'Show'">
    <p style="color: green">
    To do:
      <xsl:apply-templates/>
    </p>
  </xsl:if>
</xsl:template>

<xsl:template match="elide">
</xsl:template>

<xsl:template match="b">
  <b>
  <xsl:apply-templates/>
  </b>
</xsl:template>

<xsl:template match="example">
  <blockquote>
    <pre>
      <xsl:apply-templates/>
    </pre>
  </blockquote>
</xsl:template>

<xsl:template match="dl">
  <dl>
    <xsl:apply-templates/>
  </dl>
</xsl:template>

<xsl:template match="dt">
  <dt>
    <xsl:apply-templates/>
  </dt>
</xsl:template>

<xsl:template match="dd">
  <dd>
    <xsl:apply-templates/>
  </dd>
</xsl:template>

<xsl:template match="p">
  <p>
    <xsl:apply-templates/>
  </p>
</xsl:template>

<xsl:template match="br">
  <br>
    <xsl:apply-templates/>
  </br>
</xsl:template>

<xsl:template match="ul">
  <ul>
    <xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute>
    <xsl:apply-templates/>
  </ul>
</xsl:template>

<xsl:template match="li">
  <li>
    <xsl:apply-templates/>
  </li>
</xsl:template>

<xsl:template match="code">
  <code>
    <xsl:apply-templates/>
  </code>
</xsl:template>

<xsl:template match="tm">
  <xsl:apply-templates/>
  <sup style="font-size: xx-small">
    <xsl:text>TM</xsl:text>
  </sup>
  <xsl:text>&#032;</xsl:text>
</xsl:template>

<xsl:template match="b">
  <b>
    <xsl:apply-templates/>
  </b>
</xsl:template>

<xsl:template match="i">
  <i>
    <xsl:apply-templates/>
  </i>
</xsl:template>

<xsl:template match="space">
  <xsl:text>&#032;</xsl:text>
</xsl:template>

<xsl:template match="jvmti">
  <xsl:text>JVM</xsl:text><small style="font-size: xx-small">&#160;</small><xsl:text>TI</xsl:text>
</xsl:template>


</xsl:stylesheet>
