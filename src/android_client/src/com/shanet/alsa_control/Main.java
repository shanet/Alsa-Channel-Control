// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.io.IOException;
import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Toast;

public class Main extends Activity {
	
	public static ArrayList<String> serverList  = null;
	public static ArrayList<String> channelList = null;
	private Background bg                        = null;
    private boolean isPaused                    = false;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
                
        // Checking if this is the first run and if it is, write the corresponding values to the prefs.
        if(getSharedPreferences(Constants.SETTINGS_FILE, 0).getBoolean("onFirstRun", true)) {			
          	AlertDialog welcomeDialog = DialogUtils.createAboutDialog(this, Constants.ABOUT_THIS_APP);
          	welcomeDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
				public void onDismiss(DialogInterface dialog) {
					try {
						VersionUtils.writeOnFirstRun(Main.this);
					// An IO excpetion means the default channels couldn't be written to the file. This isn't
					// a problem now, but could be later. Let the later stuff deal with it.
					} catch (IOException ioe) {}
					
					VersionUtils.writeVersionCode(Main.this);
				}
			});
          	welcomeDialog.setTitle(R.string.welcomeTitle);
          	welcomeDialog.show();
			
        // Display the release notes if this is a new version
        } else if(VersionUtils.compareVersionCode(this)) {
          	AlertDialog changelogDialog = DialogUtils.createAboutDialog(this, Constants.CHANGELOG);
          	changelogDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
				public void onDismiss(DialogInterface dialog) {
					VersionUtils.writeVersionCode(Main.this);
				}
			});
          	changelogDialog.setTitle(R.string.changelogTitle);
          	changelogDialog.show();
        }
        
        if(savedInstanceState == null) {
            // Init the server and channel lists
        	serverList = new ArrayList<String>();
        	channelList = new ArrayList<String>();
        } else {
        	// The server and channel lists are saved; restore them
        	onRestoreInstanceState(savedInstanceState);
        }
        
        EditText serverText = (EditText) findViewById(R.id.serverText);
        EditText channelText = (EditText) findViewById(R.id.channelText);
        final CheckBox lockCheck = (CheckBox) findViewById(R.id.lockVolumes);
        final SeekBar leftVol = (SeekBar) findViewById(R.id.leftVolPicker);
        final SeekBar rightVol = (SeekBar) findViewById(R.id.rightVolPicker);
        
        // Lock the seekbars if the checkbox is checked
        OnSeekBarChangeListener lockVolListener = new OnSeekBarChangeListener() {
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
				// Apply a step size of 5%
				seekBar.setProgress(((int)(progress/Constants.STEP_SIZE))*Constants.STEP_SIZE);
				
				if(lockCheck.isChecked()) {
					if(seekBar == leftVol)
						rightVol.setProgress(leftVol.getProgress());
					else
						leftVol.setProgress(rightVol.getProgress());
				}
				
		    	// Check for empty server and channel lists
				if(serverList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyServer, Toast.LENGTH_SHORT).show();
					return;
				} else if(channelList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyChannel, Toast.LENGTH_SHORT).show();
					return;
				}
								
				// Construct the info bundle
				Bundle serverInfo = new Bundle();
				serverInfo.putInt("command", Constants.CMD_VOL);
				serverInfo.putStringArrayList("hosts", serverList);
				serverInfo.putInt("port", Utils.getIntPref(Main.this, "port"));
				serverInfo.putStringArrayList("channels", channelList);
				serverInfo.putInt("leftVol", leftVol.getProgress());
				serverInfo.putInt("rightVol", rightVol.getProgress());
				
				// Call the bg thread to send the commands to the server
				if(execBgThread(serverInfo) == Constants.FAILURE) return;
			}
			
        	public void onStopTrackingTouch(SeekBar seekBar) {}
			public void onStartTrackingTouch(SeekBar seekBar) {}
		};
        
        leftVol.setOnSeekBarChangeListener(lockVolListener);
        rightVol.setOnSeekBarChangeListener(lockVolListener);
        
        // Get the server(s) to use
		final Intent selectIntent = new Intent(Main.this, SelectDialog.class);
        serverText.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				selectIntent.putExtra("dialogType", Constants.SERVERS);
				startActivity(selectIntent);
			}
		});
        
        // Get the channel(s) to use
        channelText.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				selectIntent.putExtra("dialogType", Constants.CHANNELS);
				startActivity(selectIntent);	
			}
		});
                
        // Set the media button listeners
        final ImageButton play = (ImageButton)findViewById(R.id.play);
        final ImageButton next = (ImageButton)findViewById(R.id.next);
        final ImageButton prev = (ImageButton)findViewById(R.id.prev);
        
        play.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {		
		    	// Check for empty server list
				if(serverList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyServer, Toast.LENGTH_SHORT).show();
					return;
				}
				
				// Change the button image to the play/pause icon
				if(isPaused) {
					play.setImageDrawable(getResources().getDrawable(R.drawable.play));
				} else {
					play.setImageDrawable(getResources().getDrawable(R.drawable.pause));
				}
					
				// Build the bundle of data for the bg thread
				Bundle serverInfo = new Bundle();
				serverInfo.putInt("command", Constants.CMD_PLAY);
				serverInfo.putStringArrayList("hosts", serverList);
				serverInfo.putInt("port", Utils.getIntPref(Main.this, "port"));
				
				// Call the bg thread to send the commands to the server
				execBgThread(serverInfo);
				
				// Show a toast confirmation that the command was sent
				Toast.makeText(Main.this, String.format(Main.this.getString(R.string.sentPlay), ((isPaused) ? "Pause" : "Play")), Toast.LENGTH_SHORT).show();
				
				// Update the paused flag
				isPaused = !isPaused;
			}
        });
        
        next.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {								
		    	// Check for empty server list
				if(serverList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyServer, Toast.LENGTH_SHORT).show();
					return;
				}
			
				// Build the bundle of data for the bg thread
				Bundle serverInfo = new Bundle();
				serverInfo.putInt("command", Constants.CMD_NEXT);
				serverInfo.putStringArrayList("hosts", serverList);
				serverInfo.putInt("port", Utils.getIntPref(Main.this, "port"));
				
				// Call the bg thread to send the commands to the server
				execBgThread(serverInfo);
				
				// Show a toast confirmation that the command was sent
				Toast.makeText(Main.this, R.string.sentNext, Toast.LENGTH_SHORT).show();
			}
        });
        
        prev.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {								
		    	// Check for empty server list
				if(serverList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyServer, Toast.LENGTH_SHORT).show();
					return;
				}
				
				// Build the bundle of data for the bg thread
				Bundle serverInfo = new Bundle();
				serverInfo.putInt("command", Constants.CMD_PREV);
				serverInfo.putStringArrayList("hosts", serverList);
				serverInfo.putInt("port", Utils.getIntPref(Main.this, "port"));
				
				// Call the bg thread to send the commands to the server
				execBgThread(serverInfo);
				
				// Show a toast confirmation that the command was sent
				Toast.makeText(Main.this, R.string.sentPrev, Toast.LENGTH_SHORT).show();
			}
        });
    }
	
	public int execBgThread(Bundle serverInfo) {
		// If the background object is null, init it
		boolean firstConnect = false;
		if(bg == null) {
			bg = new Background(Main.this);
			firstConnect = true;
		}
		
		// Wait for the previous bg thread to finish if not the first time executing it since it must
		// execute once before setting its status to finished
		if(!firstConnect && bg.getStatus() != AsyncTask.Status.FINISHED) {
			return Constants.FAILURE;
		}
		bg = new Background(Main.this);
		bg.execute(serverInfo);
		
		return Constants.SUCCESS;
	}
	
	@Override
	public void onResume() {
		super.onResume(); 
		reloadLists();
	}

	
	@Override
	public void onStop() {
		super.onStop();
		// This must be overridden for the save instance state function to be called for whatever reason
	}
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		
		// Stop all the servers in the bg thread
		Background.closeAllServers(false);
	}
	
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}
	
	
	@Override
	public void onSaveInstanceState(Bundle savedInstanceState) {
		super.onSaveInstanceState(savedInstanceState);
		
		// Save the server and channel lists and current volume positions
		savedInstanceState.putStringArrayList("serverList", serverList);
		savedInstanceState.putStringArrayList("channelList", channelList);
	}
	
	
	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
		
		// Copy the server and channel lists back to the class objects
		serverList = savedInstanceState.getStringArrayList("serverList");
		channelList = savedInstanceState.getStringArrayList("channelList");
	}
	
	
	@Override 
	public boolean onCreateOptionsMenu(Menu menu) {
		return Utils.onCreateOptionsMenu(this, menu);
	}

	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		return Utils.onOptionsItemSelected(this, item);
	}

	
	private void reloadLists() {
		// Reload the server and channel text fields
		EditText serverText = (EditText) findViewById(R.id.serverText);
		EditText channelText = (EditText) findViewById(R.id.channelText);

		// Clear the current text
		serverText.setText("");
		channelText.setText("");
		
		if(serverList.size() != 0)
			serverText.setText(serverList.get(0));
		if(channelList.size() != 0)
			channelText.setText(channelList.get(0));
		
		for(int i=1; i<serverList.size(); i++)
			serverText.setText(serverText.getText().toString() + ", " + serverList.get(i));
		for(int i=1; i<channelList.size(); i++)
			channelText.setText(channelText.getText().toString() + ", " + channelList.get(i));
	}
}