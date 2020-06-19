package com.mcres.luckyfish.angustifolia.fluoxetine.entry;

import java.io.File;
import java.io.IOException;
import java.util.Map;

public interface EntryFixer {
	void findEntry() throws IOException;
	void applyTemplate(File templateJar, String order);
	void updateEntry();
	Map<String, String> getHeaderNeededClasses();
	Map<String, String> getClassMapping();
}
