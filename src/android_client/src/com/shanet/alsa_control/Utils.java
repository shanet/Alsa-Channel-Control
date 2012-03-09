// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.os.Looper;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

public abstract class Utils {	

	public static ArrayList<String> getContent(File inputFile) throws IOException {
		ArrayList<String> content = new ArrayList<String>();

		try {
			BufferedReader in = new BufferedReader(new FileReader(inputFile));
			
			String input;
			while((input = in.readLine()) != null)
				content.add(input);
			
			in.close();
		} catch (FileNotFoundException fnfe) {
			// If the file isn't found, just return the empty array
		}
		
		return content;
	}
	
	
	public static void addContent(File outputFile, String content) throws IOException {
		// Second arg in FileWriter denotes append to file
		BufferedWriter out = new BufferedWriter(new FileWriter(outputFile, true));
		out.write(content);
		out.newLine();
		out.close();
	}
	
	
	public static int removeContent(Context context, File ioFile, String content) throws IOException {
		BufferedReader in = null;
		BufferedWriter out = null;
		
		try {
			File tmpFile = new File(new ContextWrapper(context).getFilesDir().getPath() + File.separatorChar + "tmp.txt");
			
			in = new BufferedReader(new FileReader(ioFile));
			out = new BufferedWriter(new FileWriter(tmpFile, true));
			
			String input;
			while((input = in.readLine()) != null) {
				if(input.equals(content)) {
					continue;
				}
				
				out.write(input);
				out.newLine();
			}
			
			//ioFile.delete();
			tmpFile.renameTo(ioFile);
			
		} catch (FileNotFoundException fnfe) {
			return Constants.FAILURE;
		} finally {
			if(in != null && out != null) {
				in.close();
				out.close();
			}
		}

		return Constants.SUCCESS;
	}
	
	
	public static boolean onCreateOptionsMenu(Context context, Menu menu) {
	    MenuInflater inflater = ((Activity)context).getMenuInflater();
	    inflater.inflate(R.menu.options_menu, menu);
	    return true;
	}
	
	
    public static boolean onOptionsItemSelected(Context context, MenuItem item) {
		if(item.getTitle().equals(context.getString(R.string.about))) {
			DialogUtils.showAboutDialog(context, Constants.ABOUT_THIS_APP);
			return true;
		} else if(item.getTitle().equals(context.getString(R.string.changelogTitle))) {
			DialogUtils.showAboutDialog(context, Constants.CHANGELOG);
			return true;
		} else if(item.getTitle().equals(context.getString(R.string.portLabel))) {
			DialogUtils.showEditPortDialog(context);
			return true;
		}
		
		return false;
    }
    
    
	public static void writeIntPref(Context context, String key, int data) {
		SharedPreferences.Editor editor = context.getSharedPreferences(Constants.SETTINGS_FILE, 0).edit();
		editor.putInt(key, data);
		editor.commit();
	}
	
	
	public static int getIntPref(Context context, String key) {
		return context.getSharedPreferences(Constants.SETTINGS_FILE, 0).getInt(key, -1);
	}
	 
	 
	public static void changeVolume(final Context context, final String host, final int port, final ArrayList<String> channels, final int leftVol, final int rightVol) {
    	Thread thread = new Thread(new Runnable() {
			public void run() {
				Looper.prepare();
				
				Server server = new Server(host, port);
				
				try {
					server.connect();
					
					// Ensure we're connected now
					if(!server.isConnected()) {
						((Activity)context).runOnUiThread(new Runnable() {
							public void run() {
								DialogUtils.displayErrorDialog(context, R.string.serverConnectErrorTitle, R.string.serverConnectError);
							}
						});
						return;
					}
					
					// Get the server's welcome message
					
					// Check the server sent the welcome message
					if(!server.receive().equals("helo")) {
						((Activity)context).runOnUiThread(new Runnable() {
							public void run() {
								DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
							}
						});
						
						// Something is wrong. Disconnect from the server to retry if this function is called again
						server.close();
						return;
					}

					// Say hello back to complete the handshake
					server.send("helo");
					
					// Check for the ready command	
					if(!server.receive().equals("redy") ) {
						((Activity)context).runOnUiThread(new Runnable() {
							public void run() {
								DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
							}
						});
						
						// Something is wrong. We can't continue without a ready command
						server.close();
						return;
					}
					
					for(int i=0; i<channels.size(); i++) {
						// Send the change volume command
						server.send(channels.get(i) + "=" + leftVol + "," + rightVol);
						
						// Check for confirmation of changed volume
						if(server.receive().equals("ok")) {
							((Activity)context).runOnUiThread(new Runnable() {
								public void run() {
									Toast.makeText(context, R.string.volumeSet, Toast.LENGTH_LONG).show();
								}
							});
						} else {
							((Activity)context).runOnUiThread(new Runnable() {
								public void run() {
									DialogUtils.displayErrorDialog(context, R.string.setVolumeErrorTitle, R.string.setVolumeError);
								}
							});
						}
					}
					
					server.close();
					
				} catch (UnknownHostException uhe) {
					((Activity)context).runOnUiThread(new Runnable() {
						public void run() {
							DialogUtils.displayErrorDialog(context, R.string.unknownHostErrorTitle, R.string.unknownHostError);
						}
					});
				} catch (IOException ioe) {
					((Activity)context).runOnUiThread(new Runnable() {
						public void run() {
							DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
						}
					});
				} catch (NullPointerException npe) {
					((Activity)context).runOnUiThread(new Runnable() {
						public void run() {
							DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
						}
					});
				}
			}
		});
		
    	// Let's a go!
		thread.start();
    }
}
