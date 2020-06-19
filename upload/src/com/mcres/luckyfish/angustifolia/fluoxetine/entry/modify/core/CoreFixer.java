package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.crypto.Encryptor;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core.analysis.RawClassAnalyzer;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.EncryptWrapper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.JarHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;

public class CoreFixer {
	private final JarFile origin;
	private final String oldEntry;
	private final String newEntryClass;

	private final Encryptor encryptor;

	private final List<String> ignoredFiles = new ArrayList<>();
	private final List<MethodDescriptor> methodToReplace;

	private final List<String> classesWantsHeader = new ArrayList<>();

	public CoreFixer(JarFile origin, String oldEntry, String newEntryClass, List<MethodDescriptor> methodToReplace, Encryptor encryptor) {
		this.origin = origin;
		this.oldEntry = oldEntry;
		this.newEntryClass = newEntryClass;
		this.methodToReplace = methodToReplace;
		this.encryptor = encryptor;
	}

	private void fetchAnalyzer() throws IOException {
		Enumeration<JarEntry> enumeration = this.origin.entries();
		// find classes to analyze
		while (enumeration.hasMoreElements()) {
			JarEntry je = enumeration.nextElement();
			if (je.isDirectory()) {
				continue;
			}

			InputStream is = this.origin.getInputStream(je);

			if (je.getName().endsWith(".class")) {
				ClassReader classReader = new ClassReader(is);
				classReader.accept(new RawClassAnalyzer(), ClassReader.SKIP_CODE);
			}
		}
	}

	public void apply(File fileTarget, File templateTarget) throws IOException {
		fetchAnalyzer();

		JarFile template = new JarFile(templateTarget);
		// find classes to transform.
		try (JarOutputStream jos = new JarOutputStream(new FileOutputStream(fileTarget))) {
			Logger.info("Updating core code.");

			JarHelper.iterateJarFile(this.origin, (entry, is) -> {
				if (entry.isDirectory()) {
					return;
				}

				Logger.debug("Visiting file: " + entry.getName());
				// We need to ignore some files because we need to overwrite them.
				if (ignoredFiles.contains(entry.getName())) {
					Logger.debug("Skipping ignored file: " + entry.getName());
					return;
				}

				try {
					if (entry.getName().endsWith(".class")) {
						ClassReader classReader = new ClassReader(is);
						ClassWriter cw = new CoreClassWriter(classReader, ClassWriter.COMPUTE_FRAMES | ClassWriter.COMPUTE_MAXS);

						CoreClassTransformer cct = new CoreClassTransformer(cw, oldEntry, newEntryClass, methodToReplace);
						classReader.accept(cct, ClassReader.EXPAND_FRAMES);

						JarEntry targetEntry = new JarEntry(NameUtil.classNameToFileName(NameUtil.fileNameToClassName(entry.getName())));

						if (encryptor == null) {
							JarHelper.writeEntry(jos, cw.toByteArray(), targetEntry);
						} else {
							JarHelper.writeEntry(jos, EncryptWrapper.encrypt(encryptor, cw.toByteArray()), targetEntry);
						}

						if (cct.doesWantHeader()) {
							classesWantsHeader.add(NameUtil.internalNameToClassName(cct.getClassName()));
						}
					} else {
						JarHelper.copyEntry(jos, entry, is);
					}
				} catch (Exception e) {
					Logger.error("Cannot fix core.", e);
				}
			});

			Logger.info("Injecting entry.");
			// now let's write template content

			JarHelper.iterateJarFile(template, (entry, is) -> {
				try {
					JarHelper.copyEntry(jos, entry, is);
				} catch (IOException e) {
					Logger.error("Cannot copy file", e);
				}
			});
		}
	}

	public void ignoreFile(String fileName) {
		this.ignoredFiles.add(fileName);
	}

	public List<String> getClassesWantsHeader() {
		return Collections.unmodifiableList(classesWantsHeader);
	}
}
