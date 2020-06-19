package com.mcres.luckyfish.angustifolia.fluoxetine.verification;

import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

import java.util.Random;

public class MethodInjector extends MethodVisitor {
	private final Random injectionCheck = new Random();
	private final String className;

	MethodInjector(MethodVisitor methodVisitor, String className) {
		super(Opcodes.ASM5, methodVisitor);
		this.className = className;
	}

	@Override
	public void visitInsn(int opcode) {
		if (Math.abs(injectionCheck.nextInt(10)) <= 8) {
			super.visitInsn(opcode);
			return;
		}
		// the descriptor of int.
		String intsValue = Type.getMethodDescriptor(Type.getType(int.class), Type.getType(int.class), Type.getType(int.class));

		// the descriptor of float
		String floatsValue = Type.getMethodDescriptor(Type.getType(float.class), Type.getType(float.class), Type.getType(float.class));

		// the descriptor of double
		String doublesValue = Type.getMethodDescriptor(Type.getType(double.class), Type.getType(double.class), Type.getType(double.class));

		if (opcode == Opcodes.IADD) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_int_add", intsValue, false);
		} else if (opcode == Opcodes.ISUB) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_int_sub", intsValue, false);
		} else if (opcode == Opcodes.IMUL) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_int_mul", intsValue, false);
		} else if (opcode == Opcodes.IDIV) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_int_div", intsValue, false);
		} else if (opcode == Opcodes.IREM) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_int_reminder", intsValue, false);
		} else if (opcode == Opcodes.FADD) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_float_add", floatsValue, false);
		} else if (opcode == Opcodes.FSUB) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_float_sub", floatsValue, false);
		} else if (opcode == Opcodes.FMUL) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_float_mul", floatsValue, false);
		} else if (opcode == Opcodes.FDIV) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_float_div", floatsValue, false);
		} else if (opcode == Opcodes.DADD) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_double_add", doublesValue, false);
		} else if (opcode == Opcodes.DSUB) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_double_sub", doublesValue, false);
		} else if (opcode == Opcodes.DMUL) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_double_mul", doublesValue, false);
		} else if (opcode == Opcodes.DDIV) {
			this.visitMethodInsn(Opcodes.INVOKESTATIC, className, "fast_double_div", doublesValue, false);
		} else {
			super.visitInsn(opcode);
		}
	}
}
