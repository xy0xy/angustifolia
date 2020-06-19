package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.util;

import org.objectweb.asm.Opcodes;

public class AccessHelper {
	public static boolean isNativeMethod(int access) {
		return (access & Opcodes.ACC_NATIVE) == Opcodes.ACC_NATIVE;
	}

	public static int publicizeMethod(int access) {
		return (access & ((~Opcodes.ACC_PROTECTED) | (~Opcodes.ACC_PRIVATE))) | Opcodes.ACC_PUBLIC;
	}
}
