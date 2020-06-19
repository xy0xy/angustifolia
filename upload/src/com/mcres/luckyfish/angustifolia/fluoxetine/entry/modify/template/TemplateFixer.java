package com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.util.ReferenceMatcher;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core.CoreMethodList;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.JarHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.MethodDescriptor;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.*;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;

public class TemplateFixer {
	private static final String[] nameList = new String[] {
			// Maybe chinese character is accepted.
			"氢", "氦",
			"锂", "铍", "硼", "碳", "氮", "氧", "氟", "氖",
			"钠", "镁", "铝", "硅", "磷", "硫", "氯", "氩",
			"钾", "钙", "钪", "钛", "钒", "铬", "锰", "铁", "钴", "镍", "铜", "锌", "镓", "锗", "砷", "硒", "溴", "氪",
			"铷", "锶", "钇", "锆", "铌", "钼", "锝", "钌", "铑", "钯", "银", "镉", "铟", "锡", "锑", "碲", "碘", "氙",
			"铯", "钡", "铪", "钽", "钨", "铼", "锇", "铱", "铂", "金", "汞", "铊", "铅", "铋", "钋", "砹", "氡",
			"镭", "𬬻", "𬭊", "𬭳", "𬭛", "𬭶", "鿏", "𫟼", "𬬭", "鎶", "鉨", "𫓧", "镆", "𫟷",
			"镧", "铈", "镨", "钕", "钷", "钐", "铕", "钆", "铽", "镝", "钬", "铒", "铥", "镱", "镥",
			"锕", "钍", "镤", "铀", "镎", "钚", "镅", "锔", "锫", "锎", "锿", "镄", "钔", "锘", "铹"
	};

	private final JarFile jarFile;
	private final String originalEntry;
	private final int userId;
	private final int resId;
	private final String order;
	private final String nameSuffix;
	private Map.Entry<String, String> entry = null;
	private Map<String, String> classNameMapping = null;

	private final CoreMethodList cml;

	private final List<String> ignoredFiles = new ArrayList<>();
	private final List<MethodDescriptor> modifiedMethod = new LinkedList<>();

	private final Map<String, String> headerNeededClasses = new HashMap<>();

	public TemplateFixer(File targetTemplate, String originalEntry, int userId, int resId, String order, CoreMethodList cml) throws IOException {
		this.jarFile = new JarFile(targetTemplate);
		this.originalEntry = originalEntry;
		this.userId = userId;
		this.resId = resId;
		this.order = order;
		this.cml = cml;

		Random nameGen = new Random();
		try {
			// make it like a random.
			Thread.sleep(Math.abs(nameGen.nextInt()) % 10);
		} catch (InterruptedException ignored) {

		}
		nameGen.nextDouble(); // fresh the rng so that nobody would attack the rng.

		int times = 0;
		while (times == 0) {
			times = Math.abs(nameGen.nextInt(5));
		}

		StringBuilder nameBuilder = new StringBuilder();
		for (int i = 0; i < times; i ++) {
			if (i < times - 1 && nameGen.nextBoolean()) {
				nameBuilder.append(Math.abs(nameGen.nextLong()));
			}
			nameBuilder.append(nameList[Math.abs(Math.abs(nameGen.nextInt(nameList.length)))]);
		}

		this.nameSuffix = nameBuilder.toString();
	}

	public void applyToTarget(File target) throws IOException {
		Logger.info("Building template file...");

		classNameMapping = new HashMap<>();

		// transform classes.
		Map<String, byte[]> finalResult = transformClassContent(transformClassName());
		try (JarOutputStream jos = new JarOutputStream(new FileOutputStream(target))) {

			finalResult.forEach((name, data) -> {
				JarEntry next = new JarEntry(NameUtil.classNameToFileName(name));
				try {
					JarHelper.writeEntry(jos, data, next);
				} catch (IOException e) {
					Logger.error("Cannot write entry", e);
				}
			});

			JarHelper.iterateJarFile(jarFile, (je, is) -> {
				if (je.getName().endsWith(".class") || ignoredFiles.contains(je.getName())) {
					return;
				}

				JarEntry newEntry = new JarEntry(je.getName());
				try {
					JarHelper.copyEntry(jos, newEntry, is);
				} catch (Exception e) {
					Logger.error("Cannot copy content for template.", e);
				}
			});
		}
	}

