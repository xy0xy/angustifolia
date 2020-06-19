package com.mcres.luckyfish.angustifolia.fluoxetine.entry.factory;

import com.mcres.luckyfish.angustifolia.fluoxetine.crypto.Encryptor;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.EntryFixer;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.YamlReader;

import java.io.File;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class EntryFixerFactory {
	private final JarFile jarFile;
	private final File target;
	private Encryptor encryptor = null;
	private final Set<ProgramType> programType = new HashSet<>();

	public EntryFixerFactory(JarFile jar, File target) {
		jarFile = jar;
		this.target = target;

		analyzeProgramType();
	}
	private void analyzeProgramType() {
		JarEntry pluginYml = jarFile.getJarEntry("plugin.yml");
		JarEntry bungeeYml = jarFile.getJarEntry("bungee.yml");
		JarEntry modInfo = jarFile.getJarEntry("mcmod.info");
		JarEntry manifest = jarFile.getJarEntry("META-INF/MANIFEST.MF");

		if (pluginYml != null) {
			try {
				InputStream is = jarFile.getInputStream(pluginYml);
				YamlReader pluginYmlReader = new YamlReader(is);
				if (pluginYmlReader.getString("name") != null && pluginYmlReader.getString("main") != null) {
					this.programType.add(ProgramType.BUKKIT_PLUGIN);
				}
				is.close();
				Logger.info("Target file seemed to be a bukkit plugin.");
			} catch (Throwable ignored) {}
		}
	}

	public EntryFixerFactory setEncryptor(Encryptor encryptor) {
		this.encryptor = encryptor;
		return this;
	}

	public Set<ProgramType> getProgramType() {
		return Collections.unmodifiableSet(this.programType);
	}

	public Set<EntryFixer> build() {
		Set<EntryFixer> fixers = new HashSet<>();
		for (ProgramType programType : this.programType) {
			Class<? extends EntryFixer> ef = programType.getEntryFixerClass();
			try {
				fixers.add(ef.getConstructor(JarFile.class, File.class, Encryptor.class).newInstance(jarFile, target, this.encryptor));
			} catch (InstantiationException | NoSuchMethodException | InvocationTargetException | IllegalAccessException e) {
				e.printStackTrace();
			}
		}

		return fixers;
	}
}
