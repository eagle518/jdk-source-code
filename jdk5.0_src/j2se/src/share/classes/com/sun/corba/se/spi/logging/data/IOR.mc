;
; @(#)IOR.mc	1.7 03/12/15
;
; Copyright 2003 Sun Microsystems, Inc. All rights reserved.
; SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
;
("com.sun.corba.se.impl.logging" "IORSystemException" IOR
    (
	(INTERNAL 
	    (ORT_NOT_INITIALIZED 1 WARNING "ObjectReferenceTemplate is not initialized")
	    (NULL_POA 2 WARNING "Null POA")
	    (BAD_MAGIC 3 WARNING "Bad magic number {0} in ObjectKeyTemplate")
	    (STRINGIFY_WRITE_ERROR 4  WARNING "Error while stringifying an object reference")
	    (TAGGED_PROFILE_TEMPLATE_FACTORY_NOT_FOUND 5 WARNING "Could not find a TaggedProfileTemplateFactory for id {0}")
	    (INVALID_JDK1_3_1_PATCH_LEVEL 6 WARNING "Found a JDK 1.3.1 patch level indicator with value {0} less than JDK 1.3.1_01 value of 1")
	    (GET_LOCAL_SERVANT_FAILURE 7 FINE "Exception occurred while looking for ObjectAdapter {0} in IIOPProfileImpl.getServant"))
	(BAD_OPERATION
	    (ADAPTER_ID_NOT_AVAILABLE 1 WARNING "Adapter ID not available")
	    (SERVER_ID_NOT_AVAILABLE 2 WARNING "Server ID not available")
	    (ORB_ID_NOT_AVAILABLE 3 WARNING "ORB ID not available")
	    (OBJECT_ADAPTER_ID_NOT_AVAILABLE 4 WARNING "Object adapter ID not available"))
	(BAD_PARAM 
	    (BAD_OID_IN_IOR_TEMPLATE_LIST 1 WARNING "Profiles in IOR do not all have the same Object ID, so conversion to IORTemplateList is impossible")
	    (INVALID_TAGGED_PROFILE 2 WARNING "Error in reading IIOP TaggedProfile")
	    (BAD_IIOP_ADDRESS_PORT 3 WARNING "Attempt to create IIOPAdiress with port {0}, which is out of range"))
	(INV_OBJREF
	    (IOR_MUST_HAVE_IIOP_PROFILE 1 WARNING "IOR must have at least one IIOP profile"))))
