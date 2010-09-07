/*
 * @(#)OAEPParameterSpec.java	1.4 04/06/03
 *
 * Copyright (c) 2004 Sun Microsystems, Inc. All Rights Reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.spec;

import java.math.BigInteger;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.MGF1ParameterSpec;

/** 
 * This class specifies the set of parameters used with OAEP Padding,
 * as defined in the
 * <a href="http://www.ietf.org/rfc/rfc3447.txt">PKCS #1</a>
 * standard.
 *
 * Its ASN.1 definition in PKCS#1 standard is described below:
 * <pre>
 * RSAES-OAEP-params ::= SEQUENCE {
 *   hashAlgorithm      [0] OAEP-PSSDigestAlgorithms     DEFAULT sha1,
 *   maskGenAlgorithm   [1] PKCS1MGFAlgorithms  DEFAULT mgf1SHA1,
 *   pSourceAlgorithm   [2] PKCS1PSourceAlgorithms  DEFAULT pSpecifiedEmpty
 * }
 * </pre>
 * where
 * <pre>
 * OAEP-PSSDigestAlgorithms    ALGORITHM-IDENTIFIER ::= {
 *   { OID id-sha1 PARAMETERS NULL   }|
 *   { OID id-sha256 PARAMETERS NULL }|
 *   { OID id-sha384 PARAMETERS NULL }|
 *   { OID id-sha512 PARAMETERS NULL },
 *   ...  -- Allows for future expansion --
 * }
 * PKCS1MGFAlgorithms    ALGORITHM-IDENTIFIER ::= {
 *   { OID id-mgf1 PARAMETERS OAEP-PSSDigestAlgorithms },
 *   ...  -- Allows for future expansion --
 * }
 * PKCS1PSourceAlgorithms    ALGORITHM-IDENTIFIER ::= {
 *   { OID id-pSpecified PARAMETERS OCTET STRING },
 *   ...  -- Allows for future expansion --
 * }
 * </pre>
 * <p>Note: the OAEPParameterSpec.DEFAULT uses the following:
 *     message digest  -- "SHA-1"
 *     mask generation function (mgf) -- "MGF1"
 *     parameters for mgf -- MGF1ParameterSpec.SHA1
 *     source of encoding input -- PSource.PSpecified.DEFAULT
 *
 * @see java.security.spec.MGF1ParameterSpec
 * @see PSource
 *
 * @author Valerie Peng
 *
 * @version 1.5, 06/03/04
 * @since 1.5
 */
public class OAEPParameterSpec implements AlgorithmParameterSpec
{
    /** 
     * The OAEP parameter set with all default values.
     */
    public static final OAEPParameterSpec DEFAULT = null;

    /** 
     * Constructs a parameter set for OAEP padding as defined in
     * the PKCS #1 standard using the specified message digest
     * algorithm <code>mdName</code>, mask generation function
     * algorithm <code>mgfName</code>, parameters for the mask 
     * generation function <code>mgfSpec</code>, and source of
     * the encoding input P <code>pSrc</code>.
     * 
     * @param mdName the algorithm name for the message digest. 
     * @param mgfName the algorithm name for the mask generation 
     * function.
     * @param mgfSpec the parameters for the mask generation function. 
     * If null is specified, null will be returned by getMGFParameters().
     * @param pSrc the source of the encoding input P.
     * @exception NullPointerException if <code>mdName</code>, 
     * <code>mgfName</code>, or <code>pSrc</code> is null.
     */
    public OAEPParameterSpec(String mdName, String mgfName,
        AlgorithmParameterSpec mgfSpec, PSource pSrc)
    { }

    /** 
     * Returns the message digest algorithm name. 
     *
     * @return the message digest algorithm name. 
     */
    public String getDigestAlgorithm() {
        return null;
    }

    /** 
     * Returns the mask generation function algorithm name.
     *
     * @return the mask generation function algorithm name.
     */
    public String getMGFAlgorithm() {
        return null;
    }

    /** 
     * Returns the parameters for the mask generation function.
     *
     * @return the parameters for the mask generation function.
     */
    public AlgorithmParameterSpec getMGFParameters() {
        return null;
    }

    /** 
     * Returns the source of encoding input P.
     *
     * @return the source of encoding input P.
     */
    public PSource getPSource() {
        return null;
    }
}
