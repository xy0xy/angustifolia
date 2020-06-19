package com.mcres.luckyfish.angustifolia.fluoxetine.crypto;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import javax.crypto.*;
import javax.crypto.spec.SecretKeySpec;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.Security;

public class IDEA implements Encryptor {
	static {
		Security.addProvider(new BouncyCastleProvider());
	}

	private final SecretKey key;
	private final Cipher cipher;

	public IDEA() throws InvalidKeyException, NoSuchPaddingException, NoSuchAlgorithmException {
		KeyGenerator keygen = KeyGenerator.getInstance("IDEA");
		this.key = keygen.generateKey();
		SecretKeySpec spec = new SecretKeySpec(getKey(), "IDEA");
		cipher = Cipher.getInstance("IDEA");
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
