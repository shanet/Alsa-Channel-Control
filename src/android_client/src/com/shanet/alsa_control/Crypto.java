// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.security.Key;
import java.security.KeyPair;
import javax.crypto.KeyGenerator;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Security;
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;

import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.InvalidKeyException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.BadPaddingException;
import java.security.InvalidAlgorithmParameterException;

import org.bouncycastle.jce.provider.BouncyCastleProvider;


public class Crypto {

	private static int DEFAULT_RSA_KEYLEN = 2048;
	private static int DEFAULT_AES_KEYLEN = 256;

	private KeyPair localKeyPair;
	private Key remotePubKey;
	private Key aesKey;

	private Cipher rsaEncCipher;
	private Cipher aesEncCipher;

	private Cipher rsaDecCipher;
	private Cipher aesDecCipher;

	public Crypto() throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, InvalidAlgorithmParameterException {
		this(null, null);
	}

	public Crypto(final Key remotePubKey, byte[] iv) throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, InvalidAlgorithmParameterException {
		this(remotePubKey, iv, DEFAULT_RSA_KEYLEN, DEFAULT_AES_KEYLEN);
	}

	public Crypto(final Key remotePubKey, byte[] iv, final int rsaKeyLen, final int aesKeyLen) throws NoSuchAlgorithmException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, InvalidAlgorithmParameterException {
		Security.addProvider(new BouncyCastleProvider());

		rsaEncCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding", "BC");
		aesEncCipher = Cipher.getInstance("AES/CBC/PKCS7Padding", "BC");

		rsaDecCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding", "BC");
		aesDecCipher = Cipher.getInstance("AES/CBC/PKCS7Padding", "BC");

		KeyPairGenerator rsaKeyGen = KeyPairGenerator.getInstance("RSA", "BC");
		KeyGenerator aesKeyGen     = KeyGenerator.getInstance("AES", "BC");

		SecureRandom sRand = new SecureRandom();
		rsaKeyGen.initialize(rsaKeyLen, sRand);
		aesKeyGen.init(aesKeyLen, sRand);

		localKeyPair = rsaKeyGen.generateKeyPair();
		aesKey = aesKeyGen.generateKey();

		// If the iv was not specified, generate a random one
		if(iv == null) {
			iv = new byte[16];
			sRand.nextBytes(iv);
		}
		IvParameterSpec ivSpec = new IvParameterSpec(iv);

		rsaEncCipher.init(Cipher.ENCRYPT_MODE, localKeyPair.getPublic(), sRand);
		aesEncCipher.init(Cipher.ENCRYPT_MODE, aesKey, ivSpec, sRand);

		rsaDecCipher.init(Cipher.DECRYPT_MODE, localKeyPair.getPrivate(), sRand);
		aesDecCipher.init(Cipher.DECRYPT_MODE, aesKey, ivSpec, sRand);
	}
	

	public byte[] rsaEncrypt(final String msg) throws IllegalBlockSizeException, BadPaddingException {
		return rsaEncCipher.doFinal(msg.getBytes());
	}
	

	public byte[] aesEncrypt(final String msg) throws IllegalBlockSizeException, BadPaddingException {
		return aesEncCipher.doFinal(msg.getBytes());
	}
	

	public String rsaDecrypt(final byte[] encMsg) throws IllegalBlockSizeException, BadPaddingException {
		return new String(rsaDecCipher.doFinal(encMsg));
	}
	

	public String aesDecrypt(final byte[] encMsg) throws IllegalBlockSizeException, BadPaddingException {
		return new String(aesDecCipher.doFinal(encMsg));
	}

	
	public Key getRemotePubKey() {
		return remotePubKey;
	}

	
	public void setRemotePubKey(Key remotePubKey) {
		this.remotePubKey = remotePubKey;
	}
	

	public KeyPair getLocalKeyPair() {
		return localKeyPair;
	}
	

	public Key getAESKey() {
		return aesKey;
	}
	

	public void setAESKey(Key aesKey) {
		this.aesKey = aesKey;
	}
}