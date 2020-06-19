package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core.analysis.RawClass;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core.analysis.RawClassAnalyzer;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

public class CoreClassWriter extends ClassWriter {
	public CoreClassWriter(ClassReader classReader, int flags) {
		super(classReader, flags);
	}

	@Override
	protected String getCommonSuperClass(String type1, String type2) {
		try {
			RawClass rc1 = RawClassAnalyzer.getClass(type1);
			RawClass rc2 = RawClassAnalyzer.getClass(type2);

			if (rc1 != null && rc2 != null) {
				if (rc1.isAssignableFrom(rc2)) {
					return rc1.getName();
				}
				if (rc2.isAssignableFrom(rc1)) {
					return rc2.getName();
				}

				do {
					rc1 = rc1.getSuperClass();
				} while (!rc1.isAssignableFrom(rc2));
				return rc1.getName();
			} else {
				return super.getCommonSuperClass(type1, type2);
			}
		} catch (TypeNotPresentException e) {
			return "java/lang/Object";
		}
	}
}
