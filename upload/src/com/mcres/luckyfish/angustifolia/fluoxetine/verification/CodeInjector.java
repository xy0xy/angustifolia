package com.mcres.luckyfish.angustifolia.fluoxetine.verification;

import java.io.File;
import java.io.IOException;
import java.util.Enumeration;
import java.util.LinkedList;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class CodeInjector {
	private final File injectFile;

	public CodeInjector(String pathToJar) {
		injectFile = new File(pathToJar);
	}

	public void inject() {

	}

	private List<VerificationCodeInjector> injectClasses() {
		List<VerificationCodeInjector> result;
		try {
			result = new LinkedList<>();
			JarFile jar = new JarFile(injectFile);

			Enumeration<JarEntry> entries = jar.entries();
			while (entries.hasMoreElements()) {
				JarEntry je = entries.nextElement();
			}
		} catch (IOException e) {
			result = null;
		}


		return result;
	}
}
