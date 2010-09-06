<?xml version="1.0"?> 
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="jvmtiEnter.xsl"/>

<xsl:output method="text" indent="no" omit-xml-declaration="yes"/>

<xsl:param name="trace"></xsl:param>
<xsl:param name="interface"></xsl:param>

<xsl:template match="specification">
  <xsl:call-template name="sourceHeader"/>
  <xsl:text>
# include "incls/_precompiled.incl"
# include "incls/_jvmdiEnter.cpp.incl"

</xsl:text>

  <xsl:if test="$trace='Trace'">
    <xsl:text>
#ifdef JVMTI_TRACE
</xsl:text>
  </xsl:if>

  <xsl:apply-templates select="functionsection"/>

  <xsl:if test="$trace='Trace'">
    <xsl:text>
#endif /*JVMTI_TRACE */
</xsl:text>
  </xsl:if>

</xsl:template>

<xsl:template match="functionsection">
  <xsl:text>
extern "C" {

</xsl:text>
  <xsl:apply-templates select="category" mode="wrapper"/>    
  <xsl:text>
} /* end extern "C" */

// JVMDI API functions
struct JVMDI_Interface_1_ jvmdi</xsl:text>
  <xsl:value-of select="$trace"/>
  <xsl:text>_Interface = {
</xsl:text>

  <xsl:call-template name="fillFuncStruct">
    <xsl:with-param name="funcs" select="category/function[origin='jvmdi']"/>
    <xsl:with-param name="index" select="1"/>
  </xsl:call-template>

  <xsl:text>
};
</xsl:text>
</xsl:template>

<xsl:template match="category" mode="wrapper">
  <xsl:text>
  //
  // </xsl:text><xsl:value-of select="@label"/><xsl:text> functions
  // 
</xsl:text>
  <xsl:apply-templates select="function[origin='jvmdi']"/>
</xsl:template>

<xsl:template match="function">
  <xsl:text>
static jvmdiError JNICALL
</xsl:text>
  <xsl:apply-templates select="." mode="functionid"/>
  <xsl:text>(</xsl:text>
  <xsl:apply-templates select="parameters" mode="signaturenoleadcomma"/>
  <xsl:text>) {</xsl:text>
  <xsl:apply-templates select="." mode="traceSetUp"/>
  <xsl:text>
  Thread* this_thread = (Thread*)ThreadLocalStorage::thread();</xsl:text>
  <xsl:apply-templates select="." mode="transition"/>
  <xsl:choose>
    <xsl:when test="count(@impl)=1 and contains(@impl,'unimpl')">
      <xsl:text>  return JVMDI_ERROR_NOT_IMPLEMENTED;
</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>  JvmtiEnv* jvmti_env = JvmtiEnv::jvmti_env_for_jvmdi();
  if (jvmti_env == NULL) {
</xsl:text>
    <xsl:if test="$trace='Trace'">
      <xsl:text>    if (trace_flags) {
          tty->print_cr("JVMTI [%s] %s %s  - JVMDI not initialized",  curr_thread_name, func_name, 
                    JvmtiUtil::error_name(JVMDI_ERROR_ACCESS_DENIED));
    }
</xsl:text>
    </xsl:if>
    <xsl:text>    return JVMDI_ERROR_ACCESS_DENIED;
  }
</xsl:text>
      <xsl:apply-templates select="parameters" mode="dochecks"/>
      <xsl:apply-templates select="." mode="traceBefore"/>
      <xsl:text>  jvmdiError err = jvmti_env-&gt;</xsl:text>
      <xsl:if test="count(@hide)=1">
        <xsl:value-of select="@hide"/>
      </xsl:if>
      <xsl:value-of select="@id"/>
      <xsl:text>(</xsl:text>
      <xsl:apply-templates select="parameters" mode="HotSpotValue"/>
      <xsl:text>);
</xsl:text>
      <xsl:apply-templates select="." mode="traceAfter"/>
  <xsl:text>  return err;
</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:text>}

</xsl:text>
</xsl:template>

