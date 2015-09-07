#if !defined(IRC_CONNECTION_H)
#define IRC_CONNECTION_H

struct pair
{
	char *Key;
	char *Value;
};

struct map
{
	unsigned int Size;
	unsigned int Used;
	pair *Pairs;
};

struct server_data
{
	char *nick;
	char *ident;
	char *host;
	char *target;
};

struct config_info
{
	// NOTE(cory): This stuff is within a separate section of the config file, denoted by a channel name. Support for 256 channels currently. May change.
	int AdminCount;
	char *Admins[256];
	char CommandPrefix;
	sqlite3 *Database;

	// NOTE(cory): This stuff is within the core section
	char *Port;
	char *Nick;
	char *Server;
	char *User;
	char *Pass;
};

struct irc_connection
{
	config_info ConfigInfo;

	//NOTE: connection info
	int IsConnected;
	struct addrinfo Hints, *ServerInfo;
	int Status;
	int Socket;
	FILE *OutStream;
};

#endif
