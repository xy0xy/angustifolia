package com.mcres.luckyfish.angustifolia.fluoxetine.crypto;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;

public interface Encryptor {
	byte[] getKey();
	byte[] encrypt(byte[] data) throws BadPaddingException, IllegalBlockSizeException;
}
