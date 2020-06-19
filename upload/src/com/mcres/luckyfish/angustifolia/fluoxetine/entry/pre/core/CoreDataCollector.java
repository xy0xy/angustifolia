package com.mcres.luckyfish.angustifolia.fluoxetine.entry.pre.core;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.JarHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import org.objectweb.asm.ClassReader;

import java.util.jar.JarFile;

public class CoreDataCollector {
	private final JarFile target;

	public CoreDataCollector(JarFile target) {
		this.target = target;
	}

	public CoreMethodList fetchMethodList() {
		Logger.info("Collecting code.");

		JarHelper.iterateJarFile(target, (entry, is) -> {
			if (!entry.getName().endsWith(".class")) {
				return;
			}
			try {
				ClassReader cr = new ClassReader(is);
				cr.accept(new CoreClassSeeker(), ClassReader.EXPAND_FRAMES);
			} catch (Exception e) {
				Logger.error("Unable to collect method list.", e);
			}
		});

		return CoreClassSeeker.getCoreMethodList();
	}
}
