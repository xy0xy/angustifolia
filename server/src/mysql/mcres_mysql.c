#include "mcres_mysql.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

struct wrapped_mysql_session
{
	MYSQL * sock;
};

typedef struct list_node
{
	LicenseData * licenseData;
	struct list_node * next;
} ListNode;

int initMySQL()
{
	return mysql_library_init(0, NULL, NULL);
}

WrappedMySQLSession * connectMySQL(unsigned int port)
{
	WrappedMySQLSession * session = malloc(sizeof(WrappedMySQLSession));
	MYSQL * mysql = mysql_init(NULL);

	bool reconnect = true;
	unsigned int timeout = 300;
	unsigned int retries = 10;

	mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);
	mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
	mysql_options(mysql, MYSQL_OPT_RETRY_COUNT, &retries);

	mysql_real_connect(mysql, "127.0.0.1", USER_NAME, PASSWORD, DATABASE_NAME, port, NULL, CLIENT_MULTI_RESULTS);
	
	session->sock = mysql;
	return session;
}

LicenseData * getLicenseDataInMySQL(WrappedMySQLSession * session, size_t *amount, unsigned int user, char * order)
{
	LicenseData * result = NULL;
	size_t length = strlen(QUERY_COMMAND) + strlen(TABLE_NAME) + 1;
	char * query = malloc(sizeof(char) * length);
	MYSQL_STMT * preparedStatement = NULL;
	if (!query)
		goto final;
	memset(query, 0, length * sizeof(char));
	
	sprintf(query, QUERY_COMMAND, TABLE_NAME);
	
//	if (mysql_real_query(session->sock, query, length))
//		goto final;
	preparedStatement = mysql_stmt_init(session->sock);

	mysql_stmt_prepare(preparedStatement, query, strlen(query));

	MYSQL_BIND boundArguments[2];
	memset(boundArguments, 0, sizeof(MYSQL_BIND) * 2);
	
	boundArguments[0].buffer_type = MYSQL_TYPE_LONG;
	boundArguments[0].buffer = &user;
	boundArguments[0].buffer_length = sizeof(unsigned int);
	boundArguments[1].buffer_type = MYSQL_TYPE_VARCHAR;
	boundArguments[1].buffer = order;
	boundArguments[1].buffer_length = strlen(order);

	mysql_stmt_bind_param(preparedStatement, boundArguments);

	mysql_stmt_execute(preparedStatement);

	MYSQL_BIND boundResults[5] = { 0 };

	size_t resultLen[5] = { 0 };

	int userId = 0;
	char motherboardId[255] = { 0 };
	char orderTarget[255] = { 0 };
	char decryptPassword[255] = { 0 };
	int resourceId = 0;

	boundResults[0].buffer_type = MYSQL_TYPE_LONG;
	boundResults[0].buffer = &userId;
	boundResults[0].buffer_length = sizeof(int);
	boundResults[0].length = &resultLen[0];

	boundResults[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	boundResults[1].buffer = &motherboardId;
	boundResults[1].buffer_length = sizeof(char) * 255;
	boundResults[1].length = &resultLen[1];

	boundResults[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	boundResults[2].buffer = &orderTarget;
	boundResults[2].buffer_length = sizeof(char) * 255;
	boundResults[2].length = &resultLen[2];

	boundResults[3].buffer_type = MYSQL_TYPE_VAR_STRING;
	boundResults[3].buffer = decryptPassword;
	boundResults[3].buffer_length = sizeof(char) * 255;
	boundResults[3].length = &resultLen[3];

	boundResults[4].buffer_type = MYSQL_TYPE_LONG;
	boundResults[4].buffer = &resourceId;
	boundResults[4].buffer_length = sizeof(int);
	boundResults[4].length = &resultLen[4];

	mysql_stmt_bind_result(preparedStatement, boundResults);
	mysql_stmt_store_result(preparedStatement);
	
	ListNode * licenseList = malloc(sizeof(ListNode));
	ListNode * current = licenseList;
	int index = 0;
	while (1)
	{
		LicenseData * licenseData = malloc(sizeof(LicenseData));
		int err = mysql_stmt_fetch(preparedStatement);
		
		if (err == 1 || err == MYSQL_NO_DATA)
		{
			free(licenseData);
			current->next = NULL;
			current->licenseData = NULL;
			break;
		}
		
		unsigned int * fetchUserId = malloc(resultLen[0]);
		boundResults[0].buffer = fetchUserId;
		mysql_stmt_fetch_column(preparedStatement, &boundResults[0], 0, index);
		
		if (*fetchUserId != user)
		{
			free(fetchUserId);
			free(licenseData);
			continue;
		}
		licenseData->uid = *fetchUserId;
		free(fetchUserId);
		
		unsigned int * fetchResourceId = malloc(resultLen[4]);
		boundResults[4].buffer = fetchResourceId;
		mysql_stmt_fetch_column(preparedStatement, &boundResults[4], 4, index);
		licenseData->resource_id = *fetchResourceId;
		free(fetchResourceId);
		
		char * fetchOrder = malloc(resultLen[2]);
		memset(fetchOrder, 0, resultLen[2]);
		boundResults[2].buffer = fetchOrder;
		mysql_stmt_fetch_column(preparedStatement, &boundResults[2], 2, index);
		
		licenseData->order = fetchOrder;
		licenseData->orderLen = resultLen[2];
		
		if (memcmp(fetchOrder, order, resultLen[2]) || strlen(order) != resultLen[2])
		{
			free(licenseData);
			free(fetchOrder);
			continue;
		}
		
		bool motherBoardInit;
		boundResults[1].is_null = &motherBoardInit;
		mysql_stmt_fetch_column(preparedStatement, &boundResults[1], 1, index);
		if (motherBoardInit)
		{
			// schedule motherboard init.
			licenseData->motherboardUpdateRequired = true;
			licenseData->motherboardIdLen = 0;
			licenseData->motherboardId = NULL;
		}
		else
		{
			char * motherboard = malloc(resultLen[1]);
			boundResults[2].buffer = motherboard;
			mysql_stmt_fetch_column(preparedStatement, &boundResults[1], 1, index); // again.
			licenseData->motherboardId = motherboard;
			licenseData->motherboardIdLen = resultLen[1];
			licenseData->motherboardUpdateRequired = false;
		}
		
		char * decryptPasswordResult = malloc(resultLen[3]);
		boundResults[3].buffer = decryptPasswordResult;
		mysql_stmt_fetch_column(preparedStatement, &boundResults[3], 3, index);
		licenseData->decryptPassword = decryptPasswordResult;
		licenseData->decryptPasswordLen = resultLen[3];
		
		index ++;
		current->licenseData = licenseData;
		ListNode * last = current;
		current = malloc(sizeof(ListNode));
		last->next = current;
		current->licenseData = NULL;
		current->next = NULL;
	}
	
	// count size
	int size = 0;
	current = licenseList;
	while (current->next)
	{
		size ++;
		current = current->next;
	}
	*amount = size;
	
	// move to result section and free them. And covert them to array.
	result = malloc(sizeof(LicenseData) * size);
	LicenseData * data = result;
	current = licenseList;
	if (current->next)
	{
		do
		{
			if (current->licenseData)
				memcpy(data, current->licenseData, sizeof(LicenseData));
			data++;
			ListNode *n = current;
			current = current->next;
			free(n);
		} while (current);
	}
	else
	{
		free(result);
		result = NULL;
	}
	
	final:
	if (query)
		free(query);
	if (preparedStatement)
	{
		mysql_stmt_free_result(preparedStatement);
		mysql_stmt_close(preparedStatement);
	}
	
	return result;
}

void updateMotherboard(WrappedMySQLSession * session, char * order, size_t orderLen, int userId, int resourceId, char * motherboard, size_t motherboardIdLen)
{
	char * finalQuery = malloc(sizeof(char) * strlen(UPDATE_COMMAND) + 1 + strlen(TABLE_NAME) * sizeof(char));
	memset(finalQuery, 0, sizeof(char) * strlen(UPDATE_COMMAND) + 1 + strlen(TABLE_NAME) * sizeof(char));
	
	sprintf(finalQuery, UPDATE_COMMAND, TABLE_NAME);
	
	MYSQL_STMT * updateStatement = mysql_stmt_init(session->sock);
	mysql_stmt_prepare(updateStatement, finalQuery, strlen(finalQuery));
	
	MYSQL_BIND args[4];
	memset(args, 0, sizeof(MYSQL_BIND) * 4);
	
	args[0].buffer_type = MYSQL_TYPE_VARCHAR;
	args[0].buffer = motherboard;
	args[0].buffer_length = motherboardIdLen;
	
	args[1].buffer_type = MYSQL_TYPE_LONG;
	args[1].buffer = &userId;
	args[1].buffer_length = sizeof(int);
	
	args[2].buffer_type = MYSQL_TYPE_LONG;
	args[2].buffer = &resourceId;
	args[2].buffer_length = sizeof(int);
	
	args[3].buffer_type = MYSQL_TYPE_VARCHAR;
	args[3].buffer = order;
	args[3].buffer_length = orderLen;
	
	mysql_stmt_bind_param(updateStatement, args);
	
	mysql_stmt_execute(updateStatement);
	
	mysql_stmt_close(updateStatement);
}

void disconnectFromMySQL(WrappedMySQLSession * session)
{
	mysql_close((*session).sock);
	free(session);
}

void endMySQL()
{
	mysql_library_end();
}
