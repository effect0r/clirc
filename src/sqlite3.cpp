char* SqliteFindChannelID(sqlite3 *Database, char *Name)
{
	char *ErrorMsg = 0;
	char *Spam = 0;

	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("SELECT id FROM channels WHERE name LIKE '%q'", Name);
	int Result = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (Result == SQLITE_OK)
	{
		Spam = 0;
		do 
		{
			Result = sqlite3_step(Statement);
			if (Result == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Spam = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
				strcpy(Spam, Temp);

				free(Temp);
			}
		} while (Result == SQLITE_ROW);
	}
	if (Spam)
	{
		return Spam;
	}
	else
	{
		return 0;
	}
}

char* SqliteFindTriggerID(sqlite3 *Database, char *TriggerWord)
{
	char *ErrorMsg = 0;
	char *Spam = 0;

	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("SELECT id FROM triggers WHERE trigger LIKE '%q'", TriggerWord);
	int Result = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (Result == SQLITE_OK)
	{
		Spam = 0;
		do 
		{
			Result = sqlite3_step(Statement);
			if (Result == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Spam = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
				strcpy(Spam, Temp);

				free(Temp);
			}
		} while (Result == SQLITE_ROW);
	}
	if (Spam)
	{
		return Spam;
	}
	else
	{
		return 0;
	}
}

char* SqliteFindTrigger(sqlite3 *Database, char *TriggerWord)
{
	char *ErrorMsg = 0;
	char *Spam = 0;

	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("SELECT message FROM triggers WHERE trigger LIKE '%q'", TriggerWord);
	int Result = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (Result == SQLITE_OK)
	{
		Spam = 0;
		do 
		{
			Result = sqlite3_step(Statement);
			if (Result == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Spam = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
				strcpy(Spam, Temp);

				free(Temp);
			}
		} while (Result == SQLITE_ROW);
	}
	if (Spam)
	{
		return Spam;
	}
	else
	{
		return 0;
	}
}

void SqliteSelectAndJoinChannels(irc_connection *Connection, sqlite3 *Database)
{
	sqlite3_stmt *Statement;

	char *Query = "SELECT name FROM channels;";
	int Result = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (Result == SQLITE_OK)
	{
		do
		{
			Result = sqlite3_step(Statement);
			if (Result == SQLITE_ROW)
			{
				JoinChannel(Connection, (char*)sqlite3_column_text(Statement, 0));
			}
		} while (Result == SQLITE_ROW);
	}
}

int SqliteAddTrigger(sqlite3 *Database, char *Trigger, char *Spam, char *Channel)
{
	char *ErrorMsg = 0;
	sqlite3_stmt *Statement;
	char TableName[256];
	snprintf(TableName, sizeof(TableName), "triggers_%s", Channel);
	char *Query = sqlite3_mprintf("CREATE TABLE '%q' (trigger TEXT, message TEXT);", TableName);

	int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
	if (Result == SQLITE_OK)
	{
		return 1;
	}
	else
	{
		if (strstr(ErrorMsg, "already exists"))
		{

		}
		sqlite3_free(ErrorMsg);
		return 0;
	}
}

int SqliteRemoveTrigger(sqlite3 *Database, char *Trigger, char *ChannelName)
{
	char *TriggerID = 0;
	char *ErrorMsg = 0;
	TriggerID = SqliteFindTriggerID(Database, Trigger);
	if (TriggerID)
	{
		char TableName[256];
		snprintf(TableName, sizeof(TableName), "triggers_%s", ChannelName);

		char *Query = sqlite3_mprintf("DELETE FROM '%q' WHERE id='%q'", TableName, TriggerID);

		int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
		if (Result == SQLITE_OK)
		{
			return 1;
		}
		else
		{
			sqlite3_free(ErrorMsg);
		}
		free(TriggerID);
	}
	return 0;
}

int SqliteRemoveChannel(sqlite3 *Database, char *Name)
{
	char *ChannelID = 0;
	char *ErrorMsg = 0;
	ChannelID = SqliteFindChannelID(Database, Name);
	if (ChannelID)
	{
		char *Query = sqlite3_mprintf("DELETE FROM channels WHERE id='%q'", ChannelID);

		int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
		if (Result == SQLITE_OK)
		{
			char TableName[256];
			snprintf(TableName, sizeof(TableName), "triggers_%s", Name);
			Query = sqlite3_mprintf("DROP TABLE '%q';", TableName);
		
			Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
			if (Result == SQLITE_OK)
			{
				snprintf(TableName, sizeof(TableName), "quotes_%s", Name);
				Query = sqlite3_mprintf("DROP TABLE '%q';", TableName);

				Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
				if (Result == SQLITE_OK)
				{
					return 1;
				}
			}
		}
		sqlite3_free(ErrorMsg);
		free(ChannelID);
	}
	return 0;
}

int SqliteInsertChannel(sqlite3 *Database, char *Name, char *Owner)
{
	char *ErrorMsg = 0;
	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("INSERT INTO channels (name,owner) values ('%q','%q')", Name, Owner);

	int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
	if (Result == SQLITE_OK)
	{
		char TableName[256];
		snprintf(TableName, sizeof(TableName), "triggers_%s", Name);
		char *Query = sqlite3_mprintf("CREATE TABLE '%q' (trigger TEXT, message TEXT);", TableName);

		int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
		if (Result == SQLITE_OK)
		{
			snprintf(TableName, sizeof(TableName), "quotes_%s", Name);
			Query = sqlite3_mprintf("CREATE TABLE '%q' (id INTEGER PRIMARY KEY AUTOINCREMENT, server TEXT, channel TEXT, who TEXT, text TEXT, timestamp INTEGER);", TableName);

			int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
			if (Result == SQLITE_OK)
			{
				return 1;
			}
		}
	}
	if (ErrorMsg)
	{
		sqlite3_free(ErrorMsg);
	}
	return 0;
}


