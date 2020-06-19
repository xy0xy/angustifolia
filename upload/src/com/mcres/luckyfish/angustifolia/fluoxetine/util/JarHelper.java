package com.mcres.luckyfish.angustifolia.fluoxetine.util;

import org.apache.commons.io.IOUtils;

import java.io.*;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;

public class JarHelper {
	public static void iterateJarFile(JarFile target, SilentBiConsumer<JarEntry, InputStream> function) {
		try {
			Enumeration<JarEntry> jarEntryEnumeration = target.entries();
			while (jarEntryEnumeration.hasMoreElements()) {
				JarEntry je = jarEntryEnumeration.nextElement();
				if (je.isDirectory()) {
					continue;
				}
				try (InputStream is = target.getInputStream(je)) {
					function.acceptSilently(je, is);
				}
			}
		} catch (Exception e) {
			Logger.error("Cannot read jar: " + target.getName(), e);
		}
	}

	public static void writeEntry(JarOutputStream jos, InputStream fromStream, JarEntry to) throws IOException {
		jos.putNextEntry(to);
		IOUtils.copy(fromStream, jos);
		jos.closeEntry();
	}

	public static void copyEntry(JarOutputStream jos, JarEntry from, InputStream fromStream) throws IOException {
		JarEntry je = new JarEntry(from.getName());
		je.setComment(from.getComment());
		je.setExtra(from.getExtra());
		if (from.getMethod() != -1) {
			je.setMethod(from.getMethod());
		}
		if (from.getCrc() != -1) {
			je.setCrc(from.getCrc());
		}

		writeEntry(jos, fromStream, je);
	}

	public static void writeEntry(JarOutputStream jos, byte[] bytes, JarEntry to) throws IOException {
		writeEntry(jos, new ByteArrayInputStream(bytes), to);
	}

	public static void writeEntry(JarOutputStream jos, File f) throws IOException {
		writeEntry(jos, new FileInputStream(f), new JarEntry(f.getName()));
	}

	public static JarOutputStream copyJarContent(JarFile from, File to) throws IOException {
		JarOutputStream jos = new JarOutputStream(new FileOutputStream(to));

		iterateJarFile(from, (je, is) -> {
			jos.putNextEntry(new JarEntry(je.getName()));
			IOUtils.copy(is, jos);
			jos.closeEntry();
		});

		return jos;
	}
}
