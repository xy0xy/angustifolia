package com.mcres.luckyfish.angustifolia.fluoxetine.util;

import com.mcres.luckyfish.angustifolia.fluoxetine.crypto.Encryptor;
import org.apache.commons.io.IOUtils;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;

public class EncryptWrapper {
	private static final short CONSTANT_Class = 7;
	private static final short CONSTANT_Fieldref = 9;
	private static final short CONSTANT_Methodref = 10;
	private static final short CONSTANT_InterfaceMethodref = 11;
	private static final short CONSTANT_String = 8;
	private static final short CONSTANT_Integer = 3;
	private static final short CONSTANT_Float = 4;
	private static final short CONSTANT_Long = 5;
	private static final short CONSTANT_Double = 6;
	private static final short CONSTANT_NameAndType = 12;
	private static final short CONSTANT_Utf8 = 1;
	private static final short CONSTANT_MethodHandle = 15;
	private static final short CONSTANT_MethodType = 16;
	private static final short CONSTANT_InvokeDynamic = 18;

	public static byte[] encrypt(Encryptor encryptor, byte[] data) {
		// Ok let's start skip the class header.
		ByteArrayInputStream bais = new ByteArrayInputStream(data);
		ByteArrayOutputStream baos = new ByteArrayOutputStream();

		try {
			byte[] buf = new byte[4];

			// magic
			bais.read(buf);
			baos.write(buf);

			// major ver
			buf = new byte[2];
			bais.read(buf);
			baos.write(buf);

			// minor ver
			bais.read(buf);
			baos.write(buf);

			// constant pool count
			bais.read(buf);
			baos.write(buf);
			int constantPoolCount = new DataInputStream(new ByteArrayInputStream(buf)).readUnsignedShort();

			// constant pool.
			readConstantPool(constantPoolCount - 1, bais, baos);

			// access flag
			buf = new byte[2];
			bais.read(buf);
			baos.write(buf);

			// this class
			bais.read(buf);
			baos.write(buf);

			// super class
			bais.read(buf);
			baos.write(buf);

			// interface amount
			bais.read(buf);
			baos.write(buf);
			int interfaceCount = new DataInputStream(new ByteArrayInputStream(buf)).readUnsignedShort();

			buf = new byte[interfaceCount * 2];
			bais.read(buf);
			baos.write(buf);

			// ok the remaining is what we need to encrypt
			ByteArrayOutputStream encryptByteArrayOutputStream = new ByteArrayOutputStream();
			IOUtils.copy(bais, encryptByteArrayOutputStream);

			byte[] encryptTarget = encryptByteArrayOutputStream.toByteArray();
			Logger.debug("Encrypting data, length=" + encryptTarget.length);
			byte[] encrypted = encryptor.encrypt(encryptTarget);

			baos.write(encrypted);

			return baos.toByteArray();
		} catch (Exception e) {
			Logger.error("Error while encrypting the jar.");
			e.printStackTrace();
		}
		return data;
	}

	private static void readConstantPool(int constantPoolCount, ByteArrayInputStream bais, ByteArrayOutputStream baos) throws IOException {
		byte[] buf;
		for (int i = 0; i < constantPoolCount; i ++) {
			buf = new byte[1];
			bais.read(buf);
			baos.write(buf);

			int size = 0;
			int tag;

			tag = new DataInputStream(new ByteArrayInputStream(buf)).readUnsignedByte();

			switch (tag) {
				case CONSTANT_Class:
				case CONSTANT_String:
				case CONSTANT_MethodType:
					size = 2;
					break;
				case CONSTANT_Fieldref:
				case CONSTANT_Methodref:
				case CONSTANT_InterfaceMethodref:
				case CONSTANT_Integer:
				case CONSTANT_Float:
				case CONSTANT_NameAndType:
				case CONSTANT_InvokeDynamic:
					size = 4;
					break;
				case CONSTANT_Long:
				case CONSTANT_Double:
					size = 8;
					break;
				case CONSTANT_Utf8:
					buf = new byte[2];

					bais.read(buf);
					baos.write(buf);
					int length = new DataInputStream(new ByteArrayInputStream(buf)).readUnsignedShort();
					buf = new byte[1];
					for (int j = 0; j < length; j ++) {
						bais.read(buf);
						baos.write(buf);
					}

					size = 0;
					break;
				case CONSTANT_MethodHandle:
					size = 3;
					break;
			}
			// we already processed the string data.
			if (size > 0) {
				buf = new byte[1];
				for (int j = 0; j < size; j ++) {
					bais.read(buf);
					baos.write(buf);
				}
			}
		}
	}
}
