package com.mcres.luckyfish.angustifolia.fluoxetine.entry;

import com.mcres.luckyfish.angustifolia.fluoxetine.Fluoxetine;
import com.mcres.luckyfish.angustifolia.fluoxetine.crypto.Encryptor;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.core.CoreFixer;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.modify.template.TemplateFixer;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core.CoreDataCollector;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.JarHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.NameUtil;
import org.yaml.snakeyaml.Yaml;

import java.io.*;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;

public class PluginEntryFixer implements EntryFixer {
	private static final Yaml parser = new Yaml();

	private final JarFile fixTarget;
	private JarEntry pluginEntry;
	private final File target;
	private File secondaryFile;

	private final Encryptor encryptor;

	private Map<String, Object> pluginMap;
	private String fixedEntry;

	private final Map<String, String> headerNeededClasses = new HashMap<>();
	private final Map<String, String> classMap = new HashMap<>();

	public PluginEntryFixer(JarFile jar, File target, Encryptor encryptor) {
		this.fixTarget = jar;
		this.target = target;
		this.encryptor = encryptor;
	}

	@Override
	public void findEntry() throws IOException {
		JarEntry je = fixTarget.getJarEntry("plugin.yml");
		InputStream jis = fixTarget.getInputStream(je);
		pluginMap = parser.load(jis);
		jis.close();
		String entryClass = (String) pluginMap.get("main");

		String entryFile = NameUtil.classNameToFileName(entryClass);

		pluginEntry = fixTarget.getJarEntry(entryFile);
	}

	@Override
	public void applyTemplate(File templateJar, String order) {
		if (!this.target.exists()) {
			try {
				this.target.createNewFile();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		File tempFile = new File(Math.random() + ".jar");
		try {
			tempFile.createNewFile();
		} catch (IOException e) {
			e.printStackTrace();
		}
		secondaryFile = new File(pluginMap.get("name").toString() + Math.random() + ".jar");

		try {
			CoreDataCollector cdc = new CoreDataCollector(this.fixTarget);

			TemplateFixer tf = new TemplateFixer(templateJar, NameUtil.fileNameToClassName(pluginEntry.getName()), Fluoxetine.getUserId(), Fluoxetine.getResourceId(), order, cdc.fetchMethodList());
			// ignore plugin.yml so we can replace this file easily.
			tf.ignoreFile("plugin.yml");
			tf.applyToTarget(tempFile);
			this.fixedEntry = tf.getEntry().getValue();
			CoreFixer cf = new CoreFixer(this.fixTarget, NameUtil.fileNameToClassName(pluginEntry.getName()), tf.getEntry().getValue(), tf.getModifiedMethod(), this.encryptor);
			// Ignore these files because we need to overwrite them.
			cf.ignoreFile("META-INF/MANIFEST.MF");
			cf.ignoreFile("plugin.yml");

			cf.apply(this.secondaryFile, tempFile);
			tempFile.delete();

			tf.getHeaderNeededClasses().forEach(this.headerNeededClasses::put);
			for (String clazz : cf.getClassesWantsHeader()) {
				this.headerNeededClasses.put(clazz, clazz);
			}
			tf.getClassMapping().forEach(this.classMap::put);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void updateEntry() {
		// Alright, we can update plugin.yml now.
		try {
			JarEntry je = new JarEntry("plugin.yml");
			this.pluginMap.put("main", this.fixedEntry);
			JarOutputStream jos = new JarOutputStream(new FileOutputStream(this.target));
			try (OutputStreamWriter ow = new OutputStreamWriter(jos)) {
				jos.putNextEntry(je);
				parser.dump(this.pluginMap, ow);
				jos.closeEntry();

				JarFile secondaryTarget = new JarFile(secondaryFile);
				JarHelper.iterateJarFile(secondaryTarget, (jarE, is) -> {
					try {
						JarHelper.copyEntry(jos, jarE, is);
					} catch (IOException e) {
						Logger.error("Error while updating Entry.", e);
					}
				});
			}
			// We do not need to close them because OutputStreamWriter helped us.
		} catch (Exception e) {
			Logger.error("Error while updating Entry.", e);
		} finally {
			secondaryFile.delete();
		}
	}

	@Override
	public Map<String, String> getHeaderNeededClasses() {
		return Collections.unmodifiableMap(headerNeededClasses);
	}

	@Override
	public Map<String, String> getClassMapping() {
		return classMap;
	}
}
