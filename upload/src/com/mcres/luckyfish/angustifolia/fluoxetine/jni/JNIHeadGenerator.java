package com.mcres.luckyfish.angustifolia.fluoxetine.jni;

import com.mcres.luckyfish.angustifolia.fluoxetine.jni.java.DescriptorMapper;
import com.mcres.luckyfish.angustifolia.fluoxetine.jni.java.JavaClassReader;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.JarHelper;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.info.NativeMethodDescriptor;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.Type;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.jar.JarFile;

class JNIHeadGenerator {
	private final List<File> generatedHeader = new ArrayList<>();
	private final File file;
	private final File targetDir;

	public JNIHeadGenerator(File file, File target) {
		this.file = file;
		this.targetDir = new File(target, "src");
	}

	void generateHeaders() {
		// it seemed that we have to write the header generator by ourselves.
		try {
			scan();

			List<NativeMethodDescriptor> nativeMethodList = JavaClassReader.getNativeMethods();

			generateHeaderForMethods(nativeMethodList);
		} catch (IOException e) {
			Logger.error("Cannot read jar", e);
		}
	}

	private void scan() throws IOException {
		JarHelper.iterateJarFile(new JarFile(file), (owo, qwq) -> {
			if (!owo.getName().endsWith(".class")) {
				return;
			}

			ClassReader cr = new ClassReader(qwq);
			cr.accept(new JavaClassReader(), ClassReader.EXPAND_FRAMES);
		});
	}

	private void generateHeaderForMethods(List<NativeMethodDescriptor> nativeMethodList) {
		Map<String, File> fileMap = new HashMap<>();

		for (NativeMethodDescriptor nativeMethod : nativeMethodList) {
			try {
				if (!fileMap.containsKey(nativeMethod.getGeneratedClassName())) {
					File f = new File(targetDir, nativeMethod.getGeneratedClassName() + ".h");
					if (!f.exists()) {
						f.createNewFile();
					}

					fileMap.put(nativeMethod.getGeneratedClassName(), f);

					makeJNIHeaderStart(f);
				}

				File f = fileMap.get(nativeMethod.getGeneratedClassName());
				if (!f.exists()) {
					f.createNewFile();
				}

				makeJNIFunction(f, nativeMethod);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		fileMap.forEach((clazz, file) -> {
			generatedHeader.add(file);
			try {
				makeJNIHeaderEnd(file);
			} catch (Exception ignored) {

			}
		});
	}

	private void makeJNIFunction(File f, NativeMethodDescriptor nativeMethod) throws Exception {
		Type methodDescriptorType = Type.getMethodType(nativeMethod.getDescriptor());

		try (PrintWriter pw = new PrintWriter(new FileWriter(f, true))) {
			pw.print("JNIEXPORT ");
			pw.print(DescriptorMapper.getTypeMap(methodDescriptorType.getReturnType()));
			pw.print(" ");
			pw.print("JNICALL");
			pw.print(" ");

			pw.print("Java_" + nativeMethod.getGeneratedClassName() + "_" + nativeMethod.getGeneratedMethodName() + "(JNIEnv *, jclass, ");

			Type[] arguments = methodDescriptorType.getArgumentTypes();

			for (int i = 0; i < arguments.length; i++) {
				Type type = arguments[i];
				pw.print(DescriptorMapper.getTypeMap(type));

				if (i < arguments.length - 1) {
					pw.print(", ");
				} else {
					pw.println(");");
				}

				pw.println();
			}
		}
	}

	private void makeJNIHeaderStart(File f) throws Exception {
		try (PrintWriter pw = new PrintWriter(new FileWriter(f))) {
			String headerDefinition = f.getName().replaceAll("\\.", "_");
			pw.println("#ifndef INCLUDED_" + headerDefinition);
			pw.println("#define INCLUDED_" + headerDefinition);

			pw.println("#include <jni.h>");

			pw.println();

			pw.println("#ifdef __cpluscplus");
			pw.println("extern \"C\" {");
			pw.println("#endif //__cplusplus");

			pw.println();
			pw.println();

			// We'll put #endif soon
		}
	}

	private void makeJNIHeaderEnd(File f) throws Exception {
		try (PrintWriter pw = new PrintWriter(new FileWriter(f, true))) {
			pw.println();
			pw.println();

			pw.println("#ifdef __cpluscplus");
			pw.println("}");
			pw.println("#endif //__cplusplus");

			pw.println("");
			pw.println("#endif // INCLUDED_" + f.getName().replaceAll("\\.", "_"));
		}
	}

	List<String> getIncludes() {
		List<String> result = new ArrayList<>();
		for (File f : generatedHeader) {
			result.add(f.getName());
		}

		return result;
	}
}
