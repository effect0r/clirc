#if !defined(IRC_CONNECTION_H)
/* ========================================================================
 $File: $
 $Date: $
 $Revision: $
 $Creator: Cory Henderlite $
 $Notice:  $
 ======================================================================== */

#define IRC_CONNECTION_H

enum user_modes
{
    IRC_USER_MODE_OP = (1 << 0),
    IRC_USER_MODE_VOICE = (1 << 1),
    IRC_USER_MODE_HALFOP = (1 << 2),
    IRC_USER_MODE_DEAF = (1 << 3),
    IRC_USER_MODE_OPERATOR = (1 << 4)
};

enum channel_modes
{
    IRC_CHANNEL_MODE_COLORS = (1 << 0),
    IRC_CHANNEL_MODE_NOEXTERNALMESSAGES = (1 << 1),
    IRC_CHANNEL_MODE_REGISTEREDONLY = (1 << 2),
    IRC_CHANNEL_MODE_TOPICPROTECTION = (1 << 3),
    IRC_CHANNEL_MODE_INVITEONLY = (1 << 4),
    IRC_CHANNEL_MODE_MODERATED = (1 << 5),
    IRC_CHANNEL_MODE_LOCKED = (1 << 6),
    IRC_CHANNEL_MODE_LIMIT = (1 << 7)
};

typedef struct ban_item
{
    char *Setter;
    char *Host;
    long  Time;
    ban_item *Next;    
} ban_item;

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

typedef struct channel_user
{
    char *Nick;
    long Modes;
    char *Host;
    char *Ident;
    channel_user *Next;
} channel_user;

typedef struct channel
{
    char *Topic;
    long TopicTime;
    long CreationTime;
    char *TopicSetByName;
    char *Password;
    char *Name;
    //NOTE: THIS CAN BE TUNED.
    long Modes;
    long Limit;
    channel_user *Users;
    ban_item *Bans;
    channel *Next;
} channel;

typedef struct irc_connection
{
    int IsConnected;
    channel *ChannelList;
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
