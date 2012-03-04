package com.shanet.alsa_control;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

public class Utils {
	
	public static String[] getVolumeSteps() {
		// Get all the multiples of 5 between 0-100
		// The range is determined by the size of the array
		String[] vols = new String[21];
		
		for(int i=0; i<vols.length; i++)
			vols[i] = Integer.toString(i*5);
		
		return vols;
	}
	
	
	public static ArrayList<String> getServers(File inputFile) throws IOException {
		ArrayList<String> servers = new ArrayList<String>();

		try {
			BufferedReader in = new BufferedReader(new FileReader(inputFile));
			
			String input;
			while((input = in.readLine()) != null)
				servers.add(input);
			
			in.close();
		} catch (FileNotFoundException fnfe) {
			// If the file isn't found, just return the empty array
		}
		
		return servers;
	}
	
	
	public static void addServer(File outputFile, String server) throws IOException {
		BufferedWriter out = new BufferedWriter(new FileWriter(outputFile));
		out.write(server);
		out.close();
	}
	
	
	 public static void displayErrorDialog(Context context, int title, int message) {
		 // This function displays error messages with a given title and message.
		 // It cuts down on duplicate code.
			new AlertDialog.Builder(context)
				.setTitle(title)
				.setMessage(message)
				.setIcon(R.drawable.error_icon)
				.setPositiveButton(R.string.okay, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {}
				})
				.show();
	 }
}
