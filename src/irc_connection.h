#if !defined(IRC_CONNECTION_H)
/* ========================================================================
 $File: $
 $Date: $
 $Revision: $
 $Creator: Cory Henderlite $
 $Notice:  $
 ======================================================================== */

#define IRC_CONNECTION_H

typedef struct server_data
{
    char *nick;
    char *ident;
    char *host;
    char *target;
} server_data;

typedef struct irc_command_hook
{
    char *IrcCommand;
    int (*function)(char *, server_data *, void *);
    irc_command_hook *Next;
} irc_command_hook;

typedef struct irc_connection
{
    int IsConnected;
    char *Port;
    char *Nick;
    char *Server;
    char *User;
    char *Pass;
    //NOTE: connection info
    struct addrinfo Hints, *ServerInfo;
    int Status;
    int Socket;
    FILE *OutStream;
} irc_connection;

#endif
