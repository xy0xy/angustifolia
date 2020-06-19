package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core.analysis;

import org.jetbrains.annotations.Nullable;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.Opcodes;

import java.util.HashMap;
import java.util.Map;

public class RawClassAnalyzer extends ClassVisitor {
	private static final Map<String, RawClass> classMap = new HashMap<>();

	public RawClassAnalyzer() {
		super(Opcodes.ASM5);
		classMap.put("java/lang/Object", new RawClass("java/lang/Object", null, null));
	}

	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		classMap.put(name, new RawClass(name, superName, interfaces));
		super.visit(version, access, name, signature, superName, interfaces);
	}

	@Override
	public void visitEnd() {
		super.visitEnd();
	}

	@Nullable
	public static RawClass getClass(String className) {
		if (classMap.containsKey(className)) {
			return classMap.get(className);
		} else {
			try {
				RawClass rc = new RawClass(Class.forName(className.replace('/', '.')));
				classMap.put(rc.getName(), rc);
				return rc;
			} catch (Exception e) {
				return null;
			}
		}
	}
}