<!-- override to check for jvmdiid and prefix with jvmdi_ -->
<xsl:template match="function" mode="functionid">
  <xsl:text>jvmdi</xsl:text>
  <xsl:value-of select="$trace"/>
  <xsl:text>_</xsl:text>
  <xsl:choose>
    <xsl:when test="count(@jvmdiid)=1">
      <xsl:value-of select="@jvmdiid"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="@id"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!-- remove jthread parameters that are jthread/frameID pairs.
     since the jthread was added in JVMTI.
-->
<xsl:template match="param" mode="signature">
  <xsl:param name="comma"/>
  <xsl:variable name="id" select="@id"/>
  <xsl:for-each select="child::*[position()=1]">
    <xsl:if test="count(@frame)=0">
      <xsl:apply-templates select="." mode="signature"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="$id"/>
      <xsl:value-of select="$comma"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>


<!-- override to check against class and allow OBSOLETE_METHOD_ID -->
<xsl:template match="jmethodID" mode="dochecks">
  <xsl:param name="name"/>
  <xsl:text>  methodOop method_oop = (methodOop)NULL;&#xA;</xsl:text>
  <xsl:text>  if (</xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:text> != OBSOLETE_METHOD_ID) {&#xA;</xsl:text>
  <xsl:text>    if (!jniIdSupport::is_method(k_oop, </xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:text>)) {&#xA;</xsl:text>
  <xsl:apply-templates select=".." mode="traceError">     
    <xsl:with-param name="err">JVMDI_ERROR_INVALID_METHODID</xsl:with-param>
    <xsl:with-param name="comment"></xsl:with-param>
    <xsl:with-param name="extraValue"></xsl:with-param>
  </xsl:apply-templates>
  <xsl:text>&#xA;</xsl:text>
  <xsl:text>    }&#xA;</xsl:text>
  <xsl:text>    method_oop = jniIdSupport::to_method_oop(</xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:text>);&#xA;</xsl:text>
  <xsl:text>  }&#xA;</xsl:text>
</xsl:template>


<!-- override to add class checking -->
<xsl:template match="jfieldID" mode="dochecks">
  <xsl:param name="name"/>
  <xsl:text>  ResourceMark rm_fdesc(current_thread);&#xA;</xsl:text>
  <xsl:text>  fieldDescriptor fdesc;&#xA;</xsl:text>
  <xsl:text>  if (!JvmtiEnv::get_field_descriptor(k_oop, </xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:text>, &amp;fdesc)) {&#xA;</xsl:text>
  <xsl:apply-templates select=".." mode="traceError">     
    <xsl:with-param name="err">JVMDI_ERROR_INVALID_FIELDID</xsl:with-param>
  </xsl:apply-templates>
  <xsl:text>&#xA;</xsl:text>
  <xsl:text>  }&#xA;</xsl:text>
</xsl:template>


<!-- override to switch name -->
<xsl:template match="jframeID" mode="HotSpotName">
  <xsl:param name="name"/>
  <xsl:text>real_depth</xsl:text>
</xsl:template>


<!-- override to add type cast -->
<xsl:template match="param" mode="HotSpotValue">
  <xsl:param name="comma"/>
  <xsl:variable name="type" select="child::*[position()=1]"/>
  <xsl:variable name="result">
    <xsl:apply-templates select="$type" mode="HotSpotValue">
      <xsl:with-param name="name" select="@id"/>
    </xsl:apply-templates>
  </xsl:variable>
  <xsl:if test="string-length($result)!=0">
    <xsl:if test="not(contains('varargs jint jboolean jfloat jdouble void char jthread jclass',name($type)))">
      <xsl:text>(</xsl:text>
      <xsl:apply-templates select="$type" mode="HotSpotType"/>
      <xsl:text>)</xsl:text>      
    </xsl:if>
    <xsl:value-of select="$result"/>
    <xsl:value-of select="$comma"/>    
  </xsl:if>
</xsl:template>

<!-- override these to hide thread param on frame -->

