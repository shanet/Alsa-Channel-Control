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
import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

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
		} else if(item.getTitle().equals(context.getString(R.string.portMenuLabel))) {
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
}
