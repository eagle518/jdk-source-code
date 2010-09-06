/*
 * @(#)ProxyClient.java	1.28 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.io.IOException;
import java.lang.management.*;
import java.net.MalformedURLException;
import java.util.*;

import javax.management.*;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;
import static java.lang.management.ManagementFactory.*;

public class ProxyClient {
    private static Map<String, ProxyClient> cache =
			Collections.synchronizedMap(new HashMap<String, ProxyClient>());

    private boolean isDead = false;
    private String hostName;
    private int port;
    private int vmid;
    private String userName;
    private String password;
    private MBeanServerConnection server;
    private JMXConnector jmxc;
    private String url;
    private ClassLoadingMXBean    classLoadingMBean;
    private CompilationMXBean     compilationMBean;
    private MemoryMXBean          memoryMBean;
    private OperatingSystemMXBean operatingSystemMBean;
    private RuntimeMXBean         runtimeMBean;
    private ThreadMXBean          threadMBean;

    private com.sun.management.OperatingSystemMXBean sunOperatingSystemMXBean;

    private static final String CREDENTIALS = "jmx.remote.credentials";

    private List<MemoryPoolProxy>          memoryPoolProxies;
    private List<GarbageCollectorMXBean>    garbageCollectorMBeans;

    /*  
     * Gets a proxy client with the id of a Java virtual machine
     * on the local system.
     */
    public static ProxyClient getProxyClient(int vmid, String address) 
            throws IOException {
	String key = Integer.toString(vmid);
	ProxyClient proxyClient = cache.get(key);
	if (proxyClient == null) {
	    proxyClient = new ProxyClient(vmid, address);
	    cache.put(key, proxyClient);
	}
	
	return proxyClient;
    }

    /**
     * Gets a proxy client with no user/password if the VM being monitored
     * was started with no user/password.
     *
     * This method should only be used when the VM being monitored
     * was started with 
     *    com.sun.management.jmxremote.password=false
     *
     * Otherwise, you will get authentication failure! Credentials required.
     */
    public static ProxyClient getProxyClient(String hostName, int port) 
        throws IOException {

	return getProxyClient(hostName, port, "", "");
    }

    /**
     * Gets a proxy client from a free url.
     */
    public static ProxyClient getProxyClient(JMXServiceURL url, Map map) 
        throws IOException {
	ProxyClient proxyClient = cache.get(url.toString());
	if (proxyClient == null) {
	    proxyClient = new ProxyClient(url, map);
	    cache.put(url.toString(), proxyClient);
	}
	return proxyClient;
    }

    /**
     * Gets a proxy client with user/password authentication.
     *
     * @throws SecurityException if the connection cannot be
     * made for security reasons.
     * @see javax.management.remote.JMXConnectorFactory#connect
     */
    public static ProxyClient getProxyClient(String hostName, int port, 
        String username, String password) throws IOException {

	String key = hostName+":"+port+":"+username+":"+password;
	ProxyClient proxyClient = cache.get(key);
	if (proxyClient == null) {
	    proxyClient = new ProxyClient(hostName, port, username, password);
	    cache.put(key, proxyClient);
	}
	return proxyClient;
    }
    
    public ProxyClient(JMXServiceURL jmxUrl, Map map) throws IOException {
	this.url = jmxUrl.toString();
	this.vmid = -1;
	hostName = jmxUrl.getHost();
	port = jmxUrl.getPort();
	this.jmxc = JMXConnectorFactory.connect(jmxUrl, map);
	this.server = jmxc.getMBeanServerConnection();
    }

    public String getConnectionName() {
	String name;
	if (vmid >= 0) {
	    name = Integer.toString(vmid) + "@localhost";
        } else {
	    if(url != null && (hostName == null || 
			       hostName.length() == 0 || 
			       port == 0))
		name = url;
	    else 
		name  = hostName+":"+port;
	}
	if (userName != null && userName.length() > 0) {
            name = Resources.getText("ConnectionName", userName, name);
	}
	if (isDead()) {
            name = Resources.getText("ConnectionName (disconnected)", userName, name);
	    
	}
	return name;
    }

    public String toString() {
	return getConnectionName();
    }

    public ProxyClient(int vmid, String address) throws IOException {
	this.hostName = "localhost";
	this.port = 0;
	this.vmid = vmid;
	this.userName = "";
	this.password = "";	
	
	JMXServiceURL jmxUrl = new JMXServiceURL(address);
	
	this.jmxc = JMXConnectorFactory.connect(jmxUrl);
	this.server = jmxc.getMBeanServerConnection();
    }

    public ProxyClient(String hostName, int port, 
                       String username, String password) throws IOException {

        this.hostName = hostName;
        this.port = port;
	this.vmid = -1;
	this.userName = username;
	this.password = password;

        // Create an RMI connector client and connect it to
        // the RMI connector server
        final String urlPath = "/jndi/rmi://" + hostName + ":" + port + 
                               "/jmxrmi";
        final Map<String, String[]> m = new HashMap<String, String[]>();
        final String[] credentials = new String[2];
        credentials[0] = username;
        credentials[1] = password;
        m.put(CREDENTIALS, credentials);

        try {
	    if (hostName.equals("localhost") && port == 0) {
		// Monitor self
		this.server = ManagementFactory.getPlatformMBeanServer(); 
	    } else {
		JMXServiceURL url = new JMXServiceURL("rmi", "", 0, urlPath);
		this.jmxc = JMXConnectorFactory.connect(url, m);
		this.server = jmxc.getMBeanServerConnection();
	    }
        } catch (MalformedURLException e) {
            throw new InternalError("Should not reach here");
        }

        // WORKAROUND for bug 5056632
        // Check if the access role is correct by getting a RuntimeMXBean
        getRuntimeMXBean();
    }

    public MBeanServerConnection getMBeanServerConnection() {
	return server;
    }
    
    public String getUrl() {
	return url;
    }

    public String getHostName() {
	return hostName;
    }

    public int getPort() {
	return port;
    }

    public int getVmid() {
	return vmid;
    }

    public String getUserName() {
	return userName;
    }

    public String getPassword() {
	return password;
    }

    public void disconnect() {
	if (jmxc != null) {
	    // Close MBeanServer connection
	    try {
		jmxc.close();
	    } catch (IOException e) {
		// Ignore ???
	    }
	}
    }

    /**
     * Returns the list of domains in which any MBean is 
     * currently registered.
     */
    public String[] getDomains() throws IOException {
        return server.getDomains();
    }

    /**
     * Returns a map of MBeans with ObjectName as the key and MBeanInfo value
     * of a given domain.  If domain is <tt>null</tt>, all MBeans
     * are returned.  If no MBean found, an empty map is returned.
     *
     */
    public Map<ObjectName, MBeanInfo> getMBeans(String domain) 
        throws IOException {

        ObjectName name = null;
        if (domain != null) {
            try {
                name = new ObjectName(domain + ":*");
            } catch (MalformedObjectNameException e) {
                // should not reach here
                assert(false);
            }
        }
        Set mbeans = server.queryNames(name, null);
        Map<ObjectName,MBeanInfo> result = 
            new HashMap<ObjectName,MBeanInfo>(mbeans.size());
	Iterator iterator = mbeans.iterator();
	while (iterator.hasNext()) {
	    Object object = iterator.next();
	    if (object instanceof ObjectName) {
		ObjectName o = (ObjectName)object;
		try {
		    MBeanInfo info = server.getMBeanInfo(o);
		    result.put(o, info);
		} catch (IntrospectionException e) {
		    // TODO: should log the error
		} catch (InstanceNotFoundException e) {
		    // TODO: should log the error
		} catch (ReflectionException e) {
		    // TODO: should log the error
		}
	    }
        }
        return result;
    }

    /**
     * Returns a list of attributes of a named MBean.
     *
     */
    public AttributeList getAttributes(ObjectName name, String[] attributes) 
        throws IOException {
        AttributeList list = null;
        try {
            list = server.getAttributes(name, attributes);
        } catch (InstanceNotFoundException e) {
            // TODO: A MBean may have been unregistered.
            // need to set up listener to listen for MBeanServerNotification.
        } catch (ReflectionException e) {
            // TODO: should log the error
        }
        return list;
    }

    /**
     * Set the value of a specific attribute of a named MBean. 
     */
    public void setAttribute(ObjectName name, Attribute attribute) 
        throws InvalidAttributeValueException,
               MBeanException,
               IOException {
        try {
            server.setAttribute(name, attribute);
        } catch (InstanceNotFoundException e) {
            // TODO: A MBean may have been unregistered.
        } catch (AttributeNotFoundException e) {
            assert(false);
        } catch (ReflectionException e) {
            // TODO: should log the error
        }
    }

    /**
     * Invokes an operation of a named MBean.
     *
     * @throws MBeanException Wraps an exception thrown by 
     *      the MBean's invoked method. 
     */
    public Object invoke(ObjectName name, String operationName, 
                         Object[] params, String[] signature) 
        throws IOException, MBeanException {
        Object result = null;
        try {
            result = server.invoke(name, operationName, params, signature);
        } catch (InstanceNotFoundException e) {
            // TODO: A MBean may have been unregistered.
        } catch (ReflectionException e) {
            // TODO: should log the error
        }
        return result;
    }

    public ClassLoadingMXBean getClassLoadingMXBean() throws IOException {
	if (classLoadingMBean == null) {
	    classLoadingMBean =
		newPlatformMXBeanProxy(server, CLASS_LOADING_MXBEAN_NAME,
				       ClassLoadingMXBean.class);
	}
	return classLoadingMBean;
    }

    public CompilationMXBean getCompilationMXBean() throws IOException {
	if (compilationMBean == null) {
	    compilationMBean =
		newPlatformMXBeanProxy(server, COMPILATION_MXBEAN_NAME,
				       CompilationMXBean.class);
	}
	return compilationMBean;
    }

    public Collection<MemoryPoolProxy> getMemoryPoolProxies() 
        throws IOException {

	// TODO: How to deal with changes to the list??
	if (memoryPoolProxies == null) {
            ObjectName poolName = null;
            try {
                poolName = new ObjectName(MEMORY_POOL_MXBEAN_DOMAIN_TYPE + ",*");
            } catch (MalformedObjectNameException e) {
                // should not reach here
                assert(false);
            }
            Set mbeans = server.queryNames(poolName, null);
	    if (mbeans != null) {
		memoryPoolProxies = new ArrayList<MemoryPoolProxy>();
		Iterator iterator = mbeans.iterator();
		while (iterator.hasNext()) {
		    ObjectName objName = (ObjectName) iterator.next();
	  	    MemoryPoolProxy p = new MemoryPoolProxy(this, objName);
		    memoryPoolProxies.add(p);
		}
	    }
	}
	return memoryPoolProxies;
    }	
	
    public Collection<GarbageCollectorMXBean> getGarbageCollectorMXBeans() 
        throws IOException {

	// TODO: How to deal with changes to the list??
	if (garbageCollectorMBeans == null) {
            ObjectName gcName = null;
            try {
                gcName = new ObjectName(GARBAGE_COLLECTOR_MXBEAN_DOMAIN_TYPE + ",*");
            } catch (MalformedObjectNameException e) {
                // should not reach here
                assert(false);
            }
            Set mbeans = server.queryNames(gcName, null);
	    if (mbeans != null) {
		garbageCollectorMBeans = new ArrayList<GarbageCollectorMXBean>();
		Iterator iterator = mbeans.iterator();
		while (iterator.hasNext()) {
		    ObjectName on = (ObjectName) iterator.next();
                    String name = GARBAGE_COLLECTOR_MXBEAN_DOMAIN_TYPE +
                        ",name=" + on.getKeyProperty("name");

	            GarbageCollectorMXBean mBean =
		        newPlatformMXBeanProxy(server, name,
					       GarbageCollectorMXBean.class);
			garbageCollectorMBeans.add(mBean);
		}
	    }
	}
	return garbageCollectorMBeans;
    }

    public MemoryMXBean getMemoryMXBean() throws IOException {
	if (memoryMBean == null) {
	    memoryMBean =
		newPlatformMXBeanProxy(server, MEMORY_MXBEAN_NAME,
				       MemoryMXBean.class);
	}
	return memoryMBean;
    }

    public RuntimeMXBean getRuntimeMXBean() throws IOException { 
	if (runtimeMBean == null) {
	    runtimeMBean =
		newPlatformMXBeanProxy(server, RUNTIME_MXBEAN_NAME,
				       RuntimeMXBean.class);
	}
	return runtimeMBean;
    }


    public ThreadMXBean getThreadMXBean() throws IOException { 
	if (threadMBean == null) {
	    threadMBean =
		newPlatformMXBeanProxy(server, THREAD_MXBEAN_NAME,
				       ThreadMXBean.class);
	}
	return threadMBean;
    }

    public OperatingSystemMXBean getOperatingSystemMXBean() throws IOException {
	if (operatingSystemMBean == null) {
	    operatingSystemMBean =
		newPlatformMXBeanProxy(server, OPERATING_SYSTEM_MXBEAN_NAME,
				       OperatingSystemMXBean.class);
	}
	return operatingSystemMBean;
    }

    public com.sun.management.OperatingSystemMXBean 
        getSunOperatingSystemMXBean() throws IOException {

        try {
	    ObjectName on = new ObjectName(OPERATING_SYSTEM_MXBEAN_NAME);
            if (sunOperatingSystemMXBean == null) {
                if (server.isInstanceOf(on,
                        "com.sun.management.OperatingSystemMXBean")) {
	            sunOperatingSystemMXBean =
                        newPlatformMXBeanProxy(server, 
                            OPERATING_SYSTEM_MXBEAN_NAME,
			    com.sun.management.OperatingSystemMXBean.class);
                }
 	    }
        } catch (InstanceNotFoundException e) {
             return null; 
        } catch (MalformedObjectNameException e) {
             return null; // should never reach here
        }
	return sunOperatingSystemMXBean;
    }

    public <T> T getMXBean(ObjectName objName, Class<T> interfaceClass)
        throws IOException {
        return newPlatformMXBeanProxy(server,
                                      objName.toString(),
                                      interfaceClass);
 
    } 
    public synchronized void markAsDead() {
	
	if(isDead()) return;
	
	disconnect();
	isDead = true;
	String key = null;
	if(vmid != -1)
	    key = Integer.toString(vmid);
	else
	    if(url != null)
		key = url;
	    else
		key = hostName+":"+port+":"+userName+":"+password;
	
	cache.remove(key);
    }

    public synchronized boolean isDead() {
	return isDead;
    }

    public boolean isRegistered(ObjectName name) throws IOException {
	return server.isRegistered(name);
    }
}
