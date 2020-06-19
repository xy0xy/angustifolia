package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

public class OriginalReferenceMaker extends MethodVisitor {
	private final String className;
	private final MethodDescriptor md;

	private int stackSize = 0;

	public OriginalReferenceMaker(MethodVisitor methodVisitor, String className, MethodDescriptor md) {
		super(Opcodes.ASM5, methodVisitor);

		this.className = className;
		this.md = md;
	}

	@Override
	public void visitCode() {
		int index;
		if ((md.getModifier() & Opcodes.ACC_STATIC) != Opcodes.ACC_STATIC) {
			super.visitInsn(Opcodes.ICONST_0);
			index = 0;
		} else {
			super.visitInsn(Opcodes.ICONST_1);
			index = 1;
		}

		visitLdcInsn(md.getName());

		// I believe that nobody would write a tons of parameters, which is extremely insane.
		visitIntInsn(Opcodes.BIPUSH, md.getParameters().size());
		visitTypeInsn(Opcodes.ANEWARRAY, "java/lang/Object");
		Type methodType = Type.getMethodType(md.getDescriptor());
		for (int i = 0; i < md.getParameters().size(); i ++) {
			visitInsn(Opcodes.DUP);
			visitIntInsn(Opcodes.BIPUSH, i);

			Type argType = methodType.getArgumentTypes()[i];
			visitVarInsn(argType.getOpcode(Opcodes.ILOAD), index ++);
			visitInsn(Opcodes.AASTORE);
		}

		visitMethodInsn(Opcodes.INVOKESTATIC, className, "callMethodFromMain", "(ZLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;", false);

		Type returnType = Type.getReturnType(md.getDescriptor());

		if (returnType != Type.VOID_TYPE) {
			visitInsn(returnType.getOpcode(Opcodes.IRETURN));
		}

		stackSize = index;
	}

	@Override
	public void visitMaxs(int maxStack, int maxLocals) {
		super.visitMaxs(stackSize + 1, stackSize + 1);
	}
}