	private Map<String, byte[]> transformClassName() {
		Map<String, byte[]> transformedClass = new HashMap<>();

		JarHelper.iterateJarFile(jarFile, (entry, is) -> {
			if (!entry.getName().endsWith(".class")) {
				return;
			}
			try {
				ClassReader classReader = new ClassReader(is);
				is.close();
				ClassWriter cw = new ClassWriter(classReader, ClassWriter.COMPUTE_FRAMES | ClassWriter.COMPUTE_MAXS);
				// we need to rename the classes.
				TemplateClassTransformer tct = new TemplateClassTransformer(cw, nameSuffix, originalEntry, userId, resId, order);
				classReader.accept(tct, ClassReader.EXPAND_FRAMES);

				if (tct.isEntry()) {
					classNameMapping.put(NameUtil.fileNameToClassName(entry.getName()), NameUtil.fileNameToClassName(NameUtil.fileNameToClassName(entry.getName()) + nameSuffix));
					this.entry = new HashMap.SimpleEntry<>(NameUtil.fileNameToClassName(entry.getName()), NameUtil.fileNameToClassName(NameUtil.fileNameToClassName(entry.getName()) + nameSuffix));
				} else if (tct.isFixed()) {
					classNameMapping.put(NameUtil.fileNameToClassName(entry.getName()), NameUtil.fileNameToClassName(NameUtil.fileNameToClassName(entry.getName()) + nameSuffix));
				} else {
					classNameMapping.put(NameUtil.fileNameToClassName(entry.getName()), NameUtil.fileNameToClassName(NameUtil.fileNameToClassName(entry.getName())));
				}
				if (tct.isFixed()) {
					transformedClass.put(NameUtil.fileNameToClassName(entry.getName()) + nameSuffix, cw.toByteArray());
				} else {
					transformedClass.put(NameUtil.fileNameToClassName(entry.getName()), cw.toByteArray());
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		});

		return transformedClass;
	}

	private Map<String, byte[]> transformClassContent(Map<String, byte[]> lastTime) {
		Map<String, byte[]> transformedClass = new HashMap<>();
		ReferenceMatcher rm = new ReferenceMatcher(this);

		lastTime.forEach((name, data) -> {
			ClassReader cr = new ClassReader(data);
			ClassWriter cw = new ClassWriter(cr, ClassWriter.COMPUTE_FRAMES);
			// now replace the references.
			TemplateClassTransformer tct = new TemplateClassTransformer(cw, nameSuffix, originalEntry, userId, resId, order, rm, cml, modifiedMethod);
			cr.accept(tct, ClassReader.EXPAND_FRAMES);
			if (tct.doesWantHeader()) {
				headerNeededClasses.put(NameUtil.internalNameToClassName(tct.getModifiedClassName()), NameUtil.internalNameToClassName(tct.getRawClassName()));
			}
			transformedClass.put(name, cw.toByteArray());
		});

		return transformedClass;
	}

	public Map<String, String> getClassMapping() {
		if (classNameMapping == null) {
			throw new IllegalStateException("We haven't modified the template for the protected file.");
		}

		return classNameMapping;
	}

	public Map.Entry<String, String> getEntry() {
		return entry;
	}

	public void ignoreFile(String fileName) {
		this.ignoredFiles.add(fileName);
	}
	public List<MethodDescriptor> getModifiedMethod() {
		return Collections.unmodifiableList(this.modifiedMethod);
	}

	public Map<String, String> getHeaderNeededClasses() {
		return Collections.unmodifiableMap(headerNeededClasses);
	}
}
