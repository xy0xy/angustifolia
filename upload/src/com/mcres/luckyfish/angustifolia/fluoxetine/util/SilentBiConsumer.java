package com.mcres.luckyfish.angustifolia.fluoxetine.util;

public interface SilentBiConsumer<T, U> {
	void accept(T owo, U qwq) throws Exception;

	default void acceptSilently(T owo, U qwq) {
		try {
			accept(owo, qwq);
		} catch (Exception e) {
			// ignore.
		}
	}
}
