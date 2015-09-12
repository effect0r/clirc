int SqliteRemoveFromWhiteList(sqlite3 *Database, char *Channel, char *Name)
{
	char *ErrorMsg = 0;
	int Result = 1;
	sqlite3_stmt *Statement;
	char *Whitelist = 0;

	char *Query = sqlite3_mprintf("SELECT whitelist FROM channels WHERE name LIKE '%q'", Channel);
	int ResultMainQuery = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (ResultMainQuery == SQLITE_OK)
	{
		do 
		{
			ResultMainQuery = sqlite3_step(Statement);
			if (ResultMainQuery == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Whitelist = (char*)malloc(sizeof(char*) * (strlen(Temp) + 1));
				strcpy(Whitelist, Temp);

				if (strlen(Whitelist) != 0)
				{
					//Remove from the list.
					char *PositionOfUser = strstr(Whitelist, Name);
					if (PositionOfUser)
					{
						if (Whitelist == PositionOfUser)
						{
							//Beginning of list.
							char *WhitelistComma = strchr(PositionOfUser, ',');
							if (WhitelistComma)
							{
								*WhitelistComma++ = '\0';
								char *TempWhitelist = (char*)malloc(sizeof(char) * (strlen(WhitelistComma) + 1));
								strcpy(TempWhitelist, WhitelistComma);

								free(Whitelist);

								Whitelist = (char*)malloc(sizeof(char) * (strlen(TempWhitelist) + 1));
								strcpy(Whitelist, TempWhitelist);
							}
							else
							{
								// whitelist has only one name in it.
								*Whitelist = '\0';
							}
						}
						else
						{
							//Elsewhere in list. 
							*PositionOfUser++ = '\0';
							//scan forward to next comma
							char *WhitelistComma = strchr(PositionOfUser, ',');
							if (WhitelistComma)
							{
								*WhitelistComma++ = '\0';
								char *TempWhiteList = (char*)malloc(sizeof(char) * ((strlen(Whitelist)) + (strlen(WhitelistComma) + 1)));
								char *TempAfterComma = (char*)malloc(sizeof(char) * (strlen(WhitelistComma) +1));
								strcpy(TempWhiteList, Whitelist);
								strcpy(TempAfterComma, WhitelistComma);

								free(Whitelist);

								strcat(TempWhiteList, WhitelistComma);
								Whitelist = (char*)malloc(sizeof(char) * (strlen(TempWhiteList) + 1));
								strcpy(Whitelist, TempWhiteList);
								
								free(TempWhiteList);
								free(TempAfterComma);

							}
							if (Whitelist[strlen(Whitelist) - 1] == ',')
							{
								Whitelist[strlen(Whitelist) - 1] = '\0';
							}
						}
					}

					Query = sqlite3_mprintf("UPDATE channels SET whitelist = '%q' where name LIKE '%q'", Whitelist, Channel);
					int ResultNonEmptyWhiteList = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
					if (ResultNonEmptyWhiteList != SQLITE_OK)
					{
						free(ErrorMsg);
						Result = 0;
					}
					free(Temp);
					break;
				}
				else
				{
					free(Temp);
					break;
				}
			}
		} while (ResultMainQuery == SQLITE_ROW);
	}
	if (Whitelist)
	{
		free(Whitelist);
	}

	return Result;
}

int SqliteAddToWhitelist(sqlite3 *Database, char *Channel, char *Name)
{
	char *ErrorMsg = 0;
	int Result = 1;
	sqlite3_stmt *Statement;

	char *Query = sqlite3_mprintf("SELECT whitelist FROM channels WHERE name LIKE '%q'", Channel);
	int ResultMainQuery = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (ResultMainQuery == SQLITE_OK)
	{
		char *Whitelist = 0;
		do 
		{
			ResultMainQuery = sqlite3_step(Statement);
			if (ResultMainQuery == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Whitelist = (char*)malloc(sizeof(char*) * (strlen(Temp) + 1));
				strcpy(Whitelist, Temp);

				if (strlen(Whitelist) == 0)
				{
					// Empty whitelist
					Query = sqlite3_mprintf("UPDATE channels set whitelist = '%q' where name LIKE '%q'", Name, Channel);
					int ResultEmptyWhitelist = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
					if (ResultEmptyWhitelist != SQLITE_OK)
					{
						free(ErrorMsg);
						Result = 0;
					}
					free(Temp);
					break;

				}
				else
				{
					//concactenate onto the list.
					Query = sqlite3_mprintf("UPDATE channels SET whitelist = whitelist || ',%q' where name LIKE '%q'", Name, Channel);
					int ResultNonEmptyWhiteList = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
					if (ResultNonEmptyWhiteList != SQLITE_OK)
					{
						free(ErrorMsg);
						Result = 0;
					}
					free(Temp);
					break;
				}
			}
		} while (ResultMainQuery == SQLITE_ROW);
	}
	return Result;
}

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

int SqliteIsChannelAdmin(sqlite3 *Database, char *Channel, char *Name)
{
	int Result = 0;
	char *Admin = 0;
	
	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("SELECT owner FROM channels WHERE name LIKE '%q'", Channel);
	int QueryResult = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (QueryResult == SQLITE_OK)
	{
		do 
		{
			QueryResult = sqlite3_step(Statement);
			if (QueryResult == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Admin = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
				strcpy(Admin, Temp);

				free(Temp);
			}
		} while (QueryResult == SQLITE_ROW);
	}

	if (Admin)
	{
		if (strstr(Admin, Name))
		{
			Result = 1;
		}
		free(Admin);
	}
	return Result;
}

int SqliteIsWhitelistedOnChannel(sqlite3 *Database, char *Channel, char *Name)
{
	int Result = 0;
	char *Whitelist = 0;
	
	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("SELECT whitelist FROM channels WHERE name LIKE '%q'", Channel);
	int QueryResult = sqlite3_prepare_v2(Database, Query, strlen(Query) + 1, &Statement, 0);
	if (QueryResult == SQLITE_OK)
	{
		do 
		{
			QueryResult = sqlite3_step(Statement);
			if (QueryResult == SQLITE_ROW)
			{
				char *Temp = (char*)malloc(sizeof(char) * 1024);

				strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
				Whitelist = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
				strcpy(Whitelist, Temp);

				free(Temp);
			}
		} while (QueryResult == SQLITE_ROW);
	}

	if (Whitelist)
	{
		if (strstr(Whitelist, Name))
		{
			Result = 1;
		}
		free(Whitelist);
	}
	return Result;
}
