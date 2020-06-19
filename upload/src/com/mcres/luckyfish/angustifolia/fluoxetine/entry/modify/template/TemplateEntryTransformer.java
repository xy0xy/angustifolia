package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.util.ReferenceMatcher;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

public class TemplateEntryTransformer extends TemplateMethodTransformer {
	private final int userId;
	private final int resourceId;
	private final String mainClass;
	private final String order;

	public TemplateEntryTransformer(MethodVisitor methodVisitor, ReferenceMatcher rm, int userId, int resourceId, String mainClass, String order) {
		super(methodVisitor, rm);
		this.userId = userId;
		this.resourceId = resourceId;
		this.mainClass = mainClass;
		this.order = order;
	}

	@Override
	public void visitFieldInsn(int opcode, String owner, String name, String descriptor) {
		super.visitInsn(Opcodes.POP);
		if ("userId".equals(name)) {
			super.visitLdcInsn(userId);
		}
		if ("resourceId".equals(name)) {
			super.visitLdcInsn(resourceId);
		}
		if ("MAIN_CLASS".equals(name)) {
			super.visitLdcInsn(mainClass);
		}
		if ("order".equals(name)) {
			super.visitLdcInsn(order);
		}

		super.visitFieldInsn(opcode, owner, name, descriptor);
	}
}
