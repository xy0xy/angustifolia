package com.mcres.luckyfish.angustifolia.fluoxetine.util;

import org.yaml.snakeyaml.Yaml;

import java.io.InputStream;
import java.util.Map;

public class YamlReader {
	private final Yaml yml = new Yaml();
	private final Map<String, Object> content;

	public YamlReader(InputStream is) {
		this.content = yml.load(is);
	}

	public Object get(String value) {
		return content.get(value);
	}
	public String getString(String value) {
		return (String) get(value);
	}
}
