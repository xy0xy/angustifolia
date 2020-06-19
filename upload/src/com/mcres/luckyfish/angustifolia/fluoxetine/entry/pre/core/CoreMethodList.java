package com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;

import java.util.*;

public class CoreMethodList {
	private final Map<String, List<MethodDescriptor>> classMethodMap = new HashMap<>();

	public void putMethod(String className, MethodDescriptor methodDescriptor) {
		if (!classMethodMap.containsKey(className)) {
			classMethodMap.put(className, new LinkedList<>());
		}

		List<MethodDescriptor> descriptors = classMethodMap.get(className);
		descriptors.add(methodDescriptor);
	}

	public MethodDescriptor findMethod(String className, String methodName, String descriptor, String signature) {
		if (!classMethodMap.containsKey(className)) {
			return null;
		}

		for (MethodDescriptor cmd : classMethodMap.get(className)) {
			if (cmd.getName().equals(methodName) && cmd.getDescriptor().equals(descriptor) && Objects.equals(signature, cmd.getSignature())) {
				return cmd;
			}
		}

		return null;
	}

	public List<MethodDescriptor> listMethod(String className) {
		return Collections.unmodifiableList(classMethodMap.get(className));
	}
}
