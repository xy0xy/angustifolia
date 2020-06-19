package com.mcres.luckyfish.angustifolia.licenseclient.program;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import java.util.Base64;
import java.util.Locale;

public class Motd {
    private Motd() {
        throw new IllegalStateException();
    }

    public static void displayMotd(String motd) {
        // TODO: decode base64 and run condition.

		try {
			String[] strings = motd.split("#");
			if (checkCondition(strings[0])) {
				System.out.println(new String(Base64.getDecoder().decode(strings[1])));
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
    }

    private static boolean checkCondition(String conditionCode) throws ScriptException {
	    ScriptEngineManager sem = new ScriptEngineManager();
	    ScriptEngine se = sem.getEngineByName("javascript");

	    se.put("env", new Environment());
	    return (boolean) se.eval(conditionCode);
    }

    private static class Environment {
    	public String country;
    	public String language;
    	public String operationSystem;
    	public String javaVersion;

    	Environment() {
		    this.country = Locale.getDefault().getCountry().toLowerCase();
		    this.language = Locale.getDefault().getLanguage();
		    this.operationSystem = System.getProperty("os.name");
		    this.javaVersion = System.getProperty("java.version");
	    }
    }
}