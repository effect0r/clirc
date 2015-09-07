/*
 *TODO(effect0r):
 - Restructure the database such that the following is possible.
 - Move everything to the database except what's in the core config
 - Have a 'main' channel (the bot's name) be a general administration channel
 - Within this channel:
 - Add other users' channel to the bots list.
 - Set them to the admin of that channel.
 - Within the users' channel:
 - Add in trigger-spam pairs based on network and channel, and permisions.
 - Set up whitelisted users for specific commands.
 */
#include "irc_connection.h"

void PartChannel(irc_connection *Connection, char *Channel)
{
	if (Connection->IsConnected)
	{
		fprintf(Connection->OutStream, "PART %s\r\n", Channel);
		fflush(Connection->OutStream);
	}
}

void JoinChannel(irc_connection *Connection, char *Channel)
{
	if (Connection->IsConnected)
	{
		fprintf(Connection->OutStream, "JOIN %s\r\n", Channel);
		fflush(Connection->OutStream);
	}
}

void SendMessage(irc_connection *Connection, char *Channel, char *Message)
{
	if (Connection->IsConnected)
	{
		fprintf(Connection->OutStream, "PRIVMSG %s :%s\r\n", Channel, Message);
		fflush(Connection->OutStream);
	}
}

void CloseConnection(irc_connection *Connection)
{
	if (Connection->IsConnected)
	{
		fprintf(Connection->OutStream, "QUIT Leaving.\r\n");
	}
	fflush(Connection->OutStream);

	Connection->IsConnected = 0;

	free(Connection->ConfigInfo.Port);
	free(Connection->ConfigInfo.Nick);
	free(Connection->ConfigInfo.Server);
	free(Connection->ConfigInfo.User);
	free(Connection->ConfigInfo.Pass);
	fclose(Connection->OutStream);
}

#if 0
void RehashCommandsList(irc_connection *Connection, int ChannelNumber)
{
	config_info *Info = &Connection->ConfigInfo.ChannelList[ChannelNumber];
	char *FileName = Info->TriggersFileName;

	char *ValueAddress = 0;
	for (unsigned int i = 0; i < Info->TriggerMap.Used; ++i)
	{
		pair *Current = &Info->TriggerMap.Pairs[i];
		if (Current->Key)
		{
			free(Current->Key);
			Current->Key = 0;
		}
		if (Current->Value && (Current->Value != ValueAddress ))
		{
			free(Current->Value);
			ValueAddress = Current->Value;
			Current->Value = 0;
		}
	}

	free(Info->TriggerMap.Pairs);

	ProcessChannelTriggers(Connection, ChannelNumber, FileName);
}
#endif

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
			return 1;
		}
		else
		{
			sqlite3_free(ErrorMsg);
		}
		free(ChannelID);
	}
	return 0;
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

