package com.shanet.alsa_control;

import java.io.IOException;
import java.net.UnknownHostException;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.NumberPicker;
import android.widget.Toast;

public class main extends Activity {
	
	Server server;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        // Configure the UI
        AutoCompleteTextView channelText = (AutoCompleteTextView) findViewById(R.id.channelText);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, R.layout.autocomplete_item, getResources().getStringArray(R.array.channels));
        channelText.setAdapter(adapter);
        
        String[] volSteps = getVolumeSteps();
        
        NumberPicker leftVol = (NumberPicker) findViewById(R.id.leftVolPicker);
        leftVol.setMaxValue(volSteps.length-1);
        leftVol.setMinValue(0);
        leftVol.setWrapSelectorWheel(false);
        leftVol.setDisplayedValues(volSteps);
        
        NumberPicker rightVol = (NumberPicker) findViewById(R.id.rightVolPicker);
        rightVol.setMaxValue(volSteps.length-1);
        rightVol.setMinValue(0);
        rightVol.setWrapSelectorWheel(false);
        rightVol.setDisplayedValues(volSteps);
        
        Button submit = (Button) findViewById(R.id.submit);
        submit.setOnClickListener(getSubmitClickListener());
    }

	
	public String[] getVolumeSteps() {
		String[] vols = new String[21];
		
		for(int i=0; i<vols.length; i++)
			vols[i] = Integer.toString(i*5);
		
		return vols;
	}
	
	
	private OnClickListener getSubmitClickListener() {
		return new OnClickListener() {
			public void onClick(View v) {
				Thread thread = new Thread(new Runnable() {
					public void run() {
						//Looper.prepare();

						server = new Server(((EditText)(findViewById(R.id.serverText))).getText().toString(), 4242);
						try {
							server.connect();
							
							if(server.isConnected()) {
								runOnUiThread(new Runnable() {
									public void run() {
										Toast.makeText(main.this, "CONNECTED", Toast.LENGTH_LONG).show();
									}
								});
							} else {
								runOnUiThread(new Runnable() {
									public void run() {
										Toast.makeText(main.this, "NOT CONNECTED", Toast.LENGTH_LONG).show();
									}
								});
							}
							
							final String reply = "Reply: " + server.receive();
							runOnUiThread(new Runnable() {
								public void run() {
									Toast.makeText(main.this, reply, Toast.LENGTH_LONG).show();
								}
							});
						} catch (UnknownHostException e) {
							runOnUiThread(new Runnable() {
								public void run() {
									Toast.makeText(main.this, "UnknownHostException", Toast.LENGTH_LONG).show();
								}
							});
							e.printStackTrace();
						} catch (IOException e) {
							runOnUiThread(new Runnable() {
								public void run() {
									Toast.makeText(main.this, "IOException", Toast.LENGTH_LONG).show();
								}
							});
							e.printStackTrace();
						}
					}
				});
				
				thread.start();
			}
		};
	}
}