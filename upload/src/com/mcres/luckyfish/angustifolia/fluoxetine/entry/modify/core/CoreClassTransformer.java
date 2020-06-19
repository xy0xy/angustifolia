package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.util.AccessHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

import java.util.List;

public class CoreClassTransformer extends ClassVisitor {
	private final String entryName;
	private final String newEntry;

	private final List<MethodDescriptor> methodToReplace;

	private String className;
	private boolean entry = false;

	private boolean isTemplateClass = false;

	private boolean wantHeader = false;

	public CoreClassTransformer(ClassVisitor classVisitor, String entryName, String newEntry, List<MethodDescriptor> methodToReplace) {
		super(Opcodes.ASM5, classVisitor);

		this.entryName = entryName;
		this.newEntry = newEntry;
		this.methodToReplace = methodToReplace;
	}

	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		if (name.matches("^cn/mcres/luckyfish/angustifolia/.*/licenseclient/")) {
			this.isTemplateClass = true;
		}
		// clear the extending class.
		// such as, org.bukkit.plugin.java.JavaPlugin
		String superClass = superName;
		if (NameUtil.classNameToInternalName(entryName).equals(name)) {
			superClass = "java/lang/Object";
			entry = true;
		}
		className = name;
		super.visit(version, access, name, signature, superClass, interfaces);
	}

	@Override
	public FieldVisitor visitField(int access, String name, String descriptor, String signature, Object value) {
		if (this.isTemplateClass) {
			return super.visitField(access, name, descriptor, signature, value);
		}
		// replace all reference to the fixed target.
		return super.visitField(access, name, NameUtil.replace(descriptor, NameUtil.classNameToInternalName(entryName), NameUtil.classNameToInternalName(newEntry)), NameUtil.replace(signature, NameUtil.classNameToInternalName(entryName), NameUtil.classNameToInternalName(newEntry)), value);
	}

	@Override
	public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
		if (this.isTemplateClass) {
			Logger.debug("Template class, skipping.");
			return super.visitMethod(access, name, descriptor, signature, exceptions);
		}
		// replace all reference to the fixed target.
		int modifier = access;
		if (entry) {
			modifier = AccessHelper.publicizeMethod(access);
		}

		// It seemed that we cannot read encrypted class now.
//		if (AccessHelper.isNativeMethod(access)) {
//			wantHeader = true;
//		}

		return new CoreMethodTransformer(super.visitMethod(modifier, name, NameUtil.replace(descriptor, NameUtil.classNameToInternalName(entryName), NameUtil.classNameToInternalName(newEntry)), NameUtil.replace(signature, NameUtil.classNameToInternalName(entryName), NameUtil.classNameToInternalName(newEntry)), NameUtil.replace(exceptions, NameUtil.classNameToInternalName(entryName), NameUtil.classNameToInternalName(newEntry))), this.entryName, this.newEntry, (access & Opcodes.ACC_STATIC) == Opcodes.ACC_STATIC, entry, className, name, methodToReplace);
	}

	public boolean doesWantHeader() {
		return wantHeader;
	}

	public String getClassName() {
		return className;
	}
}
