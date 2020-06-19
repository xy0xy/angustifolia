package com.mcres.luckyfish.angustifolia.fluoxetine.jni;

import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.NativeMethodDescriptor;

import java.io.*;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

class CMakeFileUpdater {
	private final File root;

	CMakeFileUpdater(File root) {
		this.root = root;
	}

	void updateCMakeFile(Map<String, String> classes) throws IOException {
		File cmakeFile = new File(root, "CMakeLists.txt");

		// Read them first.
		List<String> codeList = new LinkedList<>();

		readCMakeFile(cmakeFile, codeList, classes);

		// Ok we can write them to file now.
		writeCMakeFile(cmakeFile, codeList);
	}

	private void readCMakeFile(File cmakeFile, List<String> codeList, Map<String, String> classes) throws IOException {
		try (BufferedReader br = new BufferedReader(new FileReader(cmakeFile))) {
			AtomicReference<String> s = new AtomicReference<>();
			while (true) {
				s.set(br.readLine());
				if (s.get() == null) {
					break;
				}
				classes.forEach((cooked, raw) -> s.set(s.get().replaceAll(NativeMethodDescriptor.normalizeName(raw) + "\\.h", NativeMethodDescriptor.normalizeName(cooked) + ".h")));

				codeList.add(s.get());
			}

		}

	}

	private void writeCMakeFile(File cmakeFile, List<String> codeList) throws IOException {
		try (PrintWriter pw = new PrintWriter(new FileWriter(cmakeFile))) {
			for (String line : codeList) {
				pw.println(line);
			}
		}
	}
}
