package com.mcres.luckyfish.angustifolia.fluoxetine.jni.java;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.NativeMethodDescriptor;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class JavaClassReader extends ClassVisitor {
	private static final List<NativeMethodDescriptor> nativeMethods = new ArrayList<>();

	private String className;

	public JavaClassReader() {
		super(Opcodes.ASM5);
	}

	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		className = name.replaceAll("/", ".");
		super.visit(version, access, name, signature, superName, interfaces);
	}

	@Override
	public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
		if ((access & Opcodes.ACC_NATIVE) == Opcodes.ACC_NATIVE) {
			nativeMethods.add(new NativeMethodDescriptor(access, className, name, descriptor, signature, exceptions));
		}

		return super.visitMethod(access, name, descriptor, signature, exceptions);
	}

	public static List<NativeMethodDescriptor> getNativeMethods() {
		return Collections.unmodifiableList(nativeMethods);
	}
}