int SqliteRemoveTrigger(sqlite3 *Database, char *Trigger)
{
	char *TriggerID = 0;
	char *ErrorMsg = 0;
	TriggerID = SqliteFindTriggerID(Database, Trigger);
	if (TriggerID)
	{
		char *Query = sqlite3_mprintf("DELETE FROM triggers WHERE id='%q'", TriggerID);

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

int SqliteInsertTrigger(sqlite3 *Database, char *Trigger, char *Spam, char *Channel)
{
	char *ErrorMsg = 0;
	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("INSERT INTO triggers (trigger, message, channel) VALUES('%q','%q','%q')", Trigger, Spam, Channel);

	int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
	if (Result == SQLITE_OK)
	{
		return 1;
	}
	else
	{
		sqlite3_free(ErrorMsg);
		return 0;
	}
}

int SqliteInsertChannel(sqlite3 *Database, char *Name, char *Owner)
{
	char *ErrorMsg = 0;
	sqlite3_stmt *Statement;
	char *Query = sqlite3_mprintf("INSERT INTO channel (name,owner) values ('%q','%q')", Name, Owner);

	int Result = sqlite3_exec(Database, Query, 0, 0, &ErrorMsg);
	if (Result == SQLITE_OK)
	{
		return 1;
	}
	else
	{
		sqlite3_free(ErrorMsg);
		return 0;
	}
}

int IsChannelAdmin(irc_connection *Connection, char *Name)
{
	if (!strcmp(Name, "effect0r"))
	{
		return 1;
	}
	return 0;
}

int IsMainAdmin(irc_connection *Connection, char *Name)
{
	for (int i = 0; i < Connection->ConfigInfo.AdminCount; ++i)
	{
		if (!strcmp(Connection->ConfigInfo.Admins[i], Name))
		{
			return 1;
		}
	}
	return 0;
}

void ParseMessage(irc_connection *Connection, char *Message)
{
	char *Command;
	char *HostName;
	char *Parameters;

	struct timeval TimeVal;

	gettimeofday(&TimeVal, 0);

	printf("%s\n", Message);

	if (Message[0] == ':')
	{
		HostName = &Message[1];
		Command = strchr(HostName, ' ');
		if (!Command)
		{
			return;
		}
		*Command++ = '\0';
		Parameters = strchr(Command, ' ');
		if (Parameters)
		{
			*Parameters++ = '\0';
		}
		char *FromNick = HostName;
		char *ident = strchr(FromNick, '!');
		if (ident)
		{
			*ident++ = '\0';
		}

		if (!strcmp(Command, "PRIVMSG"))
		{
			char *MessageChannel = Parameters;
			char *text = strchr(Parameters, ':');
			int ChannelNumber = -1;
			if (text)
			{
				*text++ = '\0';
			}
			char *ChannelTemp = strchr(MessageChannel, ' ');
			if (ChannelTemp)
			{
				*ChannelTemp++ = '\0';
			}
			// NOTE(effect0r):IMPORTANT(effect0r): This is the _only_ place that admin commands can happen!
			if (!strcmp(MessageChannel, "#effect0r-cpp"))
			{
				if (IsMainAdmin(Connection, FromNick))
				{
					if (text[0] == Connection->ConfigInfo.CommandPrefix)
					{
						*text++ = '\0';

						char *Param = strchr(text, ' ');
						if (Param)
						{
							*Param++ = '\0';
						}
						if (!strcmp(text, "help"))
						{
							char Message[256];
							char *AboutMsg = "Greetings meat popsicle! I'm an IRC bot written in c/c++ to help moderate this channel! For more commands, see the \"commands\" command.";
							snprintf(Message, sizeof(Message), "@%s: %s", FromNick, AboutMsg);
							SendMessage(Connection, MessageChannel, Message);
						}
						else if (!strcmp(text, "addchannel"))
						{
							// name, owner
							char *Name = Param;
							if (Name[0] != '#')
							{
								SendMessage(Connection, MessageChannel, "Usage: addchannel <channelname> <channelowner>");
								return;
							}
							char *Owner = strchr(Param, ' ');
							if (Owner) 
							{
								*Owner++ = '\0';
								while (*Owner == ' ') Owner++;
							}

							if (*Owner && (*Owner != '\n'))
							{
								if (SqliteInsertChannel(Connection->ConfigInfo.Database, Name, Owner))
								{
									char Message[256];
									snprintf(Message, sizeof(Message), "Successfully added \"%s\" (with owner \"%s\") to the channel list.", Name, Owner);

									SendMessage(Connection, MessageChannel, Message);
									JoinChannel(Connection, Name);
								}
							}
							else
							{
								SendMessage(Connection, MessageChannel, "Usage: addchannel <channelname> <channelowner>");
								return;
							}
						}
						else if (!strcmp(text, "rmchannel"))
						{
							char *Name = Param;
							if (*Name != '#')
							{
								SendMessage(Connection, MessageChannel, "Usage: rmaddchannel <channelname>");
								return;
							}
							char *Space = strchr(Name, ' ');
							if (Space)
							{
								*Space++ = '\0';
							}
							if (*Name && (*Name != '\n'))
							{
								if (SqliteRemoveChannel(Connection->ConfigInfo.Database, Name))
								{
									char Message[256];
									snprintf(Message, sizeof(Message), "Successfully removed \"%s\" from the channel list.", Name);

									SendMessage(Connection, MessageChannel, Message);
									PartChannel(Connection, Name);
								}
							}
						}
					}
				}
			}
			// NOTE(effect0r): This is the place that channel-specific commands happen.
			else
			{
				if (IsChannelAdmin(Connection, FromNick))
				{
					char *Param = strchr(text, ' ');
					if (Param)
					{
						*Param++ = '\0';
					}
					if (text[0] == Connection->ConfigInfo.CommandPrefix)
					{
						*text++ = '\0';
						if (!strcmp(text, "whitelist"))
						{

						}
						else if (!strcmp(text, "trigger"))
						{
							char *Trigger = Param;
							char *Spam = strchr(Param, ' ');
							if (Spam)
							{
								*Spam++ = '\0';
							}
							if (*Trigger && (*Trigger != '\n'))
							{
								if (SqliteInsertTrigger(Connection->ConfigInfo.Database, Trigger, Spam, MessageChannel))
								{
									char Message[256];
									snprintf(Message, sizeof(Message), "Successfully added \"%s\" to the trggers list.", Trigger);

									SendMessage(Connection, MessageChannel, Message);
								}
							}
							else
							{
								SendMessage(Connection, MessageChannel, "Useage: trigger <trigger alias> <message to send>");
							}
						}
						else if (!strcmp(text, "rmtrigger"))
						{
							char *Trigger = Param;
							char *Errata = strchr(Param, ' ');
							if (Errata)
							{
								SendMessage(Connection, MessageChannel, "Usage: rmtrigger <trigger alias>");
								return;
							}
							if (*Trigger && (*Trigger != '\n'))
							{
								if (SqliteRemoveTrigger(Connection->ConfigInfo.Database, Trigger))
								{
									char Msg[256];
									snprintf(Msg, sizeof(Msg), "Successfully removed \"%s\" from the triggers list.", Trigger);

									SendMessage(Connection, MessageChannel, Msg);
								}
							}
							else
							{
								SendMessage(Connection, MessageChannel, "Usage: rmtrigger <trigger alias>");
								return;
							}
						}
					}
					else
					{
						//look into the DB for a trigger.
						char *Spam = 0;
						Spam = SqliteFindTrigger(Connection->ConfigInfo.Database, text);
						if (Spam)
						{
							char Msg[256];
							snprintf(Msg, sizeof(Msg), "@%s, %s", FromNick, Spam);

							SendMessage(Connection, MessageChannel, Msg);
							free(Spam);
						}
					}
				}
			}
		}
		// NOTE(effect0r): Join channels in the channel list after MOTD has finished.
		else if (!strcmp(Command, "376"))
		{
			char Channel[64];
			snprintf(Channel, sizeof(Channel), "#%s", Connection->ConfigInfo.Nick);
			JoinChannel(Connection, Channel);

			SqliteSelectAndJoinChannels(Connection, Connection->ConfigInfo.Database);
		}
	}
	else
	{
		Command = Message;
		Message = strchr(Command, ' ');
		if (!Message)
		{
			return;
		}
		*Message = '\0';
		Parameters = Message + 1;

		if (!strcmp(Command, "PING"))
		{
			if (!Parameters)
			{
				return;
			}
			fprintf(Connection->OutStream, "PONG %s\r\n", &Parameters[1]);
			fflush(Connection->OutStream);
		}
	}
}

void ParseReply(irc_connection *Connection, char *Message)
{
	char *Position;

	while (Position = strstr(Message, "\r\n"))
	{
		*Position = '\0';
		ParseMessage(Connection, Message);
		Message = Position + 2;
	}
}

void MessageLoop(irc_connection *Connection)
{
	char RecieveBuffer[2048];
	int RecieveLength;

	if (Connection->IsConnected)
	{
		while (Connection->IsConnected)
		{
			RecieveLength = recv(Connection->Socket, RecieveBuffer,
					sizeof(RecieveBuffer), 0);
			if (RecieveLength)
			{
				RecieveBuffer[RecieveLength] = '\0';
				ParseReply(Connection, RecieveBuffer);
			}
		}
	}
}

int Connect(irc_connection *Connection) 
{
	int ConnError;
	ZERO(&Connection->Hints, Connection->Hints);
	Connection->Hints.ai_family = AF_UNSPEC;
	Connection->Hints.ai_socktype = SOCK_STREAM;
	if ((strlen(Connection->ConfigInfo.Port) > 0) && (strlen(Connection->ConfigInfo.Port) < 0xFFFF))
	{
		Connection->Status = getaddrinfo(Connection->ConfigInfo.Server, Connection->ConfigInfo.Port, 
				&Connection->Hints, &Connection->ServerInfo);
	}
	else
	{
		printf(":ERROR: incorrect port!\n");
	}

	if (Connection->Socket = socket(Connection->ServerInfo->ai_family,
				Connection->ServerInfo->ai_socktype,
				Connection->ServerInfo->ai_protocol))
	{
		if ((ConnError = connect(Connection->Socket,
						Connection->ServerInfo->ai_addr,
						Connection->ServerInfo->ai_addrlen) >= 0))
		{
			//NOTE: We're connected.
			Connection->OutStream = fdopen(Connection->Socket, "w");
			if (Connection->OutStream)
			{
				Connection->IsConnected = 1;

				fprintf(Connection->OutStream, "PASS %s\r\n", Connection->ConfigInfo.Pass);
				fprintf(Connection->OutStream, "NICK %s\r\n", Connection->ConfigInfo.Nick);
				fprintf(Connection->OutStream, "USER %s * 0 :%s\r\n", Connection->ConfigInfo.User, Connection->ConfigInfo.Nick);
				fflush(Connection->OutStream);
			}
			else
			{
				//NOTE: Couldn't open file
				printf("Couldn't open file.\n");
			}
		}
		else
		{
			//NOTE: Couldn't connect.
			printf("Couldn't connect.\n");
		}
	}
	else
	{
		//NOTE: Couldn't open socket
		printf("Couldn't open socket.\n");
	}
	return(0);
}
