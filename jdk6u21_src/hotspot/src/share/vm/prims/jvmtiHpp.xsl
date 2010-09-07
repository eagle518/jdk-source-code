<?xml version="1.0"?> 
<!--
 Copyright (c) 2002, 2009, Oracle and/or its affiliates. All rights reserved.
 ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.


















  
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="jvmtiLib.xsl"/>

<xsl:output method="text" indent="no" omit-xml-declaration="yes"/>

<xsl:template match="/">
  <xsl:apply-templates select="specification"/>
</xsl:template>

<xsl:template match="specification">
  <xsl:call-template name="includeHeader"/>
  <xsl:text>
    
enum {
    JVMTI_INTERNAL_CAPABILITY_COUNT = </xsl:text>
  <xsl:value-of select="count(//capabilityfield)"/>
  <xsl:text>
};


class JvmtiEnv : public JvmtiEnvBase {

private:
    
    JvmtiEnv(jint version);
    ~JvmtiEnv();

public:

    static JvmtiEnv* create_a_jvmti(jint version);

</xsl:text>
  <xsl:apply-templates select="functionsection"/>
  <xsl:text>
};
</xsl:text>
</xsl:template>

<xsl:template match="functionsection">
  <xsl:apply-templates select="category"/>
</xsl:template>

<xsl:template match="category">
  <xsl:text>
  // </xsl:text><xsl:value-of select="@label"/><xsl:text> functions
</xsl:text>
  <xsl:apply-templates select="function[not(contains(@impl,'unimpl'))]"/>
</xsl:template>

<xsl:template match="function">
  <xsl:text>    jvmtiError </xsl:text>
  <xsl:if test="count(@hide)=1">
    <xsl:value-of select="@hide"/>
  </xsl:if>
  <xsl:value-of select="@id"/>
  <xsl:text>(</xsl:text>
  <xsl:apply-templates select="parameters" mode="HotSpotSig"/>
  <xsl:text>);
</xsl:text>
</xsl:template>

</xsl:stylesheet>