<xsl:template match="jthread" mode="dochecks">
  <xsl:param name="name"/>
  <!-- If we convert and test threads -->
  <xsl:if test="(count(@impl)=0 or not(contains(@impl,'noconvert'))) and count(@frame)=0">
    <xsl:text>  JavaThread* java_thread;
</xsl:text>
    <xsl:apply-templates select="." mode="dochecksbody">
      <xsl:with-param name="name" select="$name"/>
    </xsl:apply-templates>
  </xsl:if>
</xsl:template>

<xsl:template match="jthread" mode="traceInFormat">
  <xsl:param name="name"/>
  <!-- If we convert and test threads -->
  <xsl:if test="(count(@impl)=0 or not(contains(@impl,'noconvert'))) and count(@frame)=0">
    <xsl:text> </xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>=%s</xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="jthread" mode="traceInValue">
  <xsl:param name="name"/>
  <!-- If we convert and test threads -->
  <xsl:if test="(count(@impl)=0 or not(contains(@impl,'noconvert'))) and count(@frame)=0">
    <xsl:text>, 
                    JvmtiTrace::safe_get_thread_name(java_thread)</xsl:text>  
  </xsl:if>
</xsl:template>

<!-- override to do thread lookup -->
<xsl:template match="jframeID" mode="dochecks">
  <xsl:param name="name"/>
  <xsl:text>  JavaThread* java_thread;
  jint real_depth;
  if (!JvmdiConvertJFrameID::get_thread_and_depth(depth, &amp;java_thread, &amp;real_depth)) {
</xsl:text>
    <xsl:apply-templates select=".." mode="traceError">     
      <xsl:with-param name="err">JVMDI_ERROR_INVALID_FRAMEID</xsl:with-param>
      <xsl:with-param name="comment"> - thread/frame not found</xsl:with-param>
    </xsl:apply-templates>
    <xsl:text>
  }
</xsl:text>
</xsl:template>

<!-- override to map normal enum to int  -->
<xsl:template match="enum" mode="traceInFormat">
  <xsl:param name="name"/>
  <xsl:text> </xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:choose>
    <xsl:when test=".='jvmtiError' or .='jvmtiEvent'">
      <xsl:text>=%d:%s</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>=%d</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="enum" mode="traceInValue">
  <xsl:param name="name"/>
  <xsl:text>, </xsl:text>
  <xsl:value-of select="$name"/>
  <xsl:choose>
    <xsl:when test=".='jvmtiError'">
      <xsl:text>, 
                    JvmtiUtil::error_name(</xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>)
</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test=".='jvmtiEvent'">
          <xsl:text>, 
                    JvmtiTrace::event_name(</xsl:text>
          <xsl:value-of select="$name"/>
          <xsl:text>)
        </xsl:text>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!-- override to rename data structures  -->
<xsl:template match="jframeID" mode="signature">
  <xsl:value-of select="name()"/>
</xsl:template>

<xsl:template match="struct" mode="signature">
  <xsl:variable name="name" select="."/>
  <xsl:variable name="tdef" select="(//typedef[@id=$name]|//capabilitiestypedef[@id=$name]|//callback[@id=$name])"/>
  <xsl:choose>
    <xsl:when test="count($tdef)=1 and count($tdef/@jvmdiid)=1">
      <xsl:value-of select="$tdef/@jvmdiid"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="."/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- override to rename data structures  -->
<xsl:template match="enum" mode="signature">
  <xsl:choose>
    <xsl:when test="string(.)='jvmtiError'">
      <xsl:text>jvmdiError</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>jint</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- override to remove const  -->
<xsl:template match="inptr|inbuf|vmbuf" mode="signature">
  <xsl:apply-templates select="child::*[position()=1]" mode="signature"/>
  <xsl:text>*</xsl:text>
</xsl:template>

<!-- override to rename jrawMonitorID  -->
<xsl:template match="jrawMonitorID" mode="signature">
  <xsl:text>JVMDI_RawMonitor</xsl:text>
</xsl:template>

<!-- override to rename uchar  -->
<xsl:template match="uchar" mode="signature">
  <xsl:text>jbyte</xsl:text>
</xsl:template>

</xsl:stylesheet>
