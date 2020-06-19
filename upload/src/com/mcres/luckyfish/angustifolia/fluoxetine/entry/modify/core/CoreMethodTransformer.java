package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.*;

import java.util.List;

public class CoreMethodTransformer extends MethodVisitor {
	private final boolean staticMethod;
	private final String className;
	private final boolean oldEntry;
	private final String methodName;

	private final String oldEntryName;
	private final String newEntry;

	private final List<MethodDescriptor> methodToReplace;

	CoreMethodTransformer(MethodVisitor mv, String oldEntryName, String newEntry, boolean staticMethod, boolean oldEntry, String className, String methodName, List<MethodDescriptor> methodToReplace) {
		super(Opcodes.ASM5, mv);

		this.oldEntryName = oldEntryName;
		this.newEntry = newEntry;
		this.staticMethod = staticMethod;
		this.oldEntry = oldEntry;
		this.className = className;
		this.methodName = methodName;
		this.methodToReplace = methodToReplace;
	}

	@Override
	public void visitFieldInsn(int opcode, String owner, String name, String descriptor) {
		if (owner.contains("$") || (oldEntry && owner.equals(NameUtil.classNameToInternalName(oldEntryName)) && !descriptor.contains(NameUtil.classNameToInternalName(oldEntryName)))) {
			super.visitFieldInsn(opcode, owner, name, descriptor);
		} else {
			String updatedOwner = NameUtil.replace(owner, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry));
			if (updatedOwner.equals(NameUtil.classNameToInternalName(newEntry)) && "instance".equals(name)) {
				if (opcode == Opcodes.PUTSTATIC) {
					super.visitInsn(Opcodes.POP); // ignore instance put, we have put it already.
				} else if (opcode == Opcodes.GETSTATIC) {
					super.visitMethodInsn(Opcodes.INVOKESTATIC, NameUtil.classNameToInternalName(newEntry), "getInstance", Type.getMethodDescriptor(Type.getObjectType(NameUtil.classNameToInternalName(newEntry))), false);
				} else {
					super.visitFieldInsn(opcode, updatedOwner, name, NameUtil.replace(descriptor, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)));
				}
			} else {
				super.visitFieldInsn(opcode, updatedOwner, name, NameUtil.replace(descriptor, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)));
			}
		}
	}

	@Override
	public void visitMethodInsn(int opcode, String owner, String name, String descriptor, boolean isInterface) {
		// replace the super init
		if (opcode == Opcodes.INVOKESPECIAL && "<init>".equals(methodName) && "org/bukkit/plugin/java/JavaPlugin".equals(owner) && "<init>".equals(name)) {
			super.visitInsn(Opcodes.POP);
			super.visitVarInsn(Opcodes.ALOAD, 0);
			super.visitMethodInsn(Opcodes.INVOKESPECIAL, "java/lang/Object", "<init>", "()V", false);
		} else {
			// Ok now we replace the descriptor
			boolean doCast = false;
			Type methodDescriptor = Type.getMethodType(descriptor);
			Type returnType = methodDescriptor.getReturnType();
			// And we need to skip void type or other basic type.
			for (MethodDescriptor md : methodToReplace) {
				if (owner.equals(NameUtil.classNameToInternalName(oldEntryName)) && (!returnType.equals(Type.VOID_TYPE)) && returnType.getOpcode(Opcodes.ILOAD) == Opcodes.ALOAD && md.getName().equals(name) && md.getDescriptor().equals(descriptor)) {
					methodDescriptor = Type.getMethodType(Type.getType("Ljava/lang/Object;"), methodDescriptor.getArgumentTypes());
					doCast = true;
					break;
				}
			}

			if (oldEntry && owner.equals(NameUtil.classNameToInternalName(oldEntryName))) {
				super.visitMethodInsn(opcode, NameUtil.replace(owner, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), name, methodDescriptor.getDescriptor(), isInterface);
			} else {
				super.visitMethodInsn(opcode, NameUtil.replace(owner, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), name, NameUtil.replace(methodDescriptor.getDescriptor(), NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), isInterface);
			}

			if (doCast) {
				this.visitTypeInsn(Opcodes.CHECKCAST, returnType.getInternalName());
			}
		}
	}

	@Override
	public void visitInvokeDynamicInsn(String name, String descriptor, Handle bootstrapMethodHandle, Object... bootstrapMethodArguments) {
		for (int i = 0; i < bootstrapMethodArguments.length; i ++) {
			if (bootstrapMethodArguments[i] instanceof Type) {
				Type type = (Type) bootstrapMethodArguments[i];
				if (type.getInternalName().equals(NameUtil.classNameToInternalName(oldEntryName))) {
					type = Type.getObjectType(NameUtil.classNameToInternalName(newEntry));
					bootstrapMethodArguments[i] = type;
				}
			}
		}
		super.visitInvokeDynamicInsn(name, NameUtil.replace(descriptor, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), bootstrapMethodHandle, bootstrapMethodArguments);
	}

	@Override
	public void visitTypeInsn(int opcode, String type) {
		if (type.contains("$")) {
			super.visitTypeInsn(opcode, type);
			return;
		}
		super.visitTypeInsn(opcode, NameUtil.replace(type, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)));
	}

	@Override
	public void visitVarInsn(int opcode, int i) {
		if (opcode != Opcodes.ALOAD || i != 0 || !oldEntry || staticMethod) {
			super.visitVarInsn(opcode, i);
		} else {
			visitMethodInsn(Opcodes.INVOKESTATIC, NameUtil.classNameToInternalName(newEntry), "getInstance", Type.getMethodDescriptor(Type.getObjectType(NameUtil.classNameToInternalName(newEntry))), false);
		}
	}

	@Override
	public void visitMultiANewArrayInsn(String descriptor, int numDimensions) {
		super.visitMultiANewArrayInsn(NameUtil.replace(descriptor, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), numDimensions);
	}

	@Override
	public void visitLocalVariable(String name, String descriptor, String signature, Label start, Label end, int index) {
		if (descriptor.contains(NameUtil.classNameToInternalName(oldEntryName)) && "this".equals(name) && className.contains("$")) {
			Type methodDescriptor = Type.getMethodType(Type.getType(NameUtil.classNameToInternalName(newEntry)));
			super.visitMethodInsn(Opcodes.INVOKESTATIC, NameUtil.classNameToInternalName(newEntry), "getInstance", methodDescriptor.getDescriptor(), false);
		} else if (oldEntry && "this".equals(name)) {
			super.visitLocalVariable(name, descriptor, signature, start, end, index);
		} else {
			super.visitLocalVariable(name, NameUtil.replace(descriptor, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), NameUtil.replace(signature, NameUtil.classNameToInternalName(oldEntryName), NameUtil.classNameToInternalName(newEntry)), start, end, index);
		}
	}
}
