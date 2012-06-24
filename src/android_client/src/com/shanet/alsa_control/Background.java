
package com.shanet.alsa_control;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.ArrayList;
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

	private AlertDialog dialog;
	private Context context;
	private Timer timer;
	private TimerTask tt;
	private boolean isDialogShown;
	
	public Background(Context context) {
		this.context = context;
		isDialogShown = false;
		dialog = new ProgressDialog(context);
		
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
			
		// Get the info from the bundle
		Bundle info = params[0];
		String host = info.getString("host");
		int port = info.getInt("port");
		ArrayList<String> channels = info.getStringArrayList("channels");
		int leftVol = info.getInt("leftVol");
		int rightVol = info.getInt("rightVol");
		
		Server server = new Server(host, port);
		String reply;

		try {
			// Connect to the server if not already connected
			if(!server.isConnected()) {
				server.connect();
			}

			// Ensure we're connected now
			if(!server.isConnected()) {
				((Activity)context).runOnUiThread(new Runnable() {
					public void run() {
						DialogUtils.displayErrorDialog(context, R.string.serverConnectErrorTitle, R.string.serverConnectError);
					}
				});
				return Constants.FAILURE;
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
								
			// Check the server sent the welcome message
			if(!server.receive().equals("helo")) {
				((Activity)context).runOnUiThread(new Runnable() {
					public void run() {
						DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
					}
				});
				
				// Something is wrong. Disconnect from the server to retry if this function is called again
				server.close();
				return Constants.FAILURE;
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
				return Constants.FAILURE;
			}
			
			for(int i=0; i<channels.size(); i++) {
				// Send the change volume command
				server.send(channels.get(i) + "=" + leftVol + "," + rightVol);
				
				// Check for confirmation of changed volume
				reply = server.receive();
				if(reply.equals("ok")) {
					((Activity)context).runOnUiThread(new Runnable() {
						public void run() {
							// This is annoying
							//Toast.makeText(context, R.string.volumeSet, Toast.LENGTH_LONG).show();
						}
					});
				// Check if server requested to close connection
				} else if(reply.equals("end")) {
					server.close();
					return Constants.SUCCESS;
				} else {
					((Activity)context).runOnUiThread(new Runnable() {
						public void run() {
							DialogUtils.displayErrorDialog(context, R.string.setVolumeErrorTitle, R.string.setVolumeError);
						}
					});
				}
			}
			
			// Gracefully end the connection
			server.send("end");
			
			// Check for end confirmation
			// Don't actually do anything... at least not yet.
			//if(!server.receive().equals("end"));
			
			// Shut it down
			server.close();
			
		} catch (UnknownHostException uhe) {			
			((Activity)context).runOnUiThread(new Runnable() {
				public void run() {
					dialog.dismiss();
					DialogUtils.displayErrorDialog(context, R.string.unknownHostErrorTitle, R.string.unknownHostError);
				}
			});
			return Constants.FAILURE;
		} catch (IOException ioe) {
			((Activity)context).runOnUiThread(new Runnable() {
				public void run() {
					dialog.dismiss();
					DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
				}
			});
			return Constants.FAILURE;
		} catch (NullPointerException npe) {
			((Activity)context).runOnUiThread(new Runnable() {
				public void run() {
					dialog.dismiss();
					DialogUtils.displayErrorDialog(context, R.string.serverCommErrorTitle, R.string.serverCommError);
				}
			});
			return Constants.FAILURE;
		}
		
		return Constants.SUCCESS;
	}
}