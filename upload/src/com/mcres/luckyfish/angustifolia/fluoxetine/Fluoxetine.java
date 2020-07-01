package com.mcres.luckyfish.angustifolia.fluoxetine;

import com.mcres.luckyfish.angustifolia.fluoxetine.crypto.Blowfish;
import com.mcres.luckyfish.angustifolia.fluoxetine.crypto.Encryptor;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.EntryFixer;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.factory.EntryFixerFactory;
import com.mcres.luckyfish.angustifolia.fluoxetine.jni.JNICodeGenerator;
import com.mcres.luckyfish.angustifolia.fluoxetine.storage.Database;
import com.mcres.luckyfish.angustifolia.fluoxetine.storage.MySQL;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.JarHelper;

import java.io.File;
import java.util.ArrayList;
import java.util.Base64;
import java.util.List;
import java.util.Map;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;

public class Fluoxetine {
	private static final boolean debugging = true;

	private static int userId;
	private static int resourceId;
	
	public static void main(String[] args) {
		for (String s : args) {
			if (s.equalsIgnoreCase("help")) {
				printHelpMessage();
				return;
			}
		}
		String targetJar;
		String templateJar;
		String order;
		try {
			userId = Integer.parseInt(args[1]);
			resourceId = Integer.parseInt(args[2]);
			targetJar = args[0];
			templateJar = args[3];
			order = args[4];
		} catch (ArrayIndexOutOfBoundsException e) {
			printHelpMessage();
			return;
		}

		try {
			Database database = new MySQL();
			Encryptor encryptor = new Blowfish();
			EntryFixer ef = new EntryFixerFactory(new JarFile(targetJar), new File(tempFileName(targetJar))).setEncryptor(encryptor).build().iterator().next();
			ef.findEntry();
			ef.applyTemplate(new File(templateJar), order);
			ef.updateEntry();

			// And we need to obfuscate them again.


			// Ok we have built the jar, now generate headers.
			File tempFile = new File(tempFileName(targetJar));
			Map<String, String> classes = ef.getHeaderNeededClasses();
			JNICodeGenerator jcg = new JNICodeGenerator(tempFile, classes, ef.getClassMapping());

			List<String> rawClass = new ArrayList<>();
			classes.forEach((cooked, raw) -> rawClass.add(raw));
			jcg.updateCode();

			File finalFile = new File(finalFileName(targetJar));
			if (!finalFile.exists()) {
				finalFile.createNewFile();
			}

			List<File> nativeFiles = jcg.compileCode();
			JarOutputStream jos = JarHelper.copyJarContent(new JarFile(tempFile), finalFile);
			for (File nativeFile : nativeFiles) {
				JarHelper.writeEntry(jos, nativeFile);
			}

			jos.close();

			database.store(userId, resourceId, new String(Base64.getEncoder().encode(encryptor.getKey())), order);
		} catch (Exception e) {
			e.printStackTrace();

			System.exit(-1);
		}
	}

	public static int getUserId() {
		return userId;
	}

	public static int getResourceId() {
		return resourceId;
	}

	private static void printHelpMessage() {
		System.out.println("Usage: java -jar <target jar> <user id> <resource id> <template jar> <order id>");
	}

	public static boolean isDebugging() {
		return debugging;
	}

	private static String modifyFileName(String name, String addition) {
		String[] nameSlices = name.split("\\.");
		String[] fileNameSlices = new String[nameSlices.length - 1];
		System.arraycopy(nameSlices, 0, fileNameSlices, 0, nameSlices.length - 1);
		String fileName = String.join(".", fileNameSlices) + addition;
		// add extension.
		return fileName + "." + nameSlices[nameSlices.length - 1];
	}

	public static String tempFileName(String name) {
		return modifyFileName(name, "-temp");
	}

	public static String finalFileName(String name) {
		return modifyFileName(name, "-final");
	}
}
