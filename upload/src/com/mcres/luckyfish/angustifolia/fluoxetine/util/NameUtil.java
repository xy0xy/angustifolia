package com.mcres.luckyfish.angustifolia.fluoxetine.util;

public class NameUtil {
	public static String fileNameToClassName(String fn) {
		String res = fn.replaceAll("\\.class$", "").replaceAll("/", ".");
		if (res.startsWith(".")) {
			res = res.replaceFirst("\\.", "");
		}

		return res;
	}

	public static String classNameToFileName(String cn) {
		String res = cn.replaceAll("\\.", "/");
		return res + ".class";
	}

	public static String classNameToInternalName(String cn) {
		return cn.replaceAll("\\.", "/");
	}

	public static String internalNameToClassName(String in) {
		return in.replaceAll("/", ".");
	}

	public static String replace(String target, String from, String to) {
		if (target == null) {
			return null;
		}
		return target.replaceAll(from, to);
	}

	public static String[] replace(String[] target, String from, String to) {
		if (target == null) {
			return null;
		}
		String[] result = new String[target.length];
		for (int i = 0; i < target.length; i ++) {
			result[i] = replace(target[i], from, to);
		}

		return result;
	}
}
