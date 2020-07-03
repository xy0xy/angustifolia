package com.mcres.luckyfish.angustifolia.fluoxetine.storage;

import com.mcres.luckyfish.angustifolia.fluoxetine.Fluoxetine;
import com.mcres.luckyfish.angustifolia.fluoxetine.util.Logger;
import com.mysql.cj.jdbc.Driver;

import java.sql.Connection;
import java.sql.DriverManager;

public class MySQL implements Database {
	private final String host;
	private final int port;

	private final String database;
	private final String table;

	private final String user;
	private final String password;

	public MySQL() {
		if (Fluoxetine.isDebugging()) {
			table = "license_user_purchased";
			port = 3306;
			database = "mcres";
			user = "root";
			password = "fish";
			host = "127.0.0.1";
		} else {
			host = "127.0.0.1";
			port = 3306;
			database = "mcres";
			table = "license_user_purchased";
			user = "mcres";
			password = "67mbnaprBbch5fZC";
		}
	}

	private Connection connect() throws Exception {
		Class<?> mysqlClazz = Driver.class;
		return DriverManager.getConnection("jdbc:mysql://" + host + ":" + port + "/" + database + "?allowPublicKeyRetrieval=true", user, password);
	}

	@Override
	public void store(int userId, int resourceId, String key, String order) {
		try (Connection c = connect()) {
			c.prepareStatement("INSERT INTO " + table + " values ('" +
					userId + "'," +
					"null,'" +
					order + "','" +
					key + "'," +
					resourceId + ");").executeUpdate();
		} catch (Exception e) {
			Logger.error("Cannot save data");
			e.printStackTrace();
		}
	}
}
