package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core.analysis;

public class RawClass {
	private final String name;
	private final String superName;
	private final String[] interfaces;

	RawClass(String name, String superName, String[] interfaces) {
		this.name = name;
		this.superName = superName;
		this.interfaces = interfaces;
	}

	RawClass(Class<?> clazz) {
		this(clazz.getName().replace('.', '/'), clazz.getSuperclass().getName().replace('.', '/'), fetchClassNames(clazz.getInterfaces()));
	}

	private static String[] fetchClassNames(Class<?>[] cs) {
		String[] s = new String[cs.length];
		for (int i = 0; i < cs.length; i ++) {
			s[i] = cs[i].getName().replace('.', '/');
		}
		return s;
	}

	public String getName() {
		return name;
	}

	public RawClass getSuperClass() {
		return RawClassAnalyzer.getClass(superName);
	}

	public boolean isAssignableFrom(RawClass rawClass) {
		if (this == rawClass) {
			return true;
		}
		RawClass father = RawClassAnalyzer.getClass(rawClass.superName);
		if (father != null && isAssignableFrom(father)) {
			return true;
		}
		if (rawClass.interfaces != null) {
			for (String anInterface : rawClass.interfaces) {
				RawClass interfaceClass = RawClassAnalyzer.getClass(anInterface);
				if (interfaceClass != null && isAssignableFrom(interfaceClass)) {
					return true;
				}
			}
		}
		return false;
	}

}
