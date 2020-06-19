package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.util.ReferenceMatcher;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

public class TemplateMethodTransformer extends MethodVisitor {
	private final ReferenceMatcher rm;

	public TemplateMethodTransformer(MethodVisitor methodVisitor, ReferenceMatcher rm) {
		super(Opcodes.ASM5, methodVisitor);
		this.rm = rm;
	}

	@Override
	public void visitTypeInsn(int opcode, String type) {
		super.visitTypeInsn(opcode, rm.updateTypeName(type));
	}

	@Override
	public void visitFieldInsn(int opcode, String owner, String name, String descriptor) {
		String fixedOwner = rm.updateTypeName(owner); // owoer
		super.visitFieldInsn(opcode, fixedOwner, name, rm.updateSignatureOrDescriptor(descriptor));
	}

	@Override
	public void visitMethodInsn(int opcode, String owner, String name, String descriptor, boolean isInterface) {
		super.visitMethodInsn(opcode, rm.updateTypeName(owner), name, rm.updateSignatureOrDescriptor(descriptor), isInterface);
	}

	@Override
	public void visitMultiANewArrayInsn(String descriptor, int numDimensions) {
		Logger.debug("Visiting array: " + descriptor + "->" + rm.updateSignatureOrDescriptor(descriptor));
		super.visitMultiANewArrayInsn(rm.updateSignatureOrDescriptor(descriptor), numDimensions);
	}

	@Override
	public void visitLocalVariable(String name, String descriptor, String signature, Label start, Label end, int index) {
		super.visitLocalVariable(name, rm.updateSignatureOrDescriptor(descriptor), rm.updateSignatureOrDescriptor(signature), start, end, index);
	}
}
