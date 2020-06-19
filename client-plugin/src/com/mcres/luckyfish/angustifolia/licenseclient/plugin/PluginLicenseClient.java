package com.mcres.luckyfish.angustifolia.licenseclient.plugin;

import com.mcres.luckyfish.angustifolia.licenseclient.NewEntry;
import com.mcres.luckyfish.angustifolia.licenseclient.plugin.classloader.EncryptedClassLoader;
import org.bukkit.plugin.java.JavaPlugin;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

@NewEntry
public final class PluginLicenseClient extends JavaPlugin {
	private static PluginLicenseClient instance;

	// They are not final so we can modify them there.
	public static int userId = 0;         // where the user id is, will be modified by plugin modifier on mcres.cn
	public static int resourceId = 0;     // where the resource id is, will be modified by plugin modifier on mcres.cn <-- maybe dangerous, I'd like to change later ._.
	private static final String MAIN_CLASS = ""; // where the real main class is, will be modified by plugin modifier on mcres.cn
	private static final String order = "";

	// keep the class loader there so that we can keep the independence of the plugin classes.
	private EncryptedClassLoader ecl;
	private Class<?> targetClass;
	// TODO: replace all plugin references to this class :]
	private Object targetObject;
	// TODO: add all references of the original main class to this class :p

	@Override
	public void onLoad() {
		// load classes
		instance = this;

		if (!getDataFolder().exists()) {
			getDataFolder().mkdirs();
		}
		saveResource(System.mapLibraryName("myosotis"), true);

		System.load(new File(PluginLicenseClient.getInstance().getDataFolder(), System.mapLibraryName("myosotis")).getAbsolutePath());

		try {
			ecl = new EncryptedClassLoader(this.getFile());
			targetClass = ecl.loadClass(MAIN_CLASS);
			Constructor<?> constructor = targetClass.getConstructor();
			targetObject = constructor.newInstance();

			// invoke onLoad method
			callMethodFromMain(false, "onLoad");
		} catch (NoSuchMethodException e) {
			getLogger().info("Ok this plugin doesn't have onLoad method at all, we won't call it.");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onEnable() {
		// Plugin startup logic
		try {
			callMethodFromMain(false, "onEnable");
		} catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	@Override
	public void onDisable() {
		// Plugin shutdown logic
		try {
			callMethodFromMain(false, "onDisable");
		} catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	public static PluginLicenseClient getInstance() {
		return instance;
	}

	// TODO: Custom methods from plugin's main class will be placed here and accesses the main class via direct invocation

	// TODO: Move this method into C function.
	private static Object callMethodFromMain(boolean sbatic, String name, Object ... args) {
		try {
			Class<?>[] types = new Class[args.length];
			for (int i = 0; i < args.length; i++) {
				Object argument = args[i];
				types[i] = argument.getClass();
			}

			Method m = getInstance().targetClass.getMethod(name, types);
			if (sbatic) {
				return m.invoke(null, args);
			} else {
				return m.invoke(getInstance().targetObject, args);
			}
		} catch (NoSuchMethodException e) {
			getInstance().getLogger().info("Ok this plugin doesn't have " + name + " method at all, we won't call it.");
			return null;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
}
