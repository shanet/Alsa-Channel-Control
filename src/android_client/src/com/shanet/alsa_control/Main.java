package com.shanet.alsa_control;

import java.io.File;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.ArrayList;

import android.app.Activity;
import android.content.ContextWrapper;
import android.os.Bundle;
import android.os.Looper;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.NumberPicker;
import android.widget.Toast;

public class Main extends Activity {
	
	Server server = null;
	ArrayList<String> serverList = null;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        // Configure the server list text view
        final File serverIOFile = new File(new ContextWrapper(this).getFilesDir().getPath() + File.separatorChar + "servers.txt");
        setupServerTextView(serverIOFile);
        
		// Configure the channel list text view
        AutoCompleteTextView channelText = (AutoCompleteTextView) findViewById(R.id.channelText);
        ArrayAdapter<String> channelAdapter = new ArrayAdapter<String>(this, R.layout.autocomplete_item, getResources().getStringArray(R.array.channels));
        channelText.setAdapter(channelAdapter);
        
        // Get the volume steps to be used in the number pickers below
        /*final String[] volSteps = Utils.getVolumeSteps();
        
        // Configure the left volume picker
        final NumberPicker leftVol = (NumberPicker) findViewById(R.id.leftVolPicker);
        leftVol.setMaxValue(volSteps.length-1);
        leftVol.setMinValue(0);
        leftVol.setWrapSelectorWheel(false);
        leftVol.setDisplayedValues(volSteps);
        
        // Configure the right volume picker
        final NumberPicker rightVol = (NumberPicker) findViewById(R.id.rightVolPicker);
        rightVol.setMaxValue(volSteps.length-1);
        rightVol.setMinValue(0);
        rightVol.setWrapSelectorWheel(false);
        rightVol.setDisplayedValues(volSteps);*/
        
        // Set the submit button listener
        Button submit = (Button) findViewById(R.id.submit);
        submit.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				String server = ((AutoCompleteTextView)findViewById(R.id.serverText)).getText().toString();
				String channel = ((AutoCompleteTextView)findViewById(R.id.channelText)).getText().toString();
				
				// Check if server is already in the server list
				boolean wasFound = false;
				for(int i=0; i<serverList.size(); i++) {
					if(serverList.get(i).equals(server)) {
						wasFound = true;
					}
				}
				
				// Add the new server if it was not found in the server list
				if(!wasFound) {
					try {
						Utils.addServer(serverIOFile, server);
					} catch (IOException e) {
						Utils.displayErrorDialog(Main.this, R.string.writeIOErrorTitle, R.string.writeIOError);
						e.printStackTrace();
					}
					
					// Reload the server list
					setupServerTextView(serverIOFile);
				}
				
				// Init server if necessary
				if(Main.this.server == null) {
					Main.this.server = new Server(server, 4242);
				}
				
				// Pull the current values from the UI and call the change volume method
				//changeVolume(channel, Integer.parseInt(volSteps[leftVol.getValue()]), Integer.parseInt(volSteps[rightVol.getValue()]));
			}
		});
    }
	
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		
		try {
			// If the server is connected, close the connection gracefully
			if(server.isConnected()) {
				server.send("end");
				server.close();
			}
		} catch(IOException ioe) {
			ioe.printStackTrace();
		} catch(NullPointerException npe) {
			// If server wasn't initialized, catch it and move on
		}
	}


	public void changeVolume(final String channel, final int leftVol, final int rightVol) {
    	Thread thread = new Thread(new Runnable() {
			public void run() {
				Looper.prepare();
				
				String reply = "";
				
				try {
					// Connect to the server if not already connected
					if(!server.isConnected()) {
						server.connect();
					
						// Ensure we're connected now
						if(!server.isConnected()) {
							runOnUiThread(new Runnable() {
								public void run() {
									Utils.displayErrorDialog(Main.this, R.string.serverConnectErrorTitle, R.string.serverConnectError);
								}
							});
							return;
						}
						
						// Get the server's welcome message
						reply = server.receive();
						
						// Check the server sent the welcome message
						if(reply.equals("helo")) {
							server.send("helo");
						} else {
							runOnUiThread(new Runnable() {
								public void run() {
									Utils.displayErrorDialog(Main.this, R.string.serverCommErrorTitle, R.string.serverCommError);
								}
							});
							
							// Something is wrong. Disconnect from the server to retry if this function is called again
							server.close();
							return;
						}
					}
					

					
					// Send the change volume command
					server.send(channel + "=" + leftVol + "," + rightVol);
					
					// Check for confirmation of changed volume
					if(server.receive().equals("ok")) {
						runOnUiThread(new Runnable() {
							public void run() {
								Toast.makeText(Main.this, "Volume Set", Toast.LENGTH_LONG).show();
							}
						});
					} else {
						runOnUiThread(new Runnable() {
							public void run() {
								Utils.displayErrorDialog(Main.this, R.string.setVolumeErrorTitle, R.string.setVolumeError);
							}
						});
					}					
					
				} catch (UnknownHostException uhe) {
					runOnUiThread(new Runnable() {
						public void run() {
							Utils.displayErrorDialog(Main.this, R.string.unknownHostErrorTitle, R.string.unknownHostError);
						}
					});
					uhe.printStackTrace();
				} catch (IOException ioe) {
					runOnUiThread(new Runnable() {
						public void run() {
							Utils.displayErrorDialog(Main.this, R.string.serverCommErrorTitle, R.string.serverCommError);
						}
					});
					ioe.printStackTrace();
				} catch (NullPointerException npe) {
					runOnUiThread(new Runnable() {
						public void run() {
							Utils.displayErrorDialog(Main.this, R.string.serverCommErrorTitle, R.string.serverCommError);
						}
					});
					npe.printStackTrace();
				}
			}
		});
		
    	// Let's a go!
		thread.start();
    }
	
	
	public void setupServerTextView(File serverIOFile) {
		// Load known servers into the server list array
		try {
			serverList = Utils.getServers(serverIOFile);
		} catch (IOException e) {
			Utils.displayErrorDialog(this, R.string.readIOErrorTitle, R.string.readIOError);
			e.printStackTrace();
		}
        
        // Configure the server list text view
        AutoCompleteTextView serverText = (AutoCompleteTextView) findViewById(R.id.serverText);
        ArrayAdapter<String> serverAdapter = null;
		serverAdapter = new ArrayAdapter<String>(this, R.layout.autocomplete_item, serverList);
		serverText.setAdapter(serverAdapter);
	}
}