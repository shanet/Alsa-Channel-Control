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
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Toast;

public class Main extends Activity {
	
	public static ArrayList<String> serverList = null;
	public static ArrayList<String> channelList = null;
	
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
				if(lockCheck.isChecked()) {
					if(seekBar == leftVol)
						rightVol.setProgress(leftVol.getProgress());
					else
						leftVol.setProgress(rightVol.getProgress());
				}
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
                
        // Set the submit button listener
        Button submit = (Button) findViewById(R.id.submit);
        submit.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {								
				// Check for empty server and channel lists
				if(serverList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyServer, Toast.LENGTH_LONG).show();
					return;
				} else if(channelList.isEmpty()) {
					Toast.makeText(Main.this, R.string.emptyChannel, Toast.LENGTH_LONG).show();
					return;
				}
				
				SeekBar leftVol = (SeekBar) findViewById(R.id.leftVolPicker);
				SeekBar rightVol = (SeekBar) findViewById(R.id.rightVolPicker);
				
				int port = Utils.getIntPref(Main.this, "port");
				
				// Construct the info bundle
				Bundle serverInfo = new Bundle();
				serverInfo.putInt("port", port);
				serverInfo.putStringArrayList("channels", channelList);
				serverInfo.putInt("leftVol", leftVol.getProgress());
				serverInfo.putInt("rightVol", rightVol.getProgress());

				// Send the volumes to each channel to each server
				for(int i=0; i<serverList.size(); i++) {
					serverInfo.putString("host", serverList.get(i));
					new Background(Main.this).execute(serverInfo);
				}
			}
		});
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