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

	free(Connection->Port);
	free(Connection->Nick);
	free(Connection->Server);
	free(Connection->User);
	free(Connection->Pass);
	fclose(Connection->OutStream);
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
		*Command = '\0';
		Command++;
		Parameters = strchr(Command, ' ');
		if (Parameters)
		{
			*Parameters = '\0';
			Parameters++;
		}
		char *FromNick = HostName;
		char *ident = strchr(FromNick, '!');
		if (ident)
		{
			*ident = '\0';
			ident++;
		}
		char CommandTrigger = '!';
		if (!strcmp(Command, "PRIVMSG"))
		{
			char *text = strchr(Parameters, ':');
			*text = '\0';
			text++;
			if (!strcmp(FromNick, "effect0r"))
			{
				if (text[0] == CommandTrigger)
				{
					*text = '\0';
					text++;
					char *Param= strchr(text, ' ');
					if (Param)
					{
						*Param = '\0';
						Param++;
					}
					if (!strcmp(text, "help"))
					{
						SendMessage(Connection,	"#effect0r",
								"Greetings mortal, I am an IRC bot written in (the good parts) of c++! I was written by Cory Henderlite (effect0r) from Handmade.dev.");
					}
					if (!strcmp(text, "commands"))
					{
						SendMessage(Connection, "#effect0r", "Here are the common HH stream commands: !time, !today, !schedule, !now, !site, !old, !wrist, !who, !buy, !game, !stream, !lang, !ide, !editoradvice, !college, !keyboard, !switches, !totalTime, !art, !compiler, !build, !render, !learning, !lib, !software, !tablet, !script, !quotelist, !rules, !userlist, !never, !design, !manifesto, !ask");
					}
					if (!strcmp(text, "time"))
					{
					}
					if (!strcmp(text, "today"))
					{
					}
					if (!strcmp(text, "scehdule"))
					{
					}
					if (!strcmp(text, "now"))
					{
					}
					if (!strcmp(text, "site"))
					{
						SendMessage(Connection, "#effect0r", "HH Website: http://goo.gl/fmjocD :: HH Forums: http://goo.gl/NuArvD");
					}
					if (!strcmp(text, "old"))
					{
					}
					if (!strcmp(text, "wrist"))
					{
					}
					if (!strcmp(text, "who"))
					{
					}
					if (!strcmp(text, "buy"))
					{
					}
					if (!strcmp(text, "game"))
					{
					}
					if (!strcmp(text, "stream"))
					{
					}
					if (!strcmp(text, "lang"))
					{
					}
					if (!strcmp(text, "ide"))
					{
					}
					if (!strcmp(text, "editoradvice"))
					{
					}
					if (!strcmp(text, "college"))
					{
					}
					if (!strcmp(text, "keyboard"))
					{
					}
					if (!strcmp(text, "switches"))
					{
					}
					if (!strcmp(text, "totaltime"))
					{
					}
					if (!strcmp(text, "art"))
					{
					}
					if (!strcmp(text, "compiler"))
					{
					}
					if (!strcmp(text, "build"))
					{
					}
					if (!strcmp(text, "render"))
					{
					}
					if (!strcmp(text, "learning"))
					{
					}
					if (!strcmp(text, "lib"))
					{
					}
					if (!strcmp(text, "software"))
					{
					}
					if (!strcmp(text, "tablet"))
					{
					}
					if (!strcmp(text, "script"))
					{
					}
					if (!strcmp(text, "quotelist"))
					{
					}
					if (!strcmp(text, "rules"))
					{
					}
					if (!strcmp(text, "userlist"))
					{
					}
					if (!strcmp(text, "never"))
					{
					}
					if (!strcmp(text, "design"))
					{
					}
					if (!strcmp(text, "manifesto"))
					{
					}
					if (!strcmp(text, "ask"))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
					}
					if (!strcmp(text, ""))
					{
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
	if ((strlen(Connection->Port) > 0) && (strlen(Connection->Port) < 0xFFFF))
	{
		Connection->Status = getaddrinfo(Connection->Server, Connection->Port, 
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

				fprintf(Connection->OutStream, "PASS %s\r\n", Connection->Pass);
				fprintf(Connection->OutStream, "NICK %s\r\n", Connection->Nick);
				fprintf(Connection->OutStream, "USER %s * 0 :%s\r\n", Connection->User, Connection->Nick);
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
