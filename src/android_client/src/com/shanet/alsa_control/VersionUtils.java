// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.io.File;
import java.io.IOException;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;

public abstract class VersionUtils {
	 
	public static void writeOnFirstRun(Context context) throws IOException {
		// Write a pref for if the app has been run before and default the port
		SharedPreferences.Editor editor = context.getSharedPreferences(Constants.SETTINGS_FILE, 0).edit();
		editor.putBoolean("onFirstRun", false);
		editor.putInt("port", Constants.DEFAULT_PORT);
		editor.commit();
		
		// Add the default channels to the channels file on the first run
		String[] defaultChannels = context.getResources().getStringArray(R.array.channels);
		File channelsFile = new File(new ContextWrapper(context).getFilesDir().getPath() + File.separatorChar + Constants.CHANNELS_FILE);
		for(String channel : defaultChannels)
			Utils.addContent(channelsFile, channel);
	}

	public static void writeVersionCode(Context context) {
		// Update the version code stored in the prefs
		SharedPreferences.Editor editor = context.getSharedPreferences(Constants.SETTINGS_FILE, 0).edit();
		try {
			editor.putInt("versionCode", context.getPackageManager().getPackageInfo(context.getPackageName(), PackageManager.GET_META_DATA).versionCode);
		} catch (NameNotFoundException e) {}
		editor.commit();
	}

	public static boolean compareVersionCode(Context context) {
		// Determines if the app has been updated
		try {
			if(context.getSharedPreferences(Constants.SETTINGS_FILE, 0).getInt("versionCode", 0) != context.getPackageManager().getPackageInfo(context.getPackageName(), PackageManager.GET_META_DATA).versionCode)
				return true;
			else
				return false;
		} catch (NameNotFoundException e) {}
		return false;
	}
}
