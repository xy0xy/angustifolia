package com.mcres.luckyfish.angustifolia.fluoxetine.crypto;

import javax.crypto.*;
import javax.crypto.spec.SecretKeySpec;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

public class Blowfish implements Encryptor {
	private final SecretKey key;
	private final Cipher cipher;

	public Blowfish() throws NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException {
		KeyGenerator keygen = KeyGenerator.getInstance("Blowfish");
		this.key = keygen.generateKey();
		SecretKeySpec spec = new SecretKeySpec(getKey(), "Blowfish");
		cipher = Cipher.getInstance("Blowfish");
		cipher.init(Cipher.ENCRYPT_MODE, spec);
	}

	@Override
	public byte[] getKey() {
		return key.getEncoded();
	}

	@Override
	public byte[] encrypt(byte[] data) throws BadPaddingException, IllegalBlockSizeException {
		return cipher.doFinal(data);
	}

	@Override
	protected void finalize() throws Throwable {
		key.destroy();
	}
}
