/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice: $
   ==========================================================================*/

#include "irc_connection.h"

	void
JoinChannel(irc_connection *Connection, char *Channel)
{
	if (Connection->IsConnected)
	{
		fprintf(Connection->OutStream, "JOIN %s\r\n", Channel);
		fflush(Connection->OutStream);
	}
}

	void
SendMessage(irc_connection *Connection, char *Channel, char *Message)
{
	if (Connection->IsConnected)
	{
		fprintf(Connection->OutStream, "PRIVMSG %s :%s\r\n", Channel, Message);
		fflush(Connection->OutStream);
	}
}

	void
CloseConnection(irc_connection *Connection)
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
	void
ParseMessage(irc_connection *Connection, char *Message)
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
					if (!strcmp(text, "join"))
					{
						JoinChannel(Connection, Param);
					}
					if (!strcmp(text, "help"))
					{
						SendMessage(Connection,	"#effect0r",
									"Greetings mortal, I am an IRC bot written in (the good parts) of c++! I was written by Cory Henderlite (effect0r) from Handmade.dev. Available commands: !help");
					}
				}
			}
			printf("%s\n", text);
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

int Connect(irc_connection *Connection, char *Server, char *Nick, char *User,
		char *Name, char *Pass, char *Port)
{
	int ConnError;
	ZERO(&Connection->Hints, Connection->Hints);
	Connection->Hints.ai_family = AF_UNSPEC;
	Connection->Hints.ai_socktype = SOCK_STREAM;
	if ((strlen(Port) > 0) && (strlen(Port) < 0xFFFF))
	{
		Connection->Status = getaddrinfo(Server, Port, &Connection->Hints,
				&Connection->ServerInfo);
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
				Connection->Port = STRINGALLOC(Port);
				strcpy(Connection->Port, Port);

				Connection->Nick = STRINGALLOC(Nick);
				strcpy(Connection->Nick, Nick);

				Connection->Server = STRINGALLOC(Server);
				strcpy(Connection->Server, Server);

				Connection->User = STRINGALLOC(User);
				strcpy(Connection->User, User);

				if (Pass)
				{
					Connection->Pass = STRINGALLOC(Pass);
					strcpy(Connection->Pass, Pass);
				}

				Connection->IsConnected = 1;

				fprintf(Connection->OutStream, "PASS %s\r\n", Pass);
				fprintf(Connection->OutStream, "NICK %s\r\n", Nick);
				fprintf(Connection->OutStream, "USER %s * 0 :%s\r\n", User,
						Name);
				//fprintf(Connection->OutStream, "JOIN %s\r\n", "#effect0r");
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
