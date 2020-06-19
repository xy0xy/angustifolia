#ifndef MCRES_LICENSE_SERVER_MYSQL_H
#define MCRES_LICENSE_SERVER_MYSQL_H

#include <stdlib.h>
#include <stdbool.h>
#define TEST_MYSQL

#ifdef TEST_MYSQL
#define TABLE_NAME "license_user_purchased"
#define DATABASE_NAME "mcres"
#define USER_NAME "root"
#define PASSWORD "fish"
#else
#define TABLE_NAME "license_user_purchased"
#define DATABASE_NAME "mcres"
#define USER_NAME "mcres"
#define PASSWORD "67mbnaprBbch5fZC"
#endif // TEST_MYSQL

#define QUERY_COMMAND "select * from %s where userId=? and orderTarget=?;"
#define UPDATE_COMMAND "update %s set hardwareId=? where userId=? and resourceId=? and orderTarget=?;"

typedef struct license_data
{
	unsigned int uid;
	
	size_t orderLen;
	char * order;
	
	size_t motherboardIdLen;
	char * motherboardId;
	
	size_t decryptPasswordLen;
	char * decryptPassword;
	
	unsigned int resource_id;
	
	bool motherboardUpdateRequired;
} LicenseData;

typedef struct wrapped_mysql_session WrappedMySQLSession;

int initMySQL();

WrappedMySQLSession * connectMySQL(unsigned int port);
LicenseData * getLicenseDataInMySQL(WrappedMySQLSession *, size_t *amount, unsigned int user, char * order);

void updateMotherboard(WrappedMySQLSession * session, char * order, size_t orderLen, int userId, int resourceId, char * motherboard, size_t motherboardIdLen);

void disconnectFromMySQL(WrappedMySQLSession * session);

void endMySQL();

#endif //MCRES_LICENSE_SERVER_MYSQL_H
