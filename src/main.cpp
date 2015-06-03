/* ========================================================================
 $File: $
 $Date: $
 $Revision: $
 $Creator: Cory Henderlite $
 $Notice: $
 ======================================================================== */
#include "main.h"
#include "irc_connection.cpp"

#define SERVER "ic.globalgamers.net"
#define NICK "effect0r`Services"
#define USER "effect0r`Services"
#define NAME "effect0r`Services"
#define PASSWORD ""
#define PORT "4444"

void *
ReadInput(void *Connection)
{
    char buffer[1024];
    char *Store;
    char *Command;
    char *Parameters;

    char *CurrentChannel;
    irc_connection *Conn = (irc_connection*) Connection;

    while (1)
    {
        fflush (stdin);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strlen(buffer) - 1] = '\0';
        // TODO: DEBUG THIS LOGIC!
        Command = buffer;
        if (Command[0] == '/')
        {
            Store = strchr(Command, ' ');

            if (!Store)
            {
                if (!strcmp(Command, "/help"))
                {
                    printf("Help will be here. Soon. Maybe.\n");
                }
                else if (!strcmp(Command, "/list"))
                {
                    channel *ChannelList = (channel*) Conn->ChannelList;
                    if (ChannelList)
                    {
                        printf("Channels: ");
                        while (ChannelList->Name)
                        {
                            if (!strcmp(ChannelList->Name, CurrentChannel))
                            {
                                printf("*%s* ", ChannelList->Name);
                            }
                            else
                            {
                                printf("%s ", ChannelList->Name);
                            }
                            if (!ChannelList->Next)
                            {
                                break;
                            }
                            ChannelList = ChannelList->Next;
                        }
                        printf("\n");
                    }
                    else
                    {
                        printf("No active channels\n");
                    }
                }
                else if (!strcmp(Command, "/users"))
                {
                    channel *ChannelList = (channel*) Conn->ChannelList;
                    while (ChannelList)
                    {
                        if (!strcmp(ChannelList->Name, CurrentChannel))
                        {
                            channel_user *User = ChannelList->Users;
                            if (User)
                            {
                                printf("Users for %s: ", CurrentChannel);
                                int i = 0;
                                while (User)
                                {
                                    printf("%s ", User->Nick);
                                    User = User->Next;
                                    i++;
                                }
                                printf("(%d total).\n", i);
                                break;
                            }
                        }
                        else
                        {
                            ChannelList = ChannelList->Next;
                        }
                    }
                    if (!ChannelList)
                    {
                        printf("You're not on any channels.\n");
                    }

                }
                // /part, /quit, etc
                else if (!strcmp(Command, "/quit"))
                {
                    printf("[DEBUG]: Bye!\n");
                    break;
                }
                else if (!strcmp(Command, "/chan"))
                {
                    printf("Current channel: %s\n", CurrentChannel);
                }
            }
            else
            {
                *Store = '\0';
                Parameters = Store + 1;
                if (!Parameters)
                {
                    //no Parameters, do nothing
                    continue;
                }
                else
                {
                    if (!strcmp(Command, "/join"))
                    {
                        if (Parameters[0] == '#')
                        {
                            JoinChannel(Conn, Parameters);
                            if (CurrentChannel)
                            {
                                free(CurrentChannel);
                            }
                            CurrentChannel = (char*) malloc(
                                    strlen(Parameters) * sizeof(char));
                            strcpy(CurrentChannel, Parameters);
                        }
                    }
                    else if (!strcmp(Command, "/switch"))
                    {
                        if (Parameters[0] == '#')
                        {
                            if (CurrentChannel)
                            {
                                free(CurrentChannel);
                            }
                            CurrentChannel = (char*) malloc(
                                    strlen(Parameters) * sizeof(char));
                            strcpy(CurrentChannel, Parameters);

                            printf("Now talking in %s.\n", CurrentChannel);
                        }
                    }
                    else if (!strcmp(Command, "/command"))
                    {
                        if (Parameters)
                        {
                            SendCommand(Conn, Parameters);
                        }
                    }
                }
            }
        }
        else
        {
            if (CurrentChannel)
            {
                SendMessage(Conn, CurrentChannel, buffer);
            }
            else
            {
                printf("Join a channel first!\n");
            }
        }
    }

    CloseConnection(Conn);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    irc_connection Conn = { 0 };

    pthread_t ListenThread;
    Connect(&Conn, SERVER, NICK, USER, NAME, PASSWORD, PORT);
    pthread_create(&ListenThread, 0, ReadInput, (void*) &Conn);

    MessageLoop(&Conn);
    pthread_join(ListenThread, 0);

    Conn = { 0 };
    return (0);
}
