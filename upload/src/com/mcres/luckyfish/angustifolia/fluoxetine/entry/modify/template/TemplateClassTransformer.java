package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.util.ReferenceMatcher;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.util.AccessHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core.CoreMethodList;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class TemplateClassTransformer extends ClassVisitor {
	private final String nameSuffix;
	private final String originalEntry;
	private final int userId;
	private final int resId;
	private final String order;
	private boolean isEntry = false;

	private String currentClass;

	private final boolean visitMethodAndFields;
	private ReferenceMatcher rm;

	private boolean fixed = false;

	private CoreMethodList cml;
	private List<MethodDescriptor> modifiedMethod = null;
	private final List<MethodDescriptor> existMethod = new ArrayList<>();

	private boolean wantHeader = false;

	public TemplateClassTransformer(ClassVisitor classVisitor, String nameSuffix, String originalEntry, int userId, int resId, String order) {
		super(Opcodes.ASM5, classVisitor);
		this.nameSuffix = nameSuffix;
		this.originalEntry = originalEntry;
		this.userId = userId;
		this.resId = resId;
		this.order = order;

		this.visitMethodAndFields = false;
	}

	public TemplateClassTransformer(ClassVisitor classVisitor, String nameSuffix, String originalEntry, int userId, int resId, String order, ReferenceMatcher rm, CoreMethodList cml, List<MethodDescriptor> modifiedMethod) {
		super(Opcodes.ASM5, classVisitor);
		this.nameSuffix = nameSuffix;
		this.originalEntry = originalEntry;
		this.userId = userId;
		this.resId = resId;
		this.order = order;
		this.cml = cml;
		this.modifiedMethod = modifiedMethod;

		this.visitMethodAndFields = true;
		this.rm = rm;
	}

	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		Logger.debug("Name: " + name);

		if ((name.endsWith("/licenseclient/NewEntry") && name.startsWith("cn/mcres/luckyfish/angustifolia/")) || visitMethodAndFields) {
			super.visit(version, access, name, signature, superName, interfaces);
			fixed = false;
		} else { // we don't need to fix the names when we are going to fix the methods and fields.
			if (name.contains("$")) { // We are inner classes.
				String[] nameSlices = name.split("\\$");
				Arrays.setAll(nameSlices, i -> {
					try {
						Integer.parseInt(nameSlices[i]);
						return nameSlices[i];
					} catch (NumberFormatException e) {
						return nameSlices[i] + nameSuffix;
					}
				});
				super.visit(version, access, String.join("$", nameSlices), signature, superName, interfaces);
			} else {
				super.visit(version, access, name + nameSuffix, signature, superName, interfaces);
			}
			fixed = true;
		}
		currentClass = name;
	}

	@Override
	public AnnotationVisitor visitAnnotation(String descriptor, boolean visible) {
		if ("Lcom/mcres/luckyfish/angustifolia/licenseclient/NewEntry;".equals(descriptor)) {
			this.isEntry = true;
		}
		return super.visitAnnotation(descriptor, visible);
	}

	public boolean isEntry() {
		return isEntry;
	}

	@Override
	public void visitInnerClass(String name, String outerName, String innerName, int access) {
		super.visitInnerClass(name, outerName + nameSuffix, innerName + nameSuffix, access);
	}

	@Override
	public void visitEnd() {
		// TODO: Add reference to the real main class.
		if (visitMethodAndFields && isEntry) {
			List<MethodDescriptor> descriptors = cml.listMethod(NameUtil.classNameToInternalName(originalEntry));

			for (MethodDescriptor md : descriptors) {
				if (("<init>".equals(md.getName()) || "<clinit>".equals(md.getName())) && md.getDescriptor().endsWith(")V")) {
						continue;
				}

				boolean skip = false;
				for (MethodDescriptor ctmd : this.existMethod) {
					if (ctmd.equals(md)) {
						skip = true;
						break;
					}
				}

				if (skip) {
					continue;
				}

				insertMethod(md);
			}
		}

		super.visitEnd();
	}

	@Override
	public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
		existMethod.add(new MethodDescriptor(access, name, descriptor, signature, exceptions));
		if (visitMethodAndFields) {
			if (AccessHelper.isNativeMethod(access)) {
				wantHeader = true;
			}
			if (rm.entryMatches(currentClass) && "<clinit>".equals(name)) {
				return new TemplateEntryTransformer(super.visitMethod(access, name, descriptor, signature, exceptions), rm, userId, resId, originalEntry, order);
			} else {
				return new TemplateMethodTransformer(super.visitMethod(access, name, rm.updateSignatureOrDescriptor(descriptor), rm.updateSignatureOrDescriptor(signature), exceptions), rm);
			}
		} else {
			return super.visitMethod(access, name, descriptor, signature, exceptions);
		}
	}

	@Override
	public FieldVisitor visitField(int access, String name, String descriptor, String signature, Object value) {
		if (visitMethodAndFields) {
			return new TemplateFieldTransformer(super.visitField(access, name, rm.updateSignatureOrDescriptor(descriptor), rm.updateSignatureOrDescriptor(signature), value), rm);
		} else {
			return super.visitField(access, name, descriptor, signature, value);
		}
	}

	public boolean isFixed() {
		return fixed;
	}

	private void insertMethod(MethodDescriptor md) {
		int access = AccessHelper.publicizeMethod(md.getModifier());
		access &= ~Opcodes.ACC_NATIVE; // We cannot use native method.

		Type descriptor = Type.getMethodType(md.getDescriptor());
		Type targetDescriptor = Type.getMethodType(Type.getType("Ljava/lang/Object;"), descriptor.getArgumentTypes());

		MethodVisitor mv = new OriginalReferenceMaker(super.visitMethod(access, md.getName(), targetDescriptor.getDescriptor(), null, md.getExceptions()), currentClass, md);

		for (MethodDescriptor.Parameter parameter : md.getParameters()) {
			mv.visitParameter(parameter.getName(), parameter.getAccess());
		}

		mv.visitCode();
		mv.visitMaxs(233, 333);

		mv.visitEnd();

		modifiedMethod.add(md);
	}

	public String getModifiedClassName() {
		return currentClass;
	}

	public String getRawClassName() {
		return currentClass.replace(nameSuffix, "");
	}

	public boolean doesWantHeader() {
		return wantHeader;
	}
}
