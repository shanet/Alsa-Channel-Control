// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

public abstract class Constants {
	public static final String SETTINGS_FILE = "com.shanet.alsa_control_preferences";
	public static final String LOG_FILE = "alsa_control_log";
		
	public static final int SUCCESS = 0;
	public static final int FAILURE = 1;
	
	public static final int SERVERS = 0;
	public static final int CHANNELS = 1;
	
	public static final String SERVERS_FILE = "servers.txt";
	public static final String CHANNELS_FILE = "channels.txt";
	
	public static final int DEFAULT_PORT = 4242;
	
	public static final int CMD_VOL  = 0;
	public static final int CMD_PLAY = 1;
	public static final int CMD_NEXT = 2;
	public static final int CMD_PREV = 3;
	
	public static final int ABOUT_THIS_APP = 0;
	public static final int CHANGELOG = 1;
	
	public static final int STEP_SIZE = 5;
}
