package com.mcres.luckyfish.angustifolia.fluoxetine.jni;

import com.mcres.luckyfish.angustifolia.fluoxetine.Fluoxetine;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import org.apache.commons.io.IOUtils;

import java.io.*;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

public class JNICodeGenerator {
	private final JNIHeadGenerator headGenerator;
	private final Map<String, String> nativeNeedClasses;
	private final Map<String, String> classMapping;

	private static final File linuxDirtectory = new File("linux-client-source");
	private static final File windowsDirectory = new File("windows-client-source");
	private static final File macDirectory = new File("mac-client-source");

	public JNICodeGenerator(File file, Map<String, String> nativeNeedClasses, Map<String, String> classMapping) {
		this.nativeNeedClasses = nativeNeedClasses;

		headGenerator = new JNIHeadGenerator(file, linuxDirtectory);
		// We'll add other directories soon.
		this.classMapping = classMapping;
	}

	public void updateCode() {
		headGenerator.generateHeaders();

		File linuxEntryCode = new File(new File(linuxDirtectory, "src"), "entry.c");
		// We'll do this soon
		// File windowsCode = new File(new File(windowsDirectory, "src"), "entry.c");
		// File macCode = new File(new File(macDirectory, "src"), "entry.c");

		File linuxJniCode = new File(new File(new File(linuxDirtectory, "src"), "util"), "java.c");

		try {
			modifyEntryCode(linuxEntryCode);
			// modifyCode(windowsCode);
			// modifyCode(macCode);

			modifyJniCode(linuxJniCode);

			CMakeFileUpdater linuxMakeFileUpdater = new CMakeFileUpdater(linuxDirtectory);
			// CMakeFileUpdater windowsMakeFileUpdater = new CMakeFileUpdater(windowsCode);
			// CMakeFileUpdater macMakeFileUpdater = new CMakeFileUpdater(macCode);

			linuxMakeFileUpdater.updateCMakeFile(nativeNeedClasses);
			// windowsMakeFileUpdater.updateCMakeFile(classes);
			// macMakeFileUpdater.updateCMakeFile(classes);
		} catch (IOException e) {
			Logger.error("Failed to change code", e);
		}
	}

	private void modifyEntryCode(File source) throws IOException {
		StringBuilder code = new StringBuilder();
		AtomicReference<String> line = new AtomicReference<>();

		List<String> headers = headGenerator.getIncludes();
		for (String header : headers) {
			code.append("#include \"").append(header).append("\"\n");
		}

		try (BufferedReader br = new BufferedReader(new FileReader(source))) {
			while (true) {
				line.set(br.readLine());
				if (line.get() == null) {
					break;
				}
				if (line.get().startsWith("#include \"com_mcres_luckyfish")) {
					continue; // ignore the original header.
				}
				if (line.get().contains("JNICALL")) {
					nativeNeedClasses.forEach((modified, raw) -> {
						String targetName = null;

						for (String header : headers) {
							if (header.startsWith(raw.replaceAll("\\.", "_"))) {
								targetName = header;
							}
						}

						if (targetName == null) {
							Logger.info("Uh I didn't found any method matches code: ");
							Logger.info(line.get());
							Logger.info("So I'd like to skip this line of code");
							return;
						}
						targetName = targetName.replaceAll("\\.h", "");

						String codeLine = line.get().replaceAll(raw.replaceAll("\\.", "_"), targetName);
						line.set(codeLine);
					});
				}

				code.append(line.get()).append("\n");
			}
		}
		try (BufferedWriter bw = new BufferedWriter(new FileWriter(source))) {
			bw.write(code.toString());
			bw.flush();
		}
	}

	private void modifyJniCode(File source) throws IOException {
		StringBuilder code = new StringBuilder();
		AtomicReference<String> line = new AtomicReference<>();

		try (BufferedReader br = new BufferedReader(new FileReader(source))) {
			while (true) {
				line.set(br.readLine());
				if (line.get() == null) {
					break;
				}
				classMapping.forEach((raw, modified) -> {
					Logger.debug("Checking code with class: " + raw.replaceAll("\\.", "/"));
					if (line.get().contains(raw.replaceAll("\\.", "/"))) {
						line.set(line.get().replaceAll(raw.replaceAll("\\.", "/"), modified.replaceAll("\\.", "/")));
					}
				});

				code.append(line.get()).append("\n");
			}
		}
		try (BufferedWriter bw = new BufferedWriter(new FileWriter(source))) {
			bw.write(code.toString());
			bw.flush();
		}
	}

	public List<File> compileCode() {
		// We'll call cmake to do this soon.
		List<File> result = new LinkedList<>();

		File linuxNative = makeClient(linuxDirtectory, "make");
		if (linuxNative != null) {
			result.add(linuxNative);
		}
		// TODO: Add windows and mac soon.

		return result;
	}

	private File makeClient(File directory, String makeCommand) {
		try {
			// Make the directory for CMake and make.
			File buildDirectory = new File(directory, "build");
			if (buildDirectory.exists()) {
				buildDirectory.delete();
			}

			buildDirectory.mkdirs();

			List<String> commandArguments = new ArrayList<>();
			commandArguments.add("cmake");
			if (Fluoxetine.isDebugging()) {
				commandArguments.add("-DCMAKE_BUILD_TYPE=Debug");
			} else {
				commandArguments.add("-DCMAKE_BUILD_TYPE=Release");
			}
			commandArguments.add("..");

			Process cmakeProcess = new ProcessBuilder(commandArguments).directory(buildDirectory).start();

			// Wait for cmake.
			while (cmakeProcess.isAlive()) {
				IOUtils.copy(cmakeProcess.getInputStream(), System.out);
				IOUtils.copy(cmakeProcess.getErrorStream(), System.err);
			}

			Process makeProcess = new ProcessBuilder(makeCommand).directory(buildDirectory).start();

			// Wait for make
			while (makeProcess.isAlive()) {
				IOUtils.copy(makeProcess.getInputStream(), System.out);
				IOUtils.copy(makeProcess.getErrorStream(), System.err);
			}

			// Ok we are going to get the file,
			File f = new File(buildDirectory, System.mapLibraryName("mcres_license_client"));
			if (!f.exists()) {
				throw new IOException(f.getName() + " not exist.");
			}

			File result = new File(buildDirectory, System.mapLibraryName("myosotis"));
			f.renameTo(result);

			return result;
		} catch (Exception e) {
			Logger.error("Error while trying to compile native", e);

			return null;
		}
	}
}
