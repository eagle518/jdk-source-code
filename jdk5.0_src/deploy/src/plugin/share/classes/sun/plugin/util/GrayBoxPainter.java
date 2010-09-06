/*
 * @(#)GrayBoxPainter.java	1.6 04/04/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;


import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
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
/**
 * GrayBoxPainter is a helper class to paint the applet gray box with
 * image and progress bar.
 */
public class GrayBoxPainter implements sun.net.ProgressListener
{
    private Color boxBGColor = Color.white;
    private Color boxFGColor = Color.black;
    
    // Waiting message displayed in graybox
    private String waitingMessage = null;
    
    // Custom image
    private Image		customImage;
    private URL			customImageURL;

    // Applet codebase    
    private URL codebaseURL;
    
    // Applet jar URLs
    private URL jarURLs[] = new URL[0];
    private HashMap downloadInProgressMap = new HashMap();    
    
    // Flag indicate if progress bar should be enabled
    private boolean progressBarEnabled = true; 

    // Applet panel
    private Container container;

    // Thread group the applet belongs    
    private ThreadGroup threadGroup;

    // Media tracker for loading image
    private MediaTracker tracker;
    
    // Error flag
    private boolean appletErrorOccurred = false;
    
    private Color progressColor;
    // ready flag
    private boolean animationReady = false;
    private boolean progressBarReady = false;    
    private boolean freezePaint = false;


	private GrayBoxPanel	m_grayboxPanel = null;
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

