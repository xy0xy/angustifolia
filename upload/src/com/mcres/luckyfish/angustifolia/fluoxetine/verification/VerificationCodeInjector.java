package com.mcres.luckyfish.angustifolia.fluoxetine.verification;

import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

public class VerificationCodeInjector extends ClassVisitor {
	private String className;
	public VerificationCodeInjector(ClassVisitor classVisitor) {
		super(Opcodes.ASM5, classVisitor);
	}

	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		super.visit(version, access, name, signature, superName, interfaces);
		this.className = name;
	}

	@Override
	public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
		if (access == Opcodes.ACC_NATIVE) {
			return super.visitMethod(access, name, descriptor, signature, exceptions);
		}

		return new MethodInjector(super.visitMethod(access, name, descriptor, signature, exceptions), this.className);
	}

	@Override
	public void visitEnd() {
		// the descriptor of int.
		String intsValue = Type.getMethodDescriptor(Type.getType(int.class), Type.getType(int.class), Type.getType(int.class));

		// the descriptor of float
		String floatsValue = Type.getMethodDescriptor(Type.getType(float.class), Type.getType(float.class), Type.getType(float.class));

		// the descriptor of double
		String doublesValue = Type.getMethodDescriptor(Type.getType(double.class), Type.getType(double.class), Type.getType(double.class));


		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_int_add", intsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_int_sub", intsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_int_mul", intsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_int_div", intsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_int_reminder", intsValue, null, null);

		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_float_add", floatsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_float_sub", floatsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_float_mul", floatsValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_float_div", floatsValue, null, null);

		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_double_add", doublesValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_double_sub", doublesValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_double_mul", doublesValue, null, null);
		super.visitMethod(Opcodes.ACC_PRIVATE | Opcodes.ACC_NATIVE | Opcodes.ACC_STATIC, "fast_double_div", doublesValue, null, null);
	}
}
