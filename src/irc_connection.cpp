/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice: $
   ==========================================================================*/

#include "irc_connection.h"

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

static int InsertQuote(void *data, int NumArgs, char **Rows, char **Columns)
{
	return 0;
}

static int SelectQuote(void *data, int NumArgs, char **Rows, char **Columns)
{
	// NOTE(effect0r): (#1) Are you okay with that, Mr. Compiler? --Casey, 26 Feb 2015
	irc_connection *Conn = (irc_connection*)data;
	char Quote[256];
	char FormattedTime[256];
	char *ID = 0;
	char *TimeStamp = 0;
	char *QuoteText = 0;

	struct tm tv;
	memset(&tv, 0, sizeof(struct tm));

	for (int i = 0; i < NumArgs; ++i)
	{
		if (!strcmp(Columns[i], "timestamp"))
		{
			TimeStamp = Rows[i];
		}
		else if (!strcmp(Columns[i], "id"))
		{
			ID = Rows[i];
		}
		else if (!strcmp(Columns[i], "text"))
		{
			QuoteText = Rows[i];
		}
	}
	strptime(TimeStamp, "%s", &tv);
	strftime(FormattedTime, sizeof(FormattedTime), "%d %b %Y", &tv);
	sprintf(Quote, "(#%s) \"%s\" --Casey, %s", ID, QuoteText, FormattedTime);

	SendMessage(Conn, "#effect0r", Quote);
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
			char *text = strchr(Parameters, ':');
			*text++ = '\0';
			if (!strcmp(FromNick, "effect0r"))
			{
				if (text[0] == Connection->ConfigInfo.CommandPrefix)
				{
					*text++ = '\0';
					char *Param = strchr(text, ' ');
					if (Param)
					{
						*Param++ = '\0';
					}
					int CommandNumber = MapSearch(Connection->ConfigInfo.FaqCommandsMap, text);
					if (CommandNumber >= 0)
					{
						//stuff
						char *MessageToSend = Connection->ConfigInfo.FaqCommandsMap->Pairs[CommandNumber].Value;
						SendMessage(Connection, "#effect0r", MessageToSend);
					}
					else
					{
						if (!strcmp(text, "quote"))
						{
							char Query[256];
							char *Start = "SELECT * FROM quote WHERE id=";
							strcpy(Query, Start);
							strcat(Query, Param);

							char *ErrorMsg = 0;
							int rc = sqlite3_exec(Connection->ConfigInfo.QuoteList.QuoteDB, Query, SelectQuote, (void*)Connection, &ErrorMsg);
							if (rc != SQLITE_OK) 
							{
								//error
								sqlite3_free(ErrorMsg);
							}
							else
							{
								//success
							}
						}
						else if (!strcmp(text, "addquote"))
						{
							char *ErrorMsg = 0;
							time_t Now = time(0);
							char CurrentTime[256];
							snprintf(CurrentTime, sizeof(CurrentTime), "%lu", Now);

							char *Query = sqlite3_mprintf("INSERT INTO quote (text,timestamp) VALUES ('%q','%q')", Param, CurrentTime);
							int rc = sqlite3_exec(Connection->ConfigInfo.QuoteList.QuoteDB, Query, InsertQuote, (void*)Connection, &ErrorMsg);
							if (rc != SQLITE_OK)
							{
								sqlite3_free(ErrorMsg);
							}
							else
							{
								//success
								//"Added as !quote %s.
								char Buffer[32];
								snprintf(Buffer, sizeof(Buffer), "Added as !quote %d", ++Connection->ConfigInfo.QuoteList.TotalQuotes);
								SendMessage(Connection, "#effect0r", Buffer);
							}
						}
					}
				}
			}
		}
		// NOTE(cory): Join channels in the channel list after MOTD has finished.
		else if (!strcmp(Command, "376"))
		{
			for (int i = 0; i < Connection->ConfigInfo.ChannelCount; ++i)
			{
				JoinChannel(Connection, Connection->ConfigInfo.ChannelList[i]);
			}
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
