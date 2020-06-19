package com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

class CoreClassSeeker extends ClassVisitor {
	private static final CoreMethodList cml = new CoreMethodList();

	private String className;
	public CoreClassSeeker() {
		super(Opcodes.ASM5);
	}

	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		super.visit(version, access, name, signature, superName, interfaces);
		this.className = name;
	}

	@Override
	public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
		cml.putMethod(className, new MethodDescriptor(access, name, descriptor, signature, exceptions));
		return new CoreMethodSeeker(super.visitMethod(access, name, descriptor, signature, exceptions), cml.findMethod(className, name, descriptor, signature));
	}

	public static CoreMethodList getCoreMethodList() {
		return cml;
	}
}
