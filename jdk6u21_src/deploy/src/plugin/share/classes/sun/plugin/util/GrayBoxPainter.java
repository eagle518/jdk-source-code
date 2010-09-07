/*
 * @(#)GrayBoxPainter.java	1.25 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;


import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.MediaTracker;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.awt.BorderLayout;
import com.sun.deploy.util.Trace;

/**
 * GrayBoxPainter is a helper class to paint the applet gray box with
 * image and progress bar.
 */
public class GrayBoxPainter
{
    private Color boxBGColor = Color.white;
    private Color boxFGColor = Color.black;
    private boolean boxBorder = true;
    
    // Waiting message displayed in graybox
    private String waitingMessage = null;
    
    // Custom image
    private Image		customImage;
    private URL			customImageURL;
    private boolean             customImageIsCentered;

    // Applet codebase    
    private URL codebaseURL;
    
    // Applet jar URLs
    private URL jarURLs[] = new URL[0];
    private HashMap downloadInProgressMap = new HashMap();    
    
    // Flag indicate if progress bar should be enabled
    private boolean progressBarEnabled = true; 

    // Flag indicating whether ProgressListener is being used or whether
    // progress events come in from the outside
    private boolean usingProgressListener = true;

    // Applet panel
    private Container container;

    // Thread group the applet belongs    
    private ThreadGroup threadGroup;

    // Media tracker for loading image
    private MediaTracker tracker;
    
    // Error flag
    private boolean appletErrorOccurred = false;
    
    // ready flag
    private boolean animationReady = false;
    private boolean progressBarReady = false;    
    private volatile boolean paintingSuspended = false;
    private volatile boolean paintingFinished = false;


    private GrayBoxPanel m_grayboxPanel = null;
    private ErrorDelegate errorDelegate;

    /**
     * Construct a GrayBoxPainter object.
     */
    public GrayBoxPainter(Container c) {
		this.container = c;
    }
	
    /**
     * Begin painting in graybox.
     */
    public synchronized void beginPainting(ThreadGroup tg)
    {
		this.threadGroup = tg;
		this.tracker = new MediaTracker(container);		
        
		// Load custom image if available
		loadCustomImage();

		// Track progress if progress bar is enabled and custome image
                // has not been provided by user
                if (usingProgressListener) {
                    installProgressListener();
                }
		// Show animation only after graybox appears
		// more than 1 + 1/2 second. This is to avoid the flickering
		// effect during page switch or fast loading.
		//
		final GrayBoxPainter painter = this;		    
		paintingSuspended = false;
		new Thread(new Runnable(){
			public void run() 
			{
				try {
					Thread.sleep(1500L);
				} catch (Throwable e) {
				}
				finally {		
					painter.setAnimationReady();
					painter.setProgressBarReady();
					//should not repaint during quick reloading
					if (!paintingSuspended && !paintingFinished) {
					    painter.repaintGrayBox();
					}
				}
			}
		}).start();
	}
	   
