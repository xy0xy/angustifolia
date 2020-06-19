package com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

class CoreMethodSeeker extends MethodVisitor {
	private final MethodDescriptor md;

	public CoreMethodSeeker(MethodVisitor mv, MethodDescriptor md) {
		super(Opcodes.ASM5, mv);
		this.md = md;
	}

	@Override
	public void visitParameter(String name, int access) {
		super.visitParameter(name, access);
		md.addParameter(new MethodDescriptor.Parameter(name, access));
	}
}
