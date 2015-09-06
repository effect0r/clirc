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

void RehashCommandsList(irc_connection *Connection)
{
	// TODO(cory): free the ConfigInfo within Connection, except ConfigFileName.
	config_info *Info = &Connection->ConfigInfo;

	char *ValueAddress = 0;
	for (unsigned int i = 0; i < Info->FaqCommandsMap->Used; ++i)
	{
		pair *Current = Info->FaqCommandsMap->Pairs + i;
		if (Current->Key)
		{
			free(Current->Key);
			Current->Key = 0;
		}
		if (Current->Value && (ValueAddress != Current->Value))
		{
			free(Current->Value);
			ValueAddress = Current->Value;
			Current->Value = 0;
		}
	}

	free(Info->FaqCommandsMap);

	ProcessInfoDB(Connection);
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
						char *MessageToSend = Connection->ConfigInfo.FaqCommandsMap->Pairs[CommandNumber].Value;
						SendMessage(Connection, "#effect0r", MessageToSend);
					}
					else if (!strcmp(text, "about"))
					{
						SendMessage(Connection, "#effect0r", "Greetings meat popcicle! I'm an IRC bot written in c/c++ to help moderate this channel! For more commands, see the bot's \"commands\" command.");
					}
					else if (!strcmp(text, "rehash"))
					{
						if (!strcmp(FromNick, Connection->ConfigInfo.Admin))
						{
							RehashCommandsList(Connection);
							SendMessage(Connection, "#effect0r", "Rehashing commands list... DONE!");
						}
					}
					else
					{
						if (!strcmp(text, "quote"))
						{
							char *ErrorMsg = 0;
							char *Query = sqlite3_mprintf("SELECT * FROM quote WHERE id='%q';", Param);
							sqlite3_stmt *Statement;

							int Result = sqlite3_prepare_v2(Connection->ConfigInfo.QuoteList.QuoteDB, Query, strlen(Query)+1, &Statement, 0);
							if (Result == SQLITE_OK) 
							{
								char *IDList = 0, *Text = 0, *Time = 0;
								do
								{
									Result = sqlite3_step(Statement);
									if (Result == SQLITE_ROW)
									{
										char *Temp = (char*)malloc(sizeof(char) * 1024);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
										IDList = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(IDList, Temp);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 1));
										Text = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(Text, Temp);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 2));
										Time = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(Time, Temp);

										free(Temp);

									}
								} while(Result == SQLITE_ROW);

								if (IDList && Text && Time)
								{
									struct tm tv;
									ZERO(&tv, struct tm);
									char FormattedTime[256];
									char Message[1024];

									strptime(Time, "%s", &tv);
									strftime(FormattedTime, sizeof(FormattedTime), "%d %b %Y", &tv);
									snprintf(Message, sizeof(Message), "(#%s) \"%s\" --Casey, %s", IDList, Text, FormattedTime);

									SendMessage(Connection, "#effect0r", Message);
								}
								else 
								{
									char Message[256];
									snprintf(Message, sizeof(Message), "No quote with IDList=%s", Param);
									SendMessage(Connection, "#effect0r", Message);
								}
							}
							else
							{
								sqlite3_free(ErrorMsg);
							}
						}
						else if (!strcmp(text, "fixquotetime"))
						{
							// NOTE(cory): fixquotetime 01 jan 2015
							char *Time = 0;
							char *ID = 0;
							char Buffer[256];
							struct tm tv;
							ZERO(&tv, struct tm);

							if (Param)
							{
								ID = Param;
								Time = strchr(Param, ' ');
								int IDCheck = atoi(ID);
								if (IDCheck > 0)
								{
									if (Time)
									{
										*Time++ = '\0';
										if (strptime(Time, "%d %b %Y", &tv))										
										{
											time_t EpochTime = mktime(&tv);
											char *ErrorMsg = 0;
											char TimeAsString[256];
											snprintf(TimeAsString, sizeof(TimeAsString), "%lu", EpochTime);

											char *Query = sqlite3_mprintf("UPDATE quote SET timestamp='%q' WHERE id='%q';", TimeAsString, ID);

											int Result = sqlite3_exec(Connection->ConfigInfo.QuoteList.QuoteDB, Query, 0, 0, &ErrorMsg);
											if (Result == SQLITE_OK)
											{
												sqlite3_stmt *Statement;
												Query = sqlite3_mprintf("SELECT timestamp FROM quote WHERE id='%q';", ID);
												int Result = sqlite3_prepare_v2(Connection->ConfigInfo.QuoteList.QuoteDB, Query, strlen(Query)+1, &Statement, 0);
												if (Result == SQLITE_OK)
												{
													char *Timestamp = 0;
													do 
													{
														Result = sqlite3_step(Statement);
														if (Result == SQLITE_ROW)
														{
															char *Temp = (char*)malloc(sizeof(char) * 1024);

															strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
															Timestamp = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
															strcpy(Timestamp, Temp);

															free(Temp);
														}
													} while (Result == SQLITE_ROW);

													if (Timestamp)
													{
														char FormattedTime[256];

														strptime(Timestamp, "%s", &tv);
														strftime(FormattedTime, sizeof(FormattedTime), "%d %b %Y", &tv);

														snprintf(Buffer, sizeof(Buffer), "Quote %s moved to date: %s", ID, FormattedTime);
													}
												}
											}
											else
											{
												sqlite3_free(ErrorMsg);
											}
										}
										else
										{
											//invalid format
											snprintf(Buffer, sizeof(Buffer), "You must provide a date in the formate <dd> <Mon(th)> <yyyy>");
										}
									}
									else
									{
										//invalid id
										snprintf(Buffer, sizeof(Buffer), "You must provide a time to change ID to.");
									}
								}
								else
								{
									//no time given
									snprintf(Buffer, sizeof(Buffer), "You must provide a valid ID.");
								}
							}
							else 
							{
								//no string given
								snprintf(Buffer, sizeof(Buffer), "Usage: fixquotetime <id> <dd> <Mon(th)> <yyyy>");
							}

							SendMessage(Connection, "#effect0r", Buffer);
						}
						else if (!strcmp(text, "fixquote"))
						{
							char *ID = 0;
							char *Update = 0;

							// NOTE(cory): fixquote <id> <update text>
							if (Param)
							{
								char *ErrorMsg = 0;
								ID = Param;
								int IDAsNumber = atoi(ID);
								if (IDAsNumber > 0)
								{
									Update = strchr(Param, ' ');
									if (Update)
									{
										*Update++ = '\0';

										char *Query = sqlite3_mprintf("UPDATE quote SET text='%q' WHERE id='%q';", Update, ID);

										int Result = sqlite3_exec(Connection->ConfigInfo.QuoteList.QuoteDB, Query, 0, 0, &ErrorMsg);
										if (Result == SQLITE_OK)
										{
											sqlite3_stmt *Statement;
											Query = sqlite3_mprintf("SELECT id FROM quote WHERE id='%q';", ID);

											int Result = sqlite3_prepare_v2(Connection->ConfigInfo.QuoteList.QuoteDB, Query, strlen(Query)+1, &Statement, 0);
											if (Result == SQLITE_OK)
											{
												char *ResultID = 0;
												do
												{
													Result = sqlite3_step(Statement);
													if (Result == SQLITE_ROW)
													{
														char *Temp = (char*)malloc(sizeof(char)*1024);

														strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
														ResultID = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
														strcpy(ResultID, Temp);

														free(Temp);
													}
												} while (Result == SQLITE_ROW);

												if (ResultID)
												{
													char Buffer[256];
													snprintf(Buffer, sizeof(Buffer),  "Updated quote ID=%s to \"%s\"", ResultID, Update);
													SendMessage(Connection, "#effect0r", Buffer);

												}
											}
										}
										else
										{
											sqlite3_free(ErrorMsg);
										}
									}
									else
									{
										SendMessage(Connection, "#effect0r", "Usage: fixquote <id> <update text>");
									}
								}
								else 
								{
									SendMessage(Connection, "#effect0r", "Usage: fixquote <id> <update text>");
								}

							}
							else
							{
								SendMessage(Connection, "#effect0r", "Usage: fixquote <id> <update text>");
							}
						}
						else if (!strcmp(text, "addquote"))
						{
							char *ErrorMsg = 0;
							time_t Now = time(0);
							char CurrentTime[256];
							snprintf(CurrentTime, sizeof(CurrentTime), "%lu", Now);
							sqlite3_stmt *Statement;

							char *Query = sqlite3_mprintf("INSERT INTO quote (text,timestamp) VALUES ('%q','%q');", Param, CurrentTime);
							int Result = sqlite3_exec(Connection->ConfigInfo.QuoteList.QuoteDB, Query, 0, 0, &ErrorMsg);
							if (Result == SQLITE_OK)
							{
								Query = sqlite3_mprintf("SELECT id FROM quote WHERE timestamp='%q';", CurrentTime);
								Result = sqlite3_prepare_v2(Connection->ConfigInfo.QuoteList.QuoteDB, Query, strlen(Query)+1, &Statement, 0);
								if (Result == SQLITE_OK)
								{
									char *ID = 0;
									do
									{
										Result = sqlite3_step(Statement);
										if (Result == SQLITE_ROW)
										{
											char *Temp = (char*)malloc(sizeof(char) * 128);
											strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
											ID = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
											strcpy(ID, Temp);
											free(Temp);
										}
									} while (Result == SQLITE_ROW);

									if (ID)
									{
										char Msg[256];
										snprintf(Msg, sizeof(Msg),  "Added quote ID=%s", ID);
										SendMessage(Connection, "#effect0r", Msg); 
									}
								}
							}
							else 
							{
								//failed to insert
								sqlite3_free(ErrorMsg);
							}
						}
						else if (!strcmp(text, "searchquote"))
						{
							char *ErrorMsg = 0;
							char *Query = sqlite3_mprintf("SELECT * FROM quote where text LIKE '%%%q%%'", Param);
							sqlite3_stmt *Statement;

							int Result = sqlite3_prepare_v2(Connection->ConfigInfo.QuoteList.QuoteDB, Query, strlen(Query)+1, &Statement, 0);
							if (Result == SQLITE_OK)
							{
								char *IDList[1024];
								char *TextList[1024];
								char *TimeList[1024];

								int Position = 0;

								do
								{
									Result = sqlite3_step(Statement);
									if (Result == SQLITE_ROW)
									{
										char *Temp = (char*)malloc(sizeof(char) *1024);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
										IDList[Position] = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(IDList[Position], Temp);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 1));
										TextList[Position] = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(TextList[Position], Temp);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 2));
										TimeList[Position] = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(TimeList[Position], Temp);

										++Position;
										free(Temp);
									}
								} while (Result == SQLITE_ROW);

								char Buffer[1024];
								if (Position)
								{
									if (Position == 1)
									{
										struct tm tv;
										char FormattedTime[256];
										ZERO(&tv, struct tm);

										strptime(TimeList[0], "%s", &tv);
										strftime(FormattedTime, sizeof(FormattedTime), "%d %b %Y", &tv);

										snprintf(Buffer, sizeof(Buffer),  "(#%s)\"%s\" --Casey, %s", IDList[0], TextList[0], FormattedTime);
									}
									else
									{
										char QuoteNums[1024];
										strcpy(QuoteNums, IDList[0]);
										for (int i = 1; i < Position; ++i)
										{
											char Temp[16];
											snprintf(Temp, sizeof(Temp), ", %s", IDList[i]);
											strcat(QuoteNums, Temp);
										}

										snprintf(Buffer, sizeof(Buffer), "Found %d quotes containing string %s: %s", Position, Param, QuoteNums);
									}
								}
								else
								{
									snprintf(Buffer, sizeof(Buffer), "No quotes found with string \"%s\"", Param);
								}

								SendMessage(Connection, "#effect0r", Buffer);
								// NOTE(cory): Cleanup.
								for (int i = 0; i < Position; ++i)
								{
									free(IDList[i]);
									free(TextList[i]);
									free(TimeList[i]);
								}
							}
						}
						else if(!strcmp(text, "delquote"))
						{
							char *ErrorMsg = 0;
							char *Query = sqlite3_mprintf("SELECT text FROM quote WHERE ID='%q'", Param);
							sqlite3_stmt *Statement;

							int Result = sqlite3_prepare_v2(Connection->ConfigInfo.QuoteList.QuoteDB, Query, strlen(Query)+1, &Statement, 0);
							if (Result == SQLITE_OK)
							{
								char *QuoteText = 0;
								do
								{
									Result = sqlite3_step(Statement);
									if (Result == SQLITE_ROW)
									{
										char *Temp = (char*)malloc(1024);

										strcpy(Temp, (char*)sqlite3_column_text(Statement, 0));
										QuoteText = (char*)malloc(sizeof(char) * (strlen(Temp) + 1));
										strcpy(QuoteText, Temp);

										free(Temp);
									}
								} while (Result == SQLITE_ROW);

								if (QuoteText)
								{
									// NOTE(cory): Quote exists because QuoteText has data.
									Query = sqlite3_mprintf("DELETE FROM quote WHERE id='%q'", Param);
									if (sqlite3_exec(Connection->ConfigInfo.QuoteList.QuoteDB, Query, 0, 0, &ErrorMsg) == SQLITE_OK)
									{
										char Msg[256];
										snprintf(Msg, sizeof(Msg),  "Deleted quote ID=%s", Param);
										SendMessage(Connection, "#effect0r", Msg); 
									}
								}
								else
								{
									char Message[256];
									snprintf(Message, sizeof(Message),  "Quote ID=%s does not exist.", Param);
									SendMessage(Connection, "#effect0r", Message);
								}
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
