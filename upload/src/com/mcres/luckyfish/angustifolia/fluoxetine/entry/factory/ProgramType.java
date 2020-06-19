package com.mcres.luckyfish.angustifolia.fluoxetine.entry.factory;

import com.mcres.luckyfish.angustifolia.fluoxetine.entry.EntryFixer;
import com.mcres.luckyfish.angustifolia.fluoxetine.entry.PluginEntryFixer;

public enum ProgramType {
	BUKKIT_PLUGIN(PluginEntryFixer.class);
	private final Class<? extends EntryFixer> entryFixerClass;

	ProgramType(Class<? extends EntryFixer> entryFixerClass) {
		this.entryFixerClass = entryFixerClass;
	}

	public Class<? extends EntryFixer> getEntryFixerClass() {
		return entryFixerClass;
	}
}
