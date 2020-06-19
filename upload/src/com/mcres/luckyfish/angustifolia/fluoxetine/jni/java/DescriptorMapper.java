package com.mcres.luckyfish.angustifolia.fluoxetine.jni.java;

import org.objectweb.asm.Type;

public class DescriptorMapper {
	private static String getTypeMap(Type type, boolean inArray) {
		if (Type.VOID_TYPE.equals(type)) {
			return "void";
		} else if (Type.BOOLEAN_TYPE.equals(type)) {
			return "jboolean";
		} else if (Type.CHAR_TYPE.equals(type)) {
			return "jchar";
		} else if (Type.BYTE_TYPE.equals(type)) {
			return "jbyte";
		} else if (Type.DOUBLE_TYPE.equals(type)) {
			return "jdouble";
		} else if (Type.FLOAT_TYPE.equals(type)) {
			return "jfloat";
		} else if (Type.INT_TYPE.equals(type)) {
			return "jint";
		} else if (Type.LONG_TYPE.equals(type)) {
			return "jlong";
		} else if (Type.SHORT_TYPE.equals(type)) {
			return "jshort";
		}
		if (type.equals(Type.getType(Class.class))) {
			return "jclass";
		}
		if (type.equals(Type.getType(String.class))) {
			return "jstring";
		}
		if (type.getSort() == Type.ARRAY && !inArray) {
			return getTypeMap(type.getElementType(), true) + "Array";
		}
		return "jobject";
	}

	public static String getTypeMap(Type type) {
		return getTypeMap(type, false);
	}
}