		// Track progress if progress bar is enabled
		if (progressBarEnabled)
		{
			sun.plugin.util.ProgressMonitor pm = (sun.plugin.util.ProgressMonitor) sun.net.ProgressMonitor.getDefault();
			pm.addProgressListener(threadGroup, this);
		}
		// Show progress bar only after graybox appears
		// more than 2 second. This is to avoid the flickering
		// effect during page switch or fast loading.
		//
		final GrayBoxPainter painter = this;		    
		new Thread(new Runnable(){
			public void run() 
			{
				try {
					Thread.sleep(2000L);
				} catch (Throwable e) {
				}
				finally {		
					painter.setAnimationReady();
					painter.setProgressBarReady();
					painter.repaintGrayBox();
				}
			}
		}).start();
	}
	   
    /**
     * Finish painting in graybox.
     */
    public synchronized void finishPainting()
    {
	if(this.m_grayboxPanel != null) {
		m_grayboxPanel.stop();
		container.remove(m_grayboxPanel);
		m_grayboxPanel = null;
	}

	if (progressBarEnabled)
	{
		sun.plugin.util.ProgressMonitor pm = (sun.plugin.util.ProgressMonitor) sun.net.ProgressMonitor.getDefault();
		pm.removeProgressListener(threadGroup, this);
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
      		} catch(InterruptedException e) {
			e.printStackTrace();
		}
	}
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
     * Set user-defined image.
     */
	public void setCustomImageURL(URL url){
		progressBarEnabled = false;
		customImageURL = url;
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
        
    	/**
     	* Start progress.
     	*/
    	public void progressStart(final sun.net.ProgressEvent evt)
   	{
		synchronized(progressSourceFilterList)    
		{
			// Avoid duplicate progress source
			if (progressSourceFilterList.contains(evt.getSource()))
				return;

			// if jar is specified in the applet, the jar file 
			// download becomes the factor of progress calcalation.
			//
			if (numJarTotal > 0)
			{
				for (int i=0; i < jarURLs.length; i++)
				{
					// Progress source array contains the progress
					// that we are interested. In this case, if
					// the progress source is from the jar file,
					// we will track it.
					//
					if (evt.getURL().equals(jarURLs[i])) 
					{
						progressSourceFilterList.add(evt.getSource());

						synchronized(downloadInProgressMap) {
							downloadInProgressMap.put(evt.getURL(), evt);			
					}

					break;
					}
				}
			}
			else
			{
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
    public void progressUpdate(final sun.net.ProgressEvent evt)
    {

    	// Check if the progress event is generated from the
		// progress source that we are interested.
		//
		synchronized(progressSourceFilterList) 
		{
            // when reload the applet, progressStart does not get call
            if(progressSourceFilterList.size() == 0) {
                progressStart(evt);
            }

			// Return if ProgressSource does not exist
			if (progressSourceFilterList.contains(evt.getSource()) == false)
			return;
		}
	
		if (numJarTotal > 0)
		{	
			// if jar is specified in the applet, the download progress of
			// each jar file becomes a factor of progress calcalation.
			//
			synchronized(downloadInProgressMap)
			{
				downloadInProgressMap.put(evt.getURL(), evt);		
			
				// Progress step
				int step = maximumProgress / numJarTotal;
			
				// Progress so far based on number of jar completely loaded.
				int progress = step * numberOfJarLoaded;
				
				// Add progress of each current loading file    
				for (Iterator iter = downloadInProgressMap.values().iterator(); iter.hasNext();)
				{
					sun.net.ProgressEvent pe = (sun.net.ProgressEvent) iter.next();
				
					if (pe.getExpected() != -1)
					{
						progress = (int)(progress + ((double) step) / pe.getExpected() * pe.getProgress());
					}
					else
					{
						progress = progress + step / 2;
					}
			}
				
			currentProgress = progress;		
			}

			// Repaint graybox because progress is updated.
			repaintGrayBox();	    
		}
		else
		{
			// no-op for codebase download. We only updated progress
			// whenever a file is completely downloaded.
		}	
    }
    
    /**
     * Finish progress.
     */
    public void progressFinish(final sun.net.ProgressEvent evt)
    {
		// Check if the progress event is generated from the
		// progress source that we are interested.
		//
		synchronized(progressSourceFilterList) 
		{
			// Return if ProgressSource does not exist
			if (progressSourceFilterList.contains(evt.getSource()) == false)
				return;

			progressSourceFilterList.remove(evt.getSource());
		}
		
		// Take care of the case where there is no response body in a GET request.
		// In this case, we should not try to update the progress.
		//	
		if (evt.getProgress() == 0)
		{
			downloadInProgressMap.remove(evt.getURL());
			return;
		}
		
		if (numJarTotal > 0)
		{
			synchronized(downloadInProgressMap)
			{
			downloadInProgressMap.remove(evt.getURL());			
			numberOfJarLoaded++;
			
			// Maximum progress
			if (numJarTotal == numberOfJarLoaded)
			{
				currentProgress = maximumProgress;
			}
			else
			{
				int step = maximumProgress / numJarTotal;

				// Progress step		
				int progress = step * numberOfJarLoaded;
				
				for (Iterator iter = downloadInProgressMap.values().iterator(); iter.hasNext();)
				{
				sun.net.ProgressEvent pe = (sun.net.ProgressEvent) iter.next();
				
				if (pe.getExpected() != -1)
				{
					progress = (int)(progress + ((double) step) / pe.getExpected() * pe.getProgress());
				}
				else
				{
					progress = progress + step / 2;
				}
				}
				
				currentProgress = progress;
			}
			}
		}
		else
		{
			int step = (maximumProgress - currentProgress) / 2;
			currentProgress = currentProgress + step;
		}

		// Repaint graybox because progress is updated.
		repaintGrayBox();	    
    }
          
          
    /*
     * See if user specified the background color for the applet's gray box.
     */
    public void setBoxBGColor(Color bgColor)
    {
		boxBGColor = bgColor;
    }
     
    /*
     * Return the background color for the applet's gray box.
     */
    public Color getBoxBGColor()
    {
		return boxBGColor;
    }

    /*
     * See if user specified the foreground color for the applet's gray box.
     */
    public void setBoxFGColor(Color fgColor)
    {
		boxFGColor = fgColor;
    }

    /*
     * Return the progress color for the applet's gray box.
     */
    public void setProgressColor(Color pgColor)
    {
		progressColor = pgColor;
    }

    /**
     * Set waiting message to be displayed in gray box.
     */
    public void setWaitingMessage(String msg)
    {
		waitingMessage = msg;
    }

    /**
     * Enable progress bar support
     */
    public void enableProgressBar(boolean b)
    { 
		progressBarEnabled = b;
    }      
 
    /**
     * Freeze the painting of the graybox.
     */
    public synchronized void freezePainting(boolean b)
    {
		if(b == freezePaint)
			return;

		if(b){
			if(m_grayboxPanel != null)
				container.remove(m_grayboxPanel);
		}else {			
			container.add(getGrayBoxPanel(), BorderLayout.CENTER);
		}

		container.validate();
		freezePaint = b;
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
		paintGrayBox(container, container.getGraphics());
    }
    

    private synchronized GrayBoxPanel getGrayBoxPanel() {
		if(m_grayboxPanel == null) {
			m_grayboxPanel = new GrayBoxPanel(container);
			container.add(m_grayboxPanel, BorderLayout.CENTER);
			if(!appletErrorOccurred) {
				if(customImage == null) {
					m_grayboxPanel.setMaxProgressValue(maximumProgress);
					m_grayboxPanel.start();
				} else {
					m_grayboxPanel.setCustomImage(customImage);
				}
			}else {
				m_grayboxPanel.setError();
			}		
			container.validate();
		}

		return m_grayboxPanel;
    }

    /**
     * Paint gray box.
     */
    public synchronized void paintGrayBox(Container c, Graphics g) {
		// No-op if painting is freeze
		if (freezePaint)
			return;	     

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
    }
}

