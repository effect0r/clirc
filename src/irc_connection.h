#if !defined(IRC_CONNECTION_H)
/* ========================================================================
 $File: $
 $Date: $
 $Revision: $
 $Creator: Cory Henderlite $
 $Notice:  $
 ======================================================================== */

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

struct irc_command_hook
{
    char *IrcCommand;
    int (*function)(char *, server_data *, void *);
    irc_command_hook *Next;
};

struct config_info
{
	int ChannelCount;
	char **ChannelList;
	char CommandPrefix;
};

struct irc_connection
{
    int IsConnected;
	config_info ConfigInfo;
    char *Port;
    char *Nick;
    char *Server;
    char *User;
    char *Pass;
	map *Map;
    //NOTE: connection info
    struct addrinfo Hints, *ServerInfo;
    int Status;
    int Socket;
    FILE *OutStream;
};

#endif
