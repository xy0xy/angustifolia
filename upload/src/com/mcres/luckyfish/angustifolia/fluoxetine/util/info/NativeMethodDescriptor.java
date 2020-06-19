package com.mcres.luckyfish.angustifolia.fluoxetine.util.info;

import org.objectweb.asm.Opcodes;

public class NativeMethodDescriptor extends MethodDescriptor {
	private final String className;

	public NativeMethodDescriptor(int modifier, String className, String name, String descriptor, String signature, String[] exceptions) {
		super(modifier, name, descriptor, signature, exceptions);

		if ((modifier & Opcodes.ACC_NATIVE) != Opcodes.ACC_NATIVE) {
			throw new IllegalArgumentException("The provided method must be a native method!");
		}

		this.className = className;
	}

	public String getClassName() {
		return className;
	}

	public String getGeneratedClassName() {
		return normalizeName(className);
	}

	public String getGeneratedMethodName() {
		return normalizeName(getName());
	}

	public static String normalizeName(String name) {
		String stage1 = name.replaceAll("_", "_1")
				.replaceAll(";", "_2")
				.replaceAll("\\[", "_3")
				.replaceAll("\\.", "_");

		StringBuilder sb = new StringBuilder();

		for (int i = 0; i < stage1.length(); i ++) {
			char c = stage1.charAt(i);

			if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
				sb.append(c);
			} else {
				sb.append(String.format("_0%04x", (int) c));
			}
		}

		return sb.toString();
	}
}
