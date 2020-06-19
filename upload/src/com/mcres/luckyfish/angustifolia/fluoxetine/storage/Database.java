package com.mcres.luckyfish.angustifolia.fluoxetine.storage;

public interface Database {
	void store(int userId, int resourceId, String key, String order);
}
