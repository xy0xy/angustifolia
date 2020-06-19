package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.util;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.TemplateFixer;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;

import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

public class ReferenceMatcher {
	private final Map<String, String> classMappings;
	private final Map.Entry<String, String> entry;
	public ReferenceMatcher(TemplateFixer tf) {
		classMappings = tf.getClassMapping();
		entry = tf.getEntry();
	}

	public String updateTypeName(String owner) {
		if (classMappings.containsKey(NameUtil.internalNameToClassName(owner))) {
			return NameUtil.classNameToInternalName(classMappings.get(NameUtil.internalNameToClassName(owner)));
		}
		return owner;
	}
	public String updateSignatureOrDescriptor(String replace) {
		AtomicReference<String> another = new AtomicReference<>(replace);
		classMappings.forEach((old, newName) -> {
			if (replace != null && replace.contains(NameUtil.classNameToInternalName(old))) {
				another.set(replace.replaceAll(NameUtil.classNameToInternalName(old), NameUtil.classNameToInternalName(newName)));
			}
		});

		return another.get();
	}

	public boolean entryMatches(String name) {
		return NameUtil.classNameToInternalName(name).equals(NameUtil.classNameToInternalName(entry.getValue()));
	}
}
