package com.mcres.luckyfish.angustifolia.fluoxetine.util;

import com.mcres.luckyfish.angustifolia.fluoxetine.Fluoxetine;

import java.io.PrintStream;
import java.text.DateFormat;
import java.util.Date;

public class Logger {
	public static void error(String message) {
		error(message, null);
	}

	public static void error(String message, Throwable t) {
		print("ERROR: " + message, System.err);
		if (t != null) {
			t.printStackTrace(System.err);
		}
	}

	public static void debug(String message) {
		if (Fluoxetine.isDebugging()) {
			print("DEBUG: " + message, System.out);
		}
	}

	public static void info(String message) {
		print("INFO: " + message, System.out);
	}

	private static void print(String message, PrintStream target) {
		String date = DateFormat.getInstance().format(new Date());
		target.println(date + " " + message);
	}
}