    /**
     * Finish painting in graybox.
     */
    public synchronized void finishPainting()
    {
        try {
            if(this.m_grayboxPanel != null) {
		m_grayboxPanel.stop();
		container.remove(m_grayboxPanel);
		m_grayboxPanel = null;
            }

            removeProgressListener();
            paintingSuspended = false;
            paintingFinished = true;
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }

    /**
     * Show error in graybox.
     */
    public void showLoadingError()
    {      
	appletErrorOccurred = true;

	// Repaint graybox because state if changed.
	repaintGrayBox();
    }
	
    /**
     * Load custom image.
     */
    private void loadCustomImage(){
	if(customImageURL != null){                
		try {
			// try creating custom image.  If user forgot to put their
			// image to the right place - nothing will be displayed.
			
			customImage  = Toolkit.getDefaultToolkit().getImage(customImageURL); 
					tracker.addImage(customImage, 1);                
					tracker.waitForID(1);
                        // Indicate that we should immediately start displaying this image
                        // rather than painting a white region
                        setAnimationReady();
      		} catch(InterruptedException e) {
			e.printStackTrace();
		} catch (RuntimeException e) {
                    Trace.ignoredException(e);
                }
	}
    }
    
    /**
     * Indicates whether the GrayBoxPainter is supposed to use its
     * internal ProgressListener or should wait for progress updates
     * from the outside.
     */
    public void setUsingProgressListener(boolean usingProgressListener) {
        this.usingProgressListener = usingProgressListener;
    }

    /**
     * Set progress information filter.
     */
    public void setProgressFilter(URL codebase, String archives)
    {
	codebaseURL = codebase;
	
	if (archives != null)
	{
		ArrayList archiveList = new ArrayList();
		URL archiveURLs[] = new URL[0];
    
		// Figure out which JAR files still need to be loaded.
		StringTokenizer st = new StringTokenizer(archives, ",", false);
		while(st.hasMoreTokens()) {
			String tok = st.nextToken().trim();
		
			try {    
				URL url = new URL(codebase, tok);
				archiveList.add(url);
			}catch (MalformedURLException e){
				e.printStackTrace();
			} 	    
		}
		
		archiveURLs = new URL[archiveList.size()];
		
		int i = 0;
		for (Iterator iter = archiveList.iterator(); iter.hasNext(); i++){
			archiveURLs[i] = (URL) iter.next();
		}
			
		jarURLs = archiveURLs;
		numJarTotal = archiveURLs.length;
	}
    }

    /*
     * Set user-defined image, indicating whether or not it should be centered in the applet's view.
     */
	public void setCustomImageURL(URL url, boolean centered){
            // No progress to display if custom image is provided
		progressBarEnabled = false;
		customImageURL = url;
                customImageIsCentered = centered;
	}    
    
    /**
     * Sets the progress externally. 0 = beginning, 1 = complete.
     */
    public void setProgress(float progress) {
        currentProgress = (int) (progress * maximumProgress);
        GrayBoxPanel panel = m_grayboxPanel;
        repaintGrayBox();
    }

    	// Current download progress
    	private int currentProgress = 0;
    
    	// Maximum download progress
    	private int maximumProgress = 10000;
    
    	// Number of total jars
    	private int numJarTotal = 0;
    
    	// Number of jar completely loaded so far
    	private int numberOfJarLoaded = 0;

    	// ArrayList for outstanding progress sources
    	private ArrayList progressSourceFilterList = new ArrayList();   
        
    /*
     * Sets the background color for the applet's gray box.
     */
    public void setBoxBGColor(Color bgColor)
    {
        try {
		boxBGColor = bgColor;
                if (m_grayboxPanel != null){
                    m_grayboxPanel.setBgColor(bgColor);
                }
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }
     
    /*
     * Returns the background color for the applet's gray box.
     */
    public Color getBoxBGColor()
    {
		return boxBGColor;
    }

    /*
     * Sets the foreground color for the applet's gray box.
     */
    public void setBoxFGColor(Color fgColor)
    {
        try {
            boxFGColor = fgColor;
            if (m_grayboxPanel != null){
                m_grayboxPanel.setFgColor(fgColor);
            }
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }

    /*
     * Returns the foreground color for the applet's gray box.
     */
    public Color getBoxFGColor()
    {
		return boxFGColor;
    }

    /**
     * Convenience method for setting whether or not we should draw
     * the border based on the value of the "boxborder" parameter,
     * where the absence of the parameter (a null value) means it is
     * on.
     */
    public void setBoxBorder(String boxBorderParam) {
        if (boxBorderParam == null) {
            boxBorder = true;
        } else {
            boxBorder = Boolean.valueOf(boxBorderParam).booleanValue();
        }
        if (m_grayboxPanel != null) {
            m_grayboxPanel.setBoxBorder(boxBorder);
        }
    }

    /**
     * Set waiting message to be displayed in gray box.
     */
    public void setWaitingMessage(String msg)
    {
		waitingMessage = msg;
    }
 
    /**
     * Freeze the painting of the graybox.
     */
    public synchronized void suspendPainting()
    {
        try {
            if (!paintingSuspended) {
                if (m_grayboxPanel != null) {
                    m_grayboxPanel.stop();
                    container.remove(m_grayboxPanel);
                }
                paintingSuspended = true;
                EventQueue.invokeLater(new Runnable() {
                        public void run() {
                            container.validate();
                        }
                    });
            }
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }

    public synchronized void resumePainting() 
    {
        try {
            if (paintingSuspended) {
                if (m_grayboxPanel != null) {
                    m_grayboxPanel.start();
                    container.add(m_grayboxPanel, BorderLayout.CENTER);
                }
                paintingSuspended = false;
                EventQueue.invokeLater(new Runnable() {
                        public void run() {
                            container.validate();
                        }
                    });
            }
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }


    public void setAnimationReady() {
		animationReady = true;
    }
	 
    /**
     * The graybox is ready to show progress bar.
     */
    public void setProgressBarReady()
    {
		progressBarReady = true;
    }
 
    /**
     * Repaint gray box.
     */
    private void repaintGrayBox()
    {
        try {
            paintGrayBox(container, container.getGraphics());
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }
    
    private synchronized GrayBoxPanel getGrayBoxPanel() {
	if(m_grayboxPanel == null) {
	    m_grayboxPanel = new GrayBoxPanel(container, boxBGColor, boxFGColor, errorDelegate);
            /* this gets called asynchronously, after 1.5 second delay from constructor
             * if it already has been suspended - do not add it back in
             */
            if (!paintingSuspended) {
	        container.add(m_grayboxPanel, BorderLayout.CENTER);
	        if(!appletErrorOccurred) {
		    m_grayboxPanel.setBoxBorder(boxBorder);
		    if(customImage == null) {
		        m_grayboxPanel.setMaxProgressValue(maximumProgress);
		        m_grayboxPanel.start();
		    } else {
		        m_grayboxPanel.setCustomImage(customImage, customImageIsCentered);
		    }
	        } else {
		    m_grayboxPanel.setError();
	        }		
	        container.validate();
            }
	}
	
	return m_grayboxPanel;
    }
    
    /**
     * Paint gray box.
     */
    public synchronized void paintGrayBox(Container c, Graphics g) {
        try {
            // No-op if painting is suspended or finished, 
            // graphics maybe null when this methods gets
            // called, so we need to check to make sure it is not null.
            if (paintingSuspended || paintingFinished || g == null)
                return;

            // If animation is not ready yet and there is no error,
            // paint the applet's rectangle area only.
            Dimension d = container.getSize(); 		      
            if(!animationReady  && !appletErrorOccurred) {
                if(d.width > 0 && d.height > 0) {			
                    g.setColor(Color.white);
                    g.fillRect(0, 0, d.width, d.height);
                }
                return;
            }
		 

            if (d.width > 0 && d.height > 0){
                GrayBoxPanel panel = getGrayBoxPanel();
                if(appletErrorOccurred) {
                    panel.setError();
                    panel.paint(g);
                } else {
                    panel.progress(currentProgress);
                }
            }    
        } catch (RuntimeException e) {
            Trace.ignoredException(e);
        }
    }

    public void setErrorDelegate(ErrorDelegate errorDelegate) {
        this.errorDelegate = errorDelegate;
    }

    // We need to factor out the ProgressListener functionality into a
    // separate inner class because sun.net.ProgressListener wasn't
    // supported in JDK 1.4.2 and we need to run on that platform
    private Object progressListener;
    class GrayBoxProgressListener implements sun.net.ProgressListener {
    	/**
     	* Start progress.
     	*/
    	public void progressStart(final sun.net.ProgressEvent evt) {
            synchronized(progressSourceFilterList) {
                // Avoid duplicate progress source
                if (progressSourceFilterList.contains(evt.getSource()))
                    return;

                // if jar is specified in the applet, the jar file 
                // download becomes the factor of progress calcalation.
                //
                if (numJarTotal > 0) {
                    for (int i=0; i < jarURLs.length; i++) {
                        // Progress source array contains the progress
                        // that we are interested. In this case, if
                        // the progress source is from the jar file,
                        // we will track it.
                        //
                        if (evt.getURL().equals(jarURLs[i])) {
                            progressSourceFilterList.add(evt.getSource());

                            synchronized(downloadInProgressMap) {
                                downloadInProgressMap.put(evt.getURL(), evt);            
                            }

                            break;
                        }
                    }
                } else {
                    // Applet is loaded through codebase only. We are
                    // only interested in tracking the progress if the
                    // file is from applet codebase.
                    if (evt.getURL().toString().startsWith(codebaseURL.toString()))
                        progressSourceFilterList.add(evt.getSource());        
                }
            }    
        }
    
        /**
         * Update progress.
         */
        public void progressUpdate(final sun.net.ProgressEvent evt) {

            // Check if the progress event is generated from the
            // progress source that we are interested.
            //
            synchronized(progressSourceFilterList) {
                // when reload the applet, progressStart does not get call
                if(progressSourceFilterList.size() == 0) {
                    progressStart(evt);
                }

                // Return if ProgressSource does not exist
                if (progressSourceFilterList.contains(evt.getSource()) == false)
                    return;
            }
    
            if (numJarTotal > 0) {    
                // if jar is specified in the applet, the download progress of
                // each jar file becomes a factor of progress calcalation.
                //
                synchronized(downloadInProgressMap){
                    downloadInProgressMap.put(evt.getURL(), evt);
                    currentProgress = getCurrentProgress();
                }

                // Repaint graybox because progress is updated.
                repaintGrayBox();        
            } else {
                // no-op for codebase download. We only updated progress
                // whenever a file is completely downloaded.
            }    
        }
    
        /**
         * Finish progress.
         */
        public void progressFinish(final sun.net.ProgressEvent evt) {
            // Check if the progress event is generated from the
            // progress source that we are interested.
            //
            synchronized(progressSourceFilterList) {
                // Return if ProgressSource does not exist
                if (progressSourceFilterList.contains(evt.getSource()) == false)
                    return;

                progressSourceFilterList.remove(evt.getSource());
            }
        
            // Take care of the case where there is no response body in a GET request.
            // In this case, we should not try to update the progress.
            //    
            if (ProgressMonitor.getProgress(evt) == 0) {
                downloadInProgressMap.remove(evt.getURL());
                return;
            }
        
            if (numJarTotal > 0) {
                synchronized(downloadInProgressMap) {
                    downloadInProgressMap.remove(evt.getURL());            
                    numberOfJarLoaded++;
            
                    // Maximum progress
                    if (numJarTotal == numberOfJarLoaded) {
                        currentProgress = maximumProgress;
                    } else {
                        currentProgress = getCurrentProgress();
                    }
                }
            } else {
                int step = (maximumProgress - currentProgress) / 2;
                currentProgress = currentProgress + step;
            }

            // Repaint graybox because progress is updated.
            repaintGrayBox();        
        }

        private int getCurrentProgress(){
            // Progress step                
            int step = maximumProgress / numJarTotal;
        
            // Progress so far based on number of jars completely loaded.
            int progress = step * numberOfJarLoaded;
        
            // Add progress of each current loading file 
            for (Iterator iter = downloadInProgressMap.values().iterator();  
                 iter.hasNext();){ 
                sun.net.ProgressEvent pe = (sun.net.ProgressEvent) iter.next();
            
                if (ProgressMonitor.getExpected(pe) != -1) { 
                    progress = (int)(progress + ((double) step)  
                                     / ProgressMonitor.getExpected(pe) * ProgressMonitor.getProgress(pe)); 
                } else { 
                    progress = progress + step / 2;
                }
            }                                 

            return progress;         
        }
    }

    private void installProgressListener() {
        try {
            if (progressBarEnabled && customImageURL == null){
                progressListener = new GrayBoxProgressListener();
                sun.plugin.util.ProgressMonitor pm = (sun.plugin.util.ProgressMonitor) sun.net.ProgressMonitor.getDefault();
                pm.addProgressListener(threadGroup, (GrayBoxProgressListener) progressListener);
            }
        } catch (Throwable t) {
            // We expect this mechanism to fail on JDK 1.4.2
            progressBarEnabled = false;
        }
    }

    private void removeProgressListener() {
        try {
            if (progressBarEnabled) {
                sun.plugin.util.ProgressMonitor pm = (sun.plugin.util.ProgressMonitor) sun.net.ProgressMonitor.getDefault();
                pm.removeProgressListener(threadGroup, (GrayBoxProgressListener) progressListener);
            }
        } catch (Throwable t) {
            // We expect this mechanism to fail on JDK 1.4.2
        }
    }
}

