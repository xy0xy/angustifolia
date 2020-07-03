package com.mcres.luckyfish.angustifolia.licenseclient.plugin.classloader;

import com.mcres.luckyfish.angustifolia.licenseclient.plugin.PluginLicenseClient;
import com.google.common.io.ByteStreams;
import org.bukkit.plugin.PluginLoader;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.security.CodeSigner;
import java.security.CodeSource;
import java.util.HashMap;
import java.util.Map;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;

public class EncryptedClassLoader extends URLClassLoader {
	private final JarFile jar;
	private final Map<String, Class<?>> classes = new HashMap<>();
	private final Manifest manifest;
	private final URL url;

	public EncryptedClassLoader(File file) throws IOException {
		super(new URL[]{file.toURI().toURL()});
		jar = new JarFile(file);
		manifest = jar.getManifest();
		url = file.toURI().toURL();
	}



	protected Class<?> findClass(String name) throws ClassNotFoundException {
		return findClass(name, false);
	}

	Class<?> findClass(String name, boolean checkGlobal) throws ClassNotFoundException {
		Class<?> result = classes.get(name);

		if (result == null) {
			if (checkGlobal) {
				try {
					result = Class.forName(name);
				} catch (ClassNotFoundException e) {
					// not found.. we need to get it.
				}
			}

			if (result == null) {
				// check the name so that nobody can hack into it.
				if (!name.startsWith("com.mcres.luckyfish.angustifolia.")) {
					String path = name.replace('.', '/').concat(".class");
					JarEntry entry = jar.getJarEntry(path);

					if (entry != null) {
						byte[] classBytes;

						try (InputStream is = jar.getInputStream(entry)) {
							classBytes = ByteStreams.toByteArray(is);
						} catch (IOException ex) {
							throw new ClassNotFoundException(name, ex);
						}

						classBytes = analyzeClass(classBytes);

						int dot = name.lastIndexOf('.');
						if (dot != -1) {
							String pkgName = name.substring(0, dot);
							if (getPackage(pkgName) == null) {
								try {
									if (manifest != null) {
										definePackage(pkgName, manifest, url);
									} else {
										definePackage(pkgName, null, null, null, null, null, null, null);
									}
								} catch (IllegalArgumentException ex) {
									if (getPackage(pkgName) == null) {
										throw new IllegalStateException("Cannot find package " + pkgName);
									}
								}
							}
						}

						CodeSigner[] signers = entry.getCodeSigners();
						CodeSource source = new CodeSource(url, signers);

						result = defineClass(name, classBytes, 0, classBytes.length, source);
					}
				}

				try {
					if (result == null && !checkGlobal) {
						result = findClass(name, true);
					}

					if (result == null) {
						result = super.findClass(name);
					}
				} catch (ClassNotFoundException e) {
					throw new ClassNotFoundException(name);
				}

				if (result != null) {
					try {
						// access via reflection.
						putBukkitClass(name, result);
					} catch (Exception ignored) {

					}
				}
			}

			classes.put(name, result);
		}

		return result;
	}

	private native static byte[] analyzeClass(byte[] data);

	@SuppressWarnings("unchecked")
	private void putBukkitClass(String name, Class<?> target) throws Exception {
		Class<? extends PluginLoader> loaderClass = PluginLicenseClient.getInstance().getPluginLoader().getClass();
		Method setClassMethod = loaderClass.getDeclaredMethod("setClass", String.class, Class.class);
		setClassMethod.invoke(PluginLicenseClient.getInstance().getPluginLoader(), name, target);

		ClassLoader cl = PluginLicenseClient.getInstance().getClass().getClassLoader();
		Class<? extends ClassLoader> clClass = cl.getClass();
		Field classesField = clClass.getField("classes");
		Map<String, Class<?>> classes = (Map<String, Class<?>>) classesField.get(cl);
		classes.put(name, target);
	}
}
