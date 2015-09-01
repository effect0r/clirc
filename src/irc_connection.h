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

struct quote_list
{
	unsigned int TotalQuotes;
	sqlite3 *QuoteDB; 
};

struct config_info
{
	int ChannelCount;
	char **ChannelList;
	char CommandPrefix;
	char *Admin;
	int WhiteListCount;
	char **WhiteList;
	char *Port;
	char *Nick;
	char *Server;
	char *User;
	char *Pass;
	map *FaqCommandsMap;
	quote_list QuoteList;
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
