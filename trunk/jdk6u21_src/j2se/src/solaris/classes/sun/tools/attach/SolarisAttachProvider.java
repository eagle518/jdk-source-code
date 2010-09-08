/*
 * @(#)SolarisAttachProvider.java	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.attach;

import com.sun.tools.attach.VirtualMachine;
import com.sun.tools.attach.VirtualMachineDescriptor;
import com.sun.tools.attach.AttachNotSupportedException;
import com.sun.tools.attach.spi.AttachProvider;
import java.io.IOException;

/*
 * An AttachProvider implementation for Solaris that use the doors
 * interface to the VM.
 */
public class SolarisAttachProvider extends HotSpotAttachProvider {

    public SolarisAttachProvider() {
    }

    public String name() {
  	return "sun";
    }

    public String type() {
	return "doors";
    }

    public VirtualMachine attachVirtualMachine(String vmid) 
	throws AttachNotSupportedException, IOException 
    {
	checkAttachPermission();

	// AttachNotSupportedException will be thrown if the target VM can be determined
        // to be not attachable. 
        testAttachable(vmid);

        return new SolarisVirtualMachine(this, vmid);
    }	

    public VirtualMachine attachVirtualMachine(VirtualMachineDescriptor vmd) 
        throws AttachNotSupportedException, IOException
    {
	if (vmd.provider() != this) {
	    throw new AttachNotSupportedException("provider mismatch");
	}
        // To avoid re-checking if the VM if attachable, we check if the descriptor
        // is for a hotspot VM - these descriptors are created by the listVirtualMachines
        // implementation which only returns a list of attachable VMs.
        if (vmd instanceof HotSpotVirtualMachineDescriptor) {
            assert ((HotSpotVirtualMachineDescriptor)vmd).isAttachable();
            checkAttachPermission();
            return new SolarisVirtualMachine(this, vmd.id());
        } else {
            return attachVirtualMachine(vmd.id());
        }
    }
    
}
