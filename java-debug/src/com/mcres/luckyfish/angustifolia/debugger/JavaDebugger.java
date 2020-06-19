package com.mcres.luckyfish.angustifolia.debugger;

import com.mcres.luckyfish.angustifolia.debugger.generated.GeneratedClassVisitor;
import org.apache.commons.io.IOUtils;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.util.ASMifier;
import org.objectweb.asm.util.TraceClassVisitor;

import java.io.*;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class JavaDebugger {
	private static final boolean generateJavaSource = false;
	public static void main(String[] args) {
		if (generateJavaSource) {
			ASMifier asmifier = new ASMifier();

			byte[] classData;
			if (args.length == 1) {
				try {
					classData = readClass(args[0]);
				} catch (IOException e) {
					e.printStackTrace();
					return;
				}
			} else if (args.length == 2) {
				try {
					classData = readJarClass(args[0], args[1]);
				} catch (IOException e) {
					e.printStackTrace();
					return;
				}
			} else {
				System.out.println("Usage: [jar file] <class name>");
				return;
			}

			File f = new File("../java-debug/src/cn/mcres/luckyfish/angustifolia/debugger/generated/GeneratedClassVisitor.java");
			System.out.println(f.getAbsolutePath());
			PrintWriter pw = null;
			try {
				pw = new PrintWriter(f);
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}

			ClassReader cr = new ClassReader(classData);
			TraceClassVisitor tcv = new TraceClassVisitor(null, asmifier, pw);
			cr.accept(tcv , ClassReader.EXPAND_FRAMES);
		} else {
			String className;
			if (args.length == 1) {
				className = args[0];
			} else if (args.length == 2) {
				className = args[1];
			} else {
				System.out.println("Usage: [jar file] <class name>");
				return;
			}

			String[] nameSlices = className.split("\\.");
			className = nameSlices[nameSlices.length - 1];

			try (FileOutputStream fos = new FileOutputStream(className + ".class")) {
				fos.write(GeneratedClassVisitor.dump() != null ? GeneratedClassVisitor.dump() : new byte[0]);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	private static byte[] readJarClass(String jar, String clazz) throws IOException {
		JarFile jarFile = new JarFile(jar, true);
		System.out.println("Reading class of: " + clazz.replaceAll("\\.", File.separator) + ".class");
		JarEntry jarEntry = jarFile.getJarEntry(clazz.replaceAll("\\.", File.separator) + ".class");

		try (ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
			try (InputStream is = jarFile.getInputStream(jarEntry)) {
				IOUtils.copy(is, baos);
			}

			return baos.toByteArray();
		}
	}

	private static byte[] readClass(String clazz) throws IOException {
		try (ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
			try (FileInputStream fis = new FileInputStream(new File(clazz.replaceAll("\\.", File.pathSeparator + ".class")))) {
				IOUtils.copy(fis, baos);
			}

			return baos.toByteArray();
		}
	}
}
