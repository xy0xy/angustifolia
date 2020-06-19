package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.util.ReferenceMatcher;
import org.objectweb.asm.AnnotationVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.TypePath;

public class TemplateFieldTransformer extends FieldVisitor {
	private final ReferenceMatcher rm;

	public TemplateFieldTransformer(FieldVisitor fieldVisitor, ReferenceMatcher rm) {
		super(Opcodes.ASM5, fieldVisitor);
		this.rm = rm;
	}

	@Override
	public AnnotationVisitor visitAnnotation(String descriptor, boolean visible) {
		return super.visitAnnotation(rm.updateSignatureOrDescriptor(descriptor), visible);
	}

	@Override
	public AnnotationVisitor visitTypeAnnotation(int typeRef, TypePath typePath, String descriptor, boolean visible) {
		return super.visitTypeAnnotation(typeRef, typePath, rm.updateSignatureOrDescriptor(descriptor), visible);
	}
}
