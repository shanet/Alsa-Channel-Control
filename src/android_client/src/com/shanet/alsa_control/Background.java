
package com.shanet.alsa_control;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Looper;

public class Background extends AsyncTask<Bundle, Integer, Integer> {

	private static HashMap<String, Server> servers;
	
	private AlertDialog dialog;
	private Context context;
	private Timer timer;
	private TimerTask tt;
	private boolean isDialogShown;
	
	static {
		servers = new HashMap<String, Server>();
	}
	
	public Background(Context context) {
		this.context  = context;
		isDialogShown = false;
		dialog        = new ProgressDialog(context);
		
		// Only show the connecting and communicating with server dialogs if the bg thread runs for more than 200ms.
		// Time is specified in onPreExecute() below.
		timer = new Timer();
		tt = new TimerTask() {
			@Override
			public void run() {
				Looper.prepare();
				((Activity)Background.this.context).runOnUiThread(new Runnable() {
					public void run() {
						dialog = ProgressDialog.show(Background.this.context, "", Background.this.context.getString(R.string.connServer));
					}
				});
				isDialogShown = true;
			}
		};
	}	    

	protected void onPreExecute() {
		timer.schedule(tt, 200);
	}

	protected void onPostExecute(Integer result) {
		dialog.dismiss();
		tt.cancel();
	}

