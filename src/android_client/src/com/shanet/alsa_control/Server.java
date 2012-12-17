// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

public class Server {

	private static final int DEFAULT_PORT = 4242;
	private static final int SUCCESS      = 0;
	private static final int FAILURE      = -1;
	
	public String host;
	public int port;

    private Socket connSock     = null;
    private SocketAddress addr  = null;
    private BufferedReader recv = null;
    private PrintWriter send    = null;
    private Crypto crypto       = null;

    
	public Server() throws SocketException {
		this("", DEFAULT_PORT);
	}
	
	
	public Server(String host) throws SocketException {
		this(host, DEFAULT_PORT);
	}
	
	
	public Server(String host, int port) throws SocketException {
		this.host = host;
		this.port = port;
		
		// Init the socket this way so that the timeout is set before connecting
		addr = new InetSocketAddress(host, port);
		connSock = new Socket();
    	connSock.setSoTimeout(Constants.NETWORK_TIMEOUT);
	}
	

	public void initCrypto(final Key remotePubKey, byte[] iv) throws InvalidKeyException, NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidAlgorithmParameterException {
		if(crypto == null) {
			crypto = new Crypto(remotePubKey, iv);
		}
	}
	
	
    public int connect() throws UnknownHostException, IOException {
    	connSock.connect(addr, Constants.NETWORK_TIMEOUT);
    	
    	if(connSock.isConnected()) {
    	   // Open send stream
           send = new PrintWriter(connSock.getOutputStream(), true);

           // Open read stream
           recv = new BufferedReader(new InputStreamReader(connSock.getInputStream()));
           
           if(send != null && recv != null)
              return SUCCESS;
    	}
    	return FAILURE;
    }
    

    public int send(String data, boolean useEnc) {
    	// Attempt to encrypt data if requested
    	if(useEnc) {
    		if(crypto == null) return FAILURE;
    		
    		try {
				send.println(crypto.aesEncrypt(data));
			} catch (IllegalBlockSizeException ibse) {
				return FAILURE;
			} catch (BadPaddingException bpe) {
				return FAILURE;
			}
    	} else {
	    	// Only try to send if send isn't null
	    	if(send != null) {
	    	   send.println(data);
	    	   return SUCCESS;
	    	}
    	}
    	return FAILURE;
    }

    
    public String receive(boolean useEnc) throws IOException {
    	// Attempt to encrypt data if requested
    	if(useEnc) {
    		if(crypto == null) return null;
    		
    		try {
				return crypto.aesDecrypt(recv.readLine().getBytes());
			} catch (IllegalBlockSizeException ibse) {
				return null;
			} catch (BadPaddingException bpe) {
				return null;
			}
    	} else {
        	// Only try to receive if recv isn't null
        	if(recv != null) {
        		return recv.readLine();
        	}
    	}

    	return null;
    }

    
    public void close() throws IOException {
    	if(send != null)
    		send.close();
    	if(recv != null)
    		recv.close();
    	if(connSock != null)
    		connSock.close();
    }

    
    public boolean isConnected() {
    	return connSock.isConnected();
    }
    
    
    public String getServerIPAddress() {
    	if(isConnected())
    		return connSock.getInetAddress().toString();
    	return "";
    }
    
    
    public String getHost() {
    	return host;
    }
    

    public int getPort() {
    	return port;
    }
    

    public void setPort(int port) {
    	this.port = port;
    }
    

    public void setHost(String host) {
        this.host = host;
    }
    
    // Crypto functions
    public void sendLocalPubKey() {
        send.println(crypto.getLocalKeyPair().getPublic());
    }

    public void receiveRemotePubKey() {
        //crypto.setRemotePubKey(null);
    }

    public void receiveAESKey() {
    	
    }
}