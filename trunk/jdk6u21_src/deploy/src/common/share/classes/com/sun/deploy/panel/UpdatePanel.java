/*
 * @(#)UpdatePanel.java	1.39 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Image;
import java.awt.Component;
import java.awt.Container;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.SpinnerNumberModel.*;
import javax.swing.event.*;
import java.text.MessageFormat;
import java.text.DateFormatSymbols;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.awt.GridLayout;

import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;


public class UpdatePanel extends JPanel implements ActionListener{

    private class AdvancedDialog implements ActionListener {

	public AdvancedDialog(UpdatePanel updPanel) {
            this.updPanel = updPanel;
            /*
             * Construct AdvancedDialog here.
             */
            JPanel advancedPanel = new JPanel();
            advancedPanel.setLayout(new GridBagLayout());
            c = new GridBagConstraints();

            c.anchor = GridBagConstraints.WEST;
            c.fill = GridBagConstraints.NONE;
            c.insets = new Insets(5, 0, 0, 0);
            c.weighty = 1;
            c.weightx = 0;
            c.gridwidth = GridBagConstraints.REMAINDER;

            advancedPanel.add(new JLabel(updPanel.getMessage("update.advanced_title1.text")), c);
            JPanel FreqPanel = new JPanel();
            FreqPanel.setLayout(new BoxLayout(FreqPanel, BoxLayout.Y_AXIS));
            Border border = BorderFactory.createEtchedBorder();
            FreqPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, 
                                                                 updPanel.getMessage("update.advanced_title2.text"))));

            RButton1 = new JRadioButton(updPanel.getMessage("update.check_daily.text"));
            RButton2 = new JRadioButton(updPanel.getMessage("update.check_weekly.text"));
            RButton3 = new JRadioButton(updPanel.getMessage("update.check_monthly.text"));
            FreqGroup = new ButtonGroup();
            FreqGroup.add(RButton1);
            FreqGroup.add(RButton2);
            FreqGroup.add(RButton3);
            
            RButton1.addActionListener(this);
            RButton2.addActionListener(this);
            RButton3.addActionListener(this);
            FreqPanel.add(RButton1);
            FreqPanel.add(RButton2);
            FreqPanel.add(RButton3);

            c.weighty = 3;
            c.weightx = 0;
            c.gridwidth = 2;
            c.insets = new Insets(0, 0, 0, 0);
            advancedPanel.add(FreqPanel, c);

            whenPanel = new JPanel();
            whenPanel.setLayout(new GridBagLayout());
            whenPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, 
                                                                 updPanel.getMessage("update.advanced_title3.text"))));

            atComboBoxText = updPanel.getAtComboBoxText();
            weekDays = updPanel.getWeekDays();

            dummyLabel = new JLabel("    ");
            everyLabelText = updPanel.getMessage("update.check_day.text");
            atLabelText = updPanel.getMessage("update.check_time.text");
            dayLabelText = updPanel.getMessage("update.check_date.text");
            whenLabel1 = new JLabel();
            whenLabel2 = new JLabel();

            every = new UComboBox(); 
            every.setMaximumRowCount(7);
            every.addItem(weekDays[Calendar.SUNDAY]);
            every.addItem(weekDays[Calendar.MONDAY]);
            every.addItem(weekDays[Calendar.TUESDAY]);
            every.addItem(weekDays[Calendar.WEDNESDAY]);
            every.addItem(weekDays[Calendar.THURSDAY]);
            every.addItem(weekDays[Calendar.FRIDAY]);
            every.addItem(weekDays[Calendar.SATURDAY]);
            every.addActionListener(this);

            at = new UComboBox(); 
            at.setMaximumRowCount(atComboBoxText.length);
            for (int i=0; i < atComboBoxText.length; i++) {
                at.addItem(atComboBoxText[i]);
            }
            at.addActionListener(this);
    
            daymodel = new SpinnerNumberModel(1, 1, 31, 1); 
            day = new JSpinner(daymodel);
            daymodel.addChangeListener(changeListener);

            c.anchor = GridBagConstraints.WEST;

            whenPanel1 = new JPanel();
            whenPanel2 = new JPanel();
            c.insets = new Insets(0, 10, 0, 5);
            c.gridwidth = 2;
            c.gridheight = 2;
            c.weighty = 0;
            c.weightx = 0;
            whenPanel.add(whenLabel1, c);

            c.gridwidth = GridBagConstraints.REMAINDER;
            c.weightx = 2;
            c.fill = GridBagConstraints.HORIZONTAL;
            whenPanel.add(whenPanel1, c);

            c.gridwidth = 2;
            c.fill = GridBagConstraints.NONE;
            c.weightx = 0;
            c.gridheight = GridBagConstraints.REMAINDER;
            whenPanel.add(whenLabel2, c);

            c.weightx = 1;
            c.fill = GridBagConstraints.HORIZONTAL;
            c.gridwidth = GridBagConstraints.REMAINDER;
            whenPanel.add(whenPanel2, c);
    
            c.weightx = 2;
            c.weighty = 3;
            c.fill = GridBagConstraints.BOTH;
            c.gridheight = GridBagConstraints.RELATIVE;
            c.insets = new Insets(0, 5, 0, 0);
            c.gridwidth = GridBagConstraints.REMAINDER;
            advancedPanel.add(whenPanel, c);

            if (updPanel.getUpdateFrequency() == 0) 
            {
                RButton1.setSelected(true);
            }
            else if (updPanel.getUpdateFrequency() == 2) 
            {
                RButton3.setSelected(true);
                day.setValue(new Integer(updPanel.getUpdateDay()));
            }
            else {
                RButton2.setSelected(true);
                every.setSelectedIndex(updPanel.getUpdateDay()-1);
            }
            at.setSelectedIndex(updPanel.getUpdateSchedule());
            
            descLabel = new JLabel("  ");
            descLabel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, "")));
            c.fill = GridBagConstraints.HORIZONTAL;
            c.weightx = 1;
            c.weighty = 1;
            c.insets = new Insets(0, 0, 0, 0);
            c.gridwidth = GridBagConstraints.REMAINDER;
            advancedPanel.add(descLabel, c);
             
            setPanelOptions();

            JOptionPane pane = new JOptionPane(advancedPanel, 
                                               JOptionPane.PLAIN_MESSAGE,
                                               JOptionPane.OK_CANCEL_OPTION);
            
            /*
             * Create the dialog with UpdatePanel tab as parent = owner.  This way,
             * when ControlPanel is maximized, the dialog will show up on top of
             * ControlPanel, conveniently blocking it!!!
             */
            JDialog dialog = pane.createDialog(updPanel, 
                                               updPanel.getMessage("update.advanced_title.text"));
            dialog.setModal(true);
            dialog.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
            dialog.setVisible(true);
            Integer value = (Integer) pane.getValue();
            if (value != null && value.intValue()==JOptionPane.OK_OPTION) {
                    updPanel.setUpdateSchedule(at.getSelectedIndex());
                    if (RButton1.isSelected()) 
                    {
                        updPanel.setUpdateFrequency(0);
                        updPanel.setUpdateDay(0);
                    }
                    else if (RButton3.isSelected())
                    {
                        updPanel.setUpdateFrequency(2);
                        updPanel.setUpdateDay(((Integer)day.getValue()).intValue());
                    }
                    else {
                        updPanel.setUpdateFrequency(1);
                        updPanel.setUpdateDay(every.getSelectedIndex()+1);
                    }
                    ControlPanel.propertyHasChanged();
             }
             updPanel.setText();
        }

        void setPanelOptions() {
            String str = null;
            String str1 = null;
            MessageFormat msgfmt;
 
            if (RButton1.isSelected()) {
		if (!RButton1.equals(RButtonLast)) {
                    day.setVisible(false);
                    every.setVisible(false);
                    whenLabel1.setText("  ");
                    whenLabel2.setText(atLabelText);
                    whenPanel1.add(dummyLabel);
                    whenPanel2.add(at);
                    at.setVisible(true);
                    dummyLabel.setVisible(true);
		    RButtonLast = RButton1;
		}
                str = updPanel.getMessage("update.advanced_desc1.text");
                msgfmt = new MessageFormat(str);
                str = msgfmt.format(new String[] { (String)at.getItemAt(at.getSelectedIndex()) });
            } else if (RButton2.isSelected()) {
		if (!RButton2.equals(RButtonLast)) {
                    day.setVisible(false);
                    dummyLabel.setVisible(false);
                    whenLabel1.setText(everyLabelText);
                    whenLabel2.setText(atLabelText);
                    whenPanel1.add(every);
                    whenPanel2.add(at);
                    every.setVisible(true);
                    at.setVisible(true);
		    RButtonLast = RButton2;
		}
                str1 = (String) every.getItemAt(every.getSelectedIndex());
                str = updPanel.getMessage("update.advanced_desc2.text");
            } else if (RButton3.isSelected()) {
		if (!RButton3.equals(RButtonLast)) {
                    dummyLabel.setVisible(false);
                    every.setVisible(false);
                    whenLabel1.setText(dayLabelText);
                    whenLabel2.setText(atLabelText);
                    whenPanel1.add(day);
                    whenPanel2.add(at);
                    day.setVisible(true);
                    at.setVisible(true);
		    RButtonLast = RButton3;
		}
                str1 = String.valueOf(((Integer)day.getValue()).intValue());
                str = updPanel.getMessage("update.advanced_desc3.text");
            }
            if (str1 != null) {
                msgfmt = new MessageFormat(str);
                str = msgfmt.format(new String[]{str1, (String)at.getItemAt(at.getSelectedIndex()) });
            }
            descLabel.setText(str);
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if ((e.getSource()==RButton1) ||
                    (e.getSource()==RButton2) ||
                    (e.getSource()==RButton3) ||
                    (e.getSource()==every) ||
                    (e.getSource()==at)) {
                    
                    setPanelOptions();
                }
            } catch(Exception exc) { }
        }

        ChangeListener changeListener = new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                try {
                    if (e.getSource()==daymodel) setPanelOptions();
                } catch(Exception exc) { }
            }
        };
        
        private UpdatePanel updPanel;
        private JRadioButton RButton1, RButton2, RButton3;
        private JRadioButton RButtonLast = null;
        private ButtonGroup FreqGroup;
        private String[] weekDays ;
        private String[] atComboBoxText;
        private JComboBox at, every;
        JSpinner day;
        SpinnerNumberModel daymodel;
        private String atLabelText, everyLabelText, dayLabelText;
        private JLabel whenLabel1, whenLabel2, descLabel, dummyLabel;
        JPanel whenPanel1, whenPanel2, whenPanel;
        GridBagConstraints c;
    }

    private class UComboBox extends JComboBox {
        public Dimension getPreferredSize() {
	    Dimension d = super.getPreferredSize();
	    d.width += 8;
	    return d;
	}
    }

    private class MyBoxLayout extends BoxLayout {
        public MyBoxLayout(Container c, int axis) {
	    super(c, axis);
        }

	public Dimension preferredLayoutSize(Container parent) {
	    int width = parent.getWidth();
	    Dimension size = super.preferredLayoutSize(parent);
	    if ((width > 0) && (size.width > width)) {
		// can't fit = go to a BorderLayout
		Component[] children = parent.getComponents();
		parent.removeAll();
		parent.setLayout(new BorderLayout());
		if (children.length > 0) {
		    parent.add(children[0], BorderLayout.NORTH);
		}
		// don't need Box.createGlue() component
		if (children.length > 2) {
		    parent.add(children[2], BorderLayout.EAST);
		}
		return parent.getLayout().preferredLayoutSize(parent);
	    }
	    return size;
	}
    }

    /**
     * Construct the panel, add widgets
     */
    UpdatePanel() {
        //
        // Get current Java Update settings.
        //
        PlatformSpecificUtils instance = new PlatformSpecificUtils();
        instance.onLoad(this);        
        
        setLayout(new BorderLayout());
        
        // Setup borders
        Border outerBorder = new EmptyBorder(new Insets(5, 5, 5, 5));        
        Border innerBorder = new TitledBorder(new TitledBorder(new EtchedBorder()), 
                                              getMessage("update.notify.border.text"), 
                                              TitledBorder.DEFAULT_JUSTIFICATION, 
                                              TitledBorder.DEFAULT_POSITION);
        setBorder(BorderFactory.createCompoundBorder(outerBorder, innerBorder));        
                
        /*
         * This is the panel with java cup and other stuff.  It will be added to the
         * contentPane of UpdatePanel.
         */
        JPanel topLevelPanel = new JPanel();
        topLevelPanel.setLayout(new BorderLayout());
        
        /*
         * This is the panel with java cup image.
         */
        JPanel imagePanel = new JPanel();
        imagePanel.setLayout(new BorderLayout());
        ImageIcon img = new ImageIcon(ClassLoader.getSystemResource(update_image));        
        
        JLabel imgLabel = new JLabel();
        imgLabel.setIcon((Icon)img);
        imagePanel.add(imgLabel, BorderLayout.NORTH);  
        
        // Use border layout WEST to prevent shifting of components once
        // the panel becomes visible.
        topLevelPanel.add(imagePanel, BorderLayout.WEST);

        /*
         * This is the panel with all the text areas and buttons on it.
         */
        JPanel otherStuffPanel = new JPanel();
        otherStuffPanel.setLayout(new BorderLayout());
        
        /*
         * updateTextPanel contains text area.
         */
        JPanel updateTextPanel = new JPanel();
        updateTextPanel.setLayout(new BorderLayout());
        updateTextPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

        
	DateFormatSymbols datefmt = new DateFormatSymbols();
	DateFormat formatter = DateFormat.getTimeInstance(DateFormat.SHORT);
	SimpleDateFormat sdf = new SimpleDateFormat("H:mm");
	Date temp;

	weekDays = datefmt.getWeekdays();
	atComboBoxText = new String[24];
	//start with 12AM
	for (int i=0; i < 24; i++) {
	    try {
                temp = sdf.parse(i+":00");
                atComboBoxText[i] = formatter.format(temp);
            } catch (Exception e)  { atComboBoxText[i] = i+":00";}
        }

        JSmartTextArea updateTextArea = new JSmartTextArea(getMessage("update.desc.text"));
        updateTextPanel.add(updateTextArea, BorderLayout.CENTER);        

        /*
         * Create "northPanel" and put the following subpanels on it:
         *  - updateTextPanel
         *  - notifyPanel
         *  - checkUpdatesPanel
         *
         * This is to position these three subpanels with subcomponents
         * on a separate panel and avoid re-packing by layout manager when
         * length of text in text area changes.
         */
        JPanel northPanel = new JPanel();
        northPanel.setLayout(new BorderLayout());
        northPanel.add(updateTextPanel, BorderLayout.NORTH);
                  
        /*
         * notifyPanel contains label and combo box.
         */
        JPanel notifyPanel = new JPanel();
	notifyPanel.setLayout(new MyBoxLayout(notifyPanel, BoxLayout.X_AXIS));
        notifyPanel.setBorder(BorderFactory.createEmptyBorder(5, 10, 5, 8));
        JLabel notifyMeLabel = new JLabel(getMessage("update.notify.text"));
        notifyComboBox = new UComboBox();
        notifyComboBox.addItem(getMessage("update.notify_download.text"));
        notifyComboBox.addItem(getMessage("update.notify_install.text"));
        notifyComboBox.addActionListener(this);
        notifyComboBox.setToolTipText(getMessage("update.notify_combo.tooltip"));

        notifyPanel.add(notifyMeLabel);
        notifyPanel.add(Box.createGlue());
        notifyPanel.add(notifyComboBox);

        northPanel.add(notifyPanel, BorderLayout.CENTER);
                        
        /*
         * checkUpdatesPanel contains checkbox and "Advanced..." button.
         */
        JPanel checkUpdatesPanel = new JPanel();
	checkUpdatesPanel.setLayout(new MyBoxLayout(checkUpdatesPanel, BoxLayout.X_AXIS));
        checkUpdatesPanel.setBorder(BorderFactory.createEmptyBorder(5, 7, 5, 8));
        autoUpdateChBox = new JCheckBox(getMessage("update.autoupdate.text"));
        autoUpdateChBox.addActionListener(this);
	// Disable option if the corporate override is set
	autoUpdateChBox.setEnabled(!corporateOverride);
	
        advancedUpdateBtn = new JButton(getMessage("update.advanced.button.text"));
        advancedUpdateBtn.setMnemonic(ResourceManager.getVKCode("update.advanced.button.mnemonic"));
        advancedUpdateBtn.setToolTipText(getMessage("update.advanced_btn.tooltip"));
        advancedUpdateBtn.addActionListener(this);
        
        checkUpdatesPanel.add(autoUpdateChBox);
        checkUpdatesPanel.add(Box.createGlue());
        checkUpdatesPanel.add(advancedUpdateBtn);

        // add checkUpdatesPanel to the north panel
        northPanel.add(checkUpdatesPanel, BorderLayout.SOUTH);
        otherStuffPanel.add(northPanel, BorderLayout.NORTH);
        
        /*
         * scheduleTextPanel contains text area.
         */
        JPanel scheduleTextPanel = new JPanel();
        scheduleTextPanel.setLayout(new GridLayout(1, 1));
        scheduleTextPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        updateScheduleTextArea = new JSmartTextArea(" ");
        scheduleTextPanel.add(updateScheduleTextArea);
        
        /*
         * The only purpose of the centerPanel is to have a strict position
         * within otherStuffPanel.  It should occupy 1 grid in the CENTER.
         * This way it will not get re-positioned when the length of its
         * text changes on the fly.
         */
        JPanel centerPanel = new JPanel();
        centerPanel.setLayout(new GridLayout(1, 1));

        //add panel with text area to center panel.
        centerPanel.add(scheduleTextPanel);
        otherStuffPanel.add(centerPanel, BorderLayout.CENTER);

        /*
         * updateNowPanel with text area and button.
         */
	BorderLayout layout = new BorderLayout();
	layout.setHgap(8);
        JPanel updateNowPanel = new JPanel(layout);
        updateNowPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        
        lastUpdatedTextArea = new JSmartTextArea(" ");        
        
        updateNowBtn = new JButton(getMessage("update.updatenow.button.text"));
        updateNowBtn.setMnemonic(ResourceManager.getVKCode("update.updatenow.button.mnemonic"));
        updateNowBtn.setToolTipText(getMessage("update.now_btn.tooltip"));
        updateNowBtn.addActionListener(this);
        
        // Enable "Update Now" button if manual update enalbed and
        // JRE download is complete
	updateNowBtn.setEnabled(manualUpdateEnabled && 
                sun.jkernel.DownloadManager.isJREComplete());
        
        updateNowPanel.add(lastUpdatedTextArea, BorderLayout.CENTER);
        updateNowPanel.add(updateNowBtn, BorderLayout.EAST);

        // add updateNowPanel to topLevelPanel
        otherStuffPanel.add(updateNowPanel, BorderLayout.SOUTH);
        
        // Use border layout CENTER to prevent shifting of components once
        // the panel becomes visible.        
        topLevelPanel.add(otherStuffPanel, BorderLayout.CENTER);

        // add topLevelPanel to the update panel.
        add(topLevelPanel, BorderLayout.CENTER);        
        
	reset();
    }
    

    /* Reset all the settings from the model */
    public void reset() {
	if (isSchedulerPlatform())
	{
	    autoUpdateChBox.setSelected(isAutoUpdateChecked());
	}
	else
	{
	    autoUpdateChBox.setEnabled(false);
	}
        notifyComboBox.setSelectedIndex(getUpdateNotify());
        setText();
        String t = getUpdateLastRun();
        Calendar c = new GregorianCalendar();
        try {
            c.setTime(df.parse(t));
        } catch (Exception e)  { 
            t = null; 
        }
        if (t == null) t = new String("    ");
        else {
            MessageFormat lastRunText;
	    DateFormat timefmt = DateFormat.getTimeInstance(DateFormat.SHORT);
	    DateFormat datefmt = DateFormat.getDateInstance(DateFormat.SHORT);

            lastRunText = new MessageFormat(getMessage("update.lastrun.text"));

            String str1 = timefmt.format(c.getTime());
            String str2 = datefmt.format(c.getTime());
            t = lastRunText.format(new String[]{str1,str2});
        }
        lastUpdatedTextArea.setText(t);
    }
    
    // Set textDesc to be displaed at the bottom.
    private void setText() {
        if (!autoUpdateChBox.isSelected()) {
	    advancedUpdateBtn.setEnabled(false);
	    textDesc = getMessage("update.desc_autooff.text");
            updateScheduleTextArea.setText(textDesc);
	} else {
	    advancedUpdateBtn.setEnabled(true);
	    int index = getUpdateFrequency();
            
            MessageFormat msgfmt;
            String time_str = atComboBoxText[getUpdateSchedule()];
            String str1 = null;
	    if (index == 0) {
	        textDesc = getMessage("update.desc_check_daily.text");
                msgfmt = new MessageFormat(textDesc);
                textDesc = msgfmt.format(new String[] { time_str }); 
            }
	    else if (index == 2) {
	        textDesc = getMessage("update.desc_check_monthly.text");
                str1 = String.valueOf(getUpdateDay());
            }
	    else {
	        textDesc = getMessage("update.desc_check_weekly.text") ;
                str1 = String.valueOf(weekDays[getUpdateDay()]);
            }
            if (str1 != null) {
                msgfmt = new MessageFormat(textDesc);
                textDesc = msgfmt.format(new String[]{ str1, time_str });
            }
	    textDesc = textDesc + sysTrayIconText;
	    index = notifyComboBox.getSelectedIndex();
            if (index == 1) 
	        textDesc = textDesc + getMessage("update.desc_notify_install.text");
	    else
	        textDesc = textDesc + getMessage("update.desc_notify_download.text");
            updateScheduleTextArea.setText(textDesc);
	}
 	//Disable editing of preferences if not admin user
	if (!javaUpdateEditPrefs)
	{
	    autoUpdateChBox.setEnabled(false);
	    advancedUpdateBtn.setEnabled(false);
	    notifyComboBox.setEnabled(false);
	}
    }

    /**
     * ActionListener interface implementation. All fields in this
     * panel will generate this message when changed. We use the
     * internal UI state to save the user choices so we just reset
     * the hasChanged field of our model
     *
     * @param ActionEvent info about the event
     */
    public void actionPerformed(ActionEvent e) 
    {
      try {
          if (e.getSource() == updateNowBtn) {
	      //Obtain the program files path by querying the registry
	      String cmdOutput = null; //String to store the output of exec cmd
	      String path      = null; //String to store the path
	      try{
		  Process query    = Runtime.getRuntime().exec( "REG QUERY " +
								"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion " +
								"/v ProgramFilesDir");
		  BufferedReader input = new BufferedReader(new InputStreamReader(query.getInputStream()));
		  //iterate through the result
		  while((cmdOutput = input.readLine()) != null){
		      //if cmdOutput string contains the ProgramFilesDir value, parse it
		      if(cmdOutput.contains("ProgramFilesDir")){
			  //split the line with either tabs or 4 spaces
			  //windows xp is tab delimited, vista and windows 7 is 4 space delimited
			  String tokens[] = cmdOutput.split("\t|\\s{4,4}");
			  //string will be of format "\tvalue name"\t"Value type"\t"Path"
			  //check to make sure that string was tokenized properly
			  if(tokens.length == 4){
			      path            = tokens[3];
			  }
			  break;
		      }
		  }
	      } catch (IOException ioe){
		  Trace.ignored(ioe);
	      }

	      //revert to original functionality if path was not obtained
	      if(path == null){
		  path = System.getenv("programfiles");
	      }

 	     // Invoke Update Checker, make sure that we quote the cmd, as the path might have spaces
	      String jupdchecker = "\"" + path + //should point to windows "Program files" folder
                                  File.separator + "Common Files" +
                                  File.separator + "Java" +
                                  File.separator + "Java Update" +
                                  File.separator + "jucheck.exe" + "\"";
	     try {
                Runtime.getRuntime().exec(jupdchecker);
	     } catch (IOException ioe) {
		Trace.ignored(ioe);
	        String args[] = new String[3];
	        args[0] = "cmd";
		args[1] = "/c";
                args[2] = jupdchecker;
                Runtime.getRuntime().exec(args);
	     }
         } else if (e.getSource() == advancedUpdateBtn) {
             new AdvancedDialog(this); 
         } else if (e.getSource() == notifyComboBox) {
             int index = notifyComboBox.getSelectedIndex();
	     setUpdateNotify(index);
	     setText();
             ControlPanel.propertyHasChanged();
         } else if (e.getSource() == autoUpdateChBox) {
	     boolean b = autoUpdateChBox.isSelected();
             setAutoUpdateCheck(b);
	     setText();
             ControlPanel.propertyHasChanged();
         } 
      } catch(Exception exc) {
          UIFactory.showExceptionDialog(this, exc, null, null);
      }
    }

    public String[] getAtComboBoxText() 
    {
        return atComboBoxText; 
    }
    
    public String[] getWeekDays() 
    {
        return weekDays; 
    }        

    public void setManualUpdate(boolean b) {
	manualUpdateEnabled = b;
    }

    public void setCorporateOverride(boolean b) {
	corporateOverride = b;
    }

    private String getMessage(String id)
    {
	return com.sun.deploy.resources.ResourceManager.getMessage(id);
    } 
    
    
    /*
     * Save all JavaUpdate settings in the registry.  Windows only.
     */
    public void saveUpdateSettingsInReg(){    
        PlatformSpecificUtils instance = new PlatformSpecificUtils();
        instance.onSave(this);
    }
    
    
    // Update vars
    private int		update_notify = 0;
    private int		update_frequency = 1;
    private int		update_day = 1;
    private int		update_schedule = 0;
    private String	update_lastrun ;


    public void setUpdateNotify(int i) {
        update_notify = i;
    }

    // return the index into ComboBox
    public int getUpdateNotify() {
        return update_notify;
    }

    public void setUpdateFrequency(int freq) {
	update_frequency = freq;
    }

    public void setUpdateDay(int day) {
        update_day = day;
    }

    public int getUpdateFrequency() {
        return update_frequency;
    }

    public int getUpdateDay() {
        return update_day;
    }

    public void setUpdateSchedule(int sched) {
        update_schedule = sched;
    }

    public int getUpdateSchedule() {
        return update_schedule;
    }

    public void setUpdateLastRun(String s) {
        update_lastrun = s;
    }

    // return the index into ComboBox
    public String getUpdateLastRun() {
        return update_lastrun;
    }   
        
    public void enableJavaUpdate(boolean b) {
        // This is called from native.
        javaUpdateEnabled = b;
    }

    public boolean isJavaUpdateEnabled() {
        return javaUpdateEnabled;
    }

    public void enableEditPrefs(boolean b) {
        javaUpdateEditPrefs = b;
    }

    public void setAutoUpdateCheck(boolean b) {
        int result = UIFactory.CANCEL;
	if ((b == false) && (b != autoUpdateCheck))
	{
	    //showWarning Dialog
	    // OK button should have "Check Monthly/Check Weekly/Check Daily" 
	    // Cancel button should have "Never Check" 
            final String cancelString = ResourceManager.getMessage(
                "update.autoupdate.disable.neverCheck");
	    final String title = ResourceManager.getMessage("update.warning");
	    final String masthead = ResourceManager.getMessage(
                    "update.autoupdate.disable.message");
	    final String message = ResourceManager.getMessage(
                    "update.autoupdate.disable.info");
	    String okString = ResourceManager.getMessage("update.autoupdate.disable.monthlyCheck");
	    if (getUpdateFrequency() == 0)
		okString = ResourceManager.getMessage("update.autoupdate.disable.dailyCheck");
            else if (getUpdateFrequency() == 1) 
		okString = ResourceManager.getMessage("update.autoupdate.disable.weeklyCheck");

	    result = UIFactory.showWarningDialog(null, null, masthead, message, 
                    title, okString, cancelString);            
	}
        if (result == UIFactory.OK)
	{
	    autoUpdateChBox.setSelected(true);
	}
	else
	{
	    autoUpdateCheck = b;
	}
    }

    public boolean isAutoUpdateChecked() {
        return autoUpdateCheck;
    }

    public void setSchedulerPlatform(boolean b) {
        schedulerPlatform = b;
    }

    public boolean isSchedulerPlatform() {
        return schedulerPlatform;
    }

        
    private static final String update_image = "com/sun/deploy/resources/image/JavaUpdateIcon-48.png";
    private JButton updateNowBtn, advancedUpdateBtn;
    private JComboBox notifyComboBox;
    private JCheckBox autoUpdateChBox;
    private JSmartTextArea updateScheduleTextArea, lastUpdatedTextArea;
    private String textDesc;
    private DateFormat df = new SimpleDateFormat("EEE, dd MMM yyyy hh:mm:ss zzz",Locale.US);
    private String sysTrayIconText = getMessage("update.desc_systrayicon.text");
    private String[] weekDays ;
    private String[] atComboBoxText;
    private boolean javaUpdateEnabled = false;  
    private boolean javaUpdateEditPrefs = false;
    private boolean autoUpdateCheck = false;
    private boolean schedulerPlatform = true;
    private boolean manualUpdateEnabled = true;
    private boolean corporateOverride = false;
} 