	protected Integer doInBackground(Bundle...params) {		
		// There should only be 1 bundle
		if(params.length != 1) {
			((Activity)context).runOnUiThread(new Runnable() {
				public void run() {
					DialogUtils.displayErrorDialog(context, R.string.malformedDataErrorTitle, R.string.malformedDataError);
				}
			});
			return Constants.FAILURE;
		}
			
		// Get the UI info from the bundle
		Bundle info = params[0];
		int command = info.getInt("command", -1);
		int port = info.getInt("port");
		ArrayList<String> hosts = info.getStringArrayList("hosts");
		ArrayList<String> channels = info.getStringArrayList("channels");
		int leftVol = info.getInt("leftVol");
		int rightVol = info.getInt("rightVol");
		boolean useEnc = false;
		
		// Send the requested commands to the server in one swoop
		Server server;
		String reply;
		
		for(final String host : hosts) {
			// Create a new server for the host if it doesn't already exist
			if(!servers.containsKey(host)) {
				servers.put(host, new Server(host, port));
			}
			
			// Get the server object for the given host
			server = servers.get(host);
		
			try {
				// Connect to the server if not already connected
				if(!server.isConnected()) {
					server.connect();
					
					// Ensure we're connected now
					if(!server.isConnected()) {
						((Activity)context).runOnUiThread(new Runnable() {
							public void run() {
								DialogUtils.displayErrorDialog(context, R.string.serverConnectErrorTitle, R.string.serverConnectError);
							}
						});
						continue;
					}
					
					// This is the first connection with the server so we must perform the handshake
					// Check the server sent the welcome message
					if(!server.receive(false).equals("helo")) {
						((Activity)context).runOnUiThread(new Runnable() {
							public void run() {
								DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
							}
						});
						
						// Something is wrong
						server.send("end", false);
						server.close();
						continue;
					}
		
					// Tell the server if we're using encryption or not
					// For now, this client doesn't support encryption
					server.send("noenc", false);
					
					// Check for the ready command	
					if(!server.receive(false).equals("redy")) {
						((Activity)context).runOnUiThread(new Runnable() {
							public void run() {
								DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
							}
						});
						
						// Something is wrong. We can't continue without a ready command
						server.send("end", false);
						server.close();
						continue;
					}
				}
				
				// Change the dialog from connecting to server to communicating with server if dialogs are shown
				((Activity)context).runOnUiThread(new Runnable() {
					public void run() {
						if(isDialogShown) {
							dialog.dismiss();
							dialog = ProgressDialog.show(context, "", context.getString(R.string.commServer));
						}
					}
				});
				
				switch(command) {
					case Constants.CMD_VOL:
						// Send the volume command
						server.send("vol", useEnc);

						// Check for okay
						reply = server.receive(useEnc);
						if(reply.equals("err")) {
							((Activity)context).runOnUiThread(new Runnable() {
								public void run() {
									DialogUtils.displayErrorDialog(context, R.string.setVolumeErrorTitle, R.string.setVolumeError);
								}
							});
							continue;
						// Check if server requested to close connection
						} else if(reply.equals("end")) {
							server.send("bye", useEnc);
							server.close();
							continue;
						}
						
						// Send how many channels we're about to send
						server.send(Integer.valueOf(channels.size()).toString(), useEnc);
						
						// Check for okay
						reply = server.receive(useEnc);
						if(reply.equals("err")) {
							((Activity)context).runOnUiThread(new Runnable() {
								public void run() {
									DialogUtils.displayErrorDialog(context, R.string.setVolumeErrorTitle, R.string.setVolumeError);
								}
							});
							continue;
						// Check if server requested to close connection
						} else if(reply.equals("end")) {
							server.send("bye", useEnc);
							server.close();
							continue;
						}
						
						// Send the volume command						
						for(int i=0; i<channels.size(); i++) {							
							// Send the volume data
							server.send(channels.get(i) + "=" + leftVol + "," + rightVol, useEnc);
							
							// Check for confirmation of changed volume
							reply = server.receive(useEnc);
							if(reply.equals("err")) {
								((Activity)context).runOnUiThread(new Runnable() {
									public void run() {
										DialogUtils.displayErrorDialog(context, R.string.setVolumeErrorTitle, R.string.setVolumeError);
									}
								});
								continue;
							// Check if server requested to close connection
							} else if(reply.equals("end")) {
								server.send("bye", useEnc);
								server.close();
								continue;
							}
						}
						break;
						
					case Constants.CMD_PLAY:
					case Constants.CMD_NEXT:
					case Constants.CMD_PREV:
						// Send the media command
						switch(command) {
							case Constants.CMD_PLAY:
								server.send("play", useEnc);
								break;
							case Constants.CMD_NEXT:
								server.send("next", useEnc);
								break;
							case Constants.CMD_PREV:
								server.send("prev", useEnc);
								break;
						}
						
						// Check for errors
						reply = server.receive(useEnc);
						if(reply.equals("err")) {
							((Activity)context).runOnUiThread(new Runnable() {
								public void run() {
									DialogUtils.displayErrorDialog(context, R.string.mediaCommandErrorTitle, R.string.mediaCommandError);
								}
							});
						// Check if server requested to close connection
						} else if(reply.equals("end")) {
							server.send("bye", useEnc);
							server.close();
							continue;
						}
						break;
						
					default:
						server.close();
						return Constants.FAILURE;
				}
			} catch (UnknownHostException uhe) {			
				((Activity)context).runOnUiThread(new Runnable() {
					public void run() {
						dialog.dismiss();
						DialogUtils.displayErrorDialog(context, R.string.unknownHostErrorTitle, R.string.unknownHostError);
					}
				});
				
				// Shut down the server
				try {
					if(server != null) server.close();
				} catch (IOException ioe) {}
			} catch (IOException ioe) {
				ioe.printStackTrace();
	
				((Activity)context).runOnUiThread(new Runnable() {
					public void run() {
						dialog.dismiss();
						DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
					}
				});
				
				// Shut down the server
				try {
					if(server != null) server.close();
				} catch (IOException ioe2) {}
			} catch (NullPointerException npe) {
				((Activity)context).runOnUiThread(new Runnable() {
					public void run() {
						dialog.dismiss();
						DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
					}
				});
				
				// Shut down the server
				try {
					if(server != null) server.close();
				} catch (IOException ioe) {}
			}
		}
		
		return Constants.SUCCESS;
	}
	
	public static void closeAllServers(boolean useEnc) {
		Collection<Server> servers = Background.servers.values();
		if(servers == null) return;
		
		Iterator<Server> iter = servers.iterator();
		Server curServer;
		while(iter.hasNext()) {
			try {
				// If the server is not null and still connected, gracefully end the connection by sending the end command to the server
				curServer = iter.next();
				if(curServer != null) {
					if(curServer.isConnected()) {
						curServer.send("end", useEnc);
					}
					
					curServer.close();
				}
			} catch (IOException ioe) {}
		}
	}
}