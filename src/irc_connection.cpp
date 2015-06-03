/* ========================================================================
 $File: $
 $Date: $
 $Revision: $
 $Creator: Cory Henderlite $
 $Notice: $
 ==========================================================================*/

#include "irc_connection.h"

void
FreeChannelBans(ban_item *Ban)
{
    ban_item *Temp;

    while(Ban)
    {
        free(Ban->Setter);
        free(Ban->Host);

        Temp = Ban;
        Ban = Ban->Next;
        free(Temp);
    }
}

void
FreeChannelUsers(channel_user *Users)
{
    channel_user *Temp;

    while (Users)
    {
        free(Users->Nick);
        free(Users->Ident);
        free(Users->Host);

        Temp = Users;
        Users = Users->Next;
        free(Temp);
    }
}

void
FreeChannelList(channel *Channel)
{
    channel *Temp;
    while (Channel)
    {
        free(Channel->Topic);
        free(Channel->Password);
        free(Channel->Name);

        FreeChannelBans(Channel->Bans);
        free(Channel->Bans);
        FreeChannelUsers(Channel->Users);
        free(Channel->Users);

        Temp = Channel;
        Channel = Channel->Next;
        free(Temp);
    }
}

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
SendMessage(irc_connection *Connection, char *Channel, char *Command)
{
    if (Connection->IsConnected)
    {
        fprintf(Connection->OutStream, "PRIVMSG %s :%s\r\n", Channel, Command);
        fflush(Connection->OutStream);
    }
}

void
SendCommand(irc_connection *Connection, char *Command)
{
    if (Connection->IsConnected)
    {
        fprintf(Connection->OutStream, "%s\r\n", Command);
        fflush(Connection->OutStream);
    }
}

void
SendCommand(irc_connection *Connection, char *Command, char *Channel)
{
    if (Connection->IsConnected)
    {
        fprintf(Connection->OutStream, "%s %s\r\n", Command, Channel);
        fflush(Connection->OutStream);
    }
}

void
SendMode(irc_connection *Connection, char *Command, char *Channel)
{
    if (Connection->IsConnected)
    {
        fprintf(Connection->OutStream, "MODE %s %s\r\n", Channel, Command);
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

    FreeChannelList(Connection->ChannelList);
}

void
ParseMessage(irc_connection *Connection, char *Message)
{
    long Time;
    char *Command;
    char *ChanName;
    char *Channel;
    char *Data;
    char *HostName;
    char *NewNick;
    char *Parameters;
    char *Position;
    char *Reason;
    char *Recipient;
    char *Setter;
    char *TimeStr;
    char *Topic;

    server_data From;

    channel *ChanList;

    channel_user *UserList;
    channel_user *User;
    struct timeval TimeVal;

    gettimeofday(&TimeVal, 0);

    From.target = 0;

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
        From.nick = HostName;
        From.ident = strchr(HostName, '!');
        if (From.ident)
        {
            *From.ident = '\0';
            From.ident++;
            From.host = strchr(From.ident, '@');
            if (From.host)
            {
                *From.host = '\0';
                From.host++;
            }
        }
        // TODO: Bulletproof this!
        if (!strcmp(Command, "JOIN"))
        {
            ChanName = Parameters;
            if (ChanName[0] == ':')
            {
                *ChanName = '\0';
                ChanName++;
            }                

            if (!strcmp(From.nick, Connection->Nick))
            {
                //NOTE: you're joining a channel. It can't exist.
                ChanList = Connection->ChannelList;
                if (ChanList)
                {
                    while (ChanList->Name)
                    {
                        if (!ChanList->Next)
                        {
                            ChanList->Next = CHANNELALLOC;
                            ZERO(ChanList->Next, channel);

                            ChanList->Next->Name = STRINGALLOC(ChanName);
                            strcpy(ChanList->Next->Name, ChanName);

                            break;

                        }
                        ChanList = ChanList->Next;
                    }
                }
                else
                {
                    Connection->ChannelList = CHANNELALLOC;
                    ZERO(Connection->ChannelList, channel);

                    Connection->ChannelList->Name = STRINGALLOC(ChanName);
                    strcpy(Connection->ChannelList->Name, ChanName);
                }
                SendCommand(Connection, "WHO", ChanName);

            }
            else
            { //NOTE: Not you joining channel, so channel is assumed to be in the list.
                ChanList = Connection->ChannelList;

                while (ChanList)
                {
                    if (!strcmp(ChanList->Name, ChanName))
                    {
                        UserList = ChanList->Users;
                        while (UserList->Nick)
                        {
                            if (!UserList->Next)
                            {
                                UserList->Next = USERALLOC;
                                ZERO(UserList->Next, channel_user);
                            }
                            UserList = UserList->Next;
                        }

                        UserList->Nick = STRINGALLOC(From.nick);
                        strcpy(UserList->Nick, From.nick);

                        UserList->Host = STRINGALLOC(From.host);
                        strcpy(UserList->Host, From.host);

                        UserList->Ident = STRINGALLOC(From.ident);
                        strcpy(UserList->Ident, From.ident);
                        break;
                    }
                    ChanList = ChanList->Next;
                }

            }
        }
        else if (!strcmp(Command, "PART"))
        {
            if (!Parameters)
            {
                return;
            }

            ChanName = Parameters;

            Reason = strchr(ChanName, ' ');
            *Reason = '\0';
            Reason += 2;

            ChanList = Connection->ChannelList;
            while (ChanList)
            {
                if (!strcmp(ChanList->Name, ChanName))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            channel_user *PreviousUser;
            channel_user *UserToDelete = ChanList->Users;
            while (UserToDelete)
            {
                if (!strcmp(UserToDelete->Nick, From.nick))
                {
                    break;
                }
                PreviousUser = UserToDelete;
                UserToDelete = UserToDelete->Next;
            }

            if (!PreviousUser)
            {
                //NOTE: Head of list.
                ChanList->Users = UserToDelete->Next;
                free(UserToDelete);
            }
            else
            {
                PreviousUser->Next = UserToDelete->Next;
                free(UserToDelete);
            }
        }
        // NOTE:TODO: Maybe a bug here. Found user by inespection, however, it's throwing a SIGSEGV
        // because reasons?
        else if (!strcmp(Command, "QUIT"))
        {
            int i = 0;
            NewNick = From.nick;
            Reason = strchr(Parameters, ':');
            *Reason = '\0';
            Reason++;

            ChanList = Connection->ChannelList;
            while (ChanList)
            {
                channel_user *PreviousUser;
                channel_user *UserToDelete = ChanList->Users;
                while (UserToDelete)
                {
                    if (!strcmp(UserToDelete->Nick, From.nick))
                    {
                        break;
                    }
                    PreviousUser = UserToDelete;
                    UserToDelete = UserToDelete->Next;
                }

                if (!PreviousUser)
                {
                    //NOTE: Head of list.
                    ChanList->Users = UserToDelete->Next;
                    free(UserToDelete);
                }
                else if (!UserToDelete)
                {
                    //NOTE: End of list. How is this even possible?
                }
                else
                {
                    PreviousUser->Next = UserToDelete->Next;
                    free(UserToDelete);
                }
                ChanList = ChanList->Next;
            }
            int sdfe = 0;
        }
        // TODO: Test _anything_ with ban_list in it!!!
        else if (!strcmp(Command, "MODE"))
        {
            // NOTE: Modes come in as the identifier -> then the i-th in errataarray, where i is
            // the first element that has errata. IE: +nmtlo 15 effect0r would parse
            //      + n m t l=>15 o=>effect0r.
            // With mixed modes +o-b effect0r *!*@*, they are parsed the same. It would parse like
            //      + o=>effect0r - b=>*!*@*.
            // This algorithm parses from left to right in the above manner.
            // It could use more bulletproofing, but it worked in my tests.
            char *ModeChanges;
            char *Errata;
            char *ErrataElements[10];
            int ArrayElement = 0;
            bool IsPlus = 0; // 0 for minus, 1 for plus

            // NOTE: to prevent stack corruption.
            for (int i = 0; i < ARRAYCOUNT(ErrataElements); ++i)
            {
                ErrataElements[i] = 0;
            }

            Channel = Parameters;

            ModeChanges = strchr(Channel, ' ');
            if (!ModeChanges)
            {
                return;
            }
            *ModeChanges = '\0';
            ModeChanges++;

            // NOTE: extra stuff (bans, key, limit)
            Errata = strchr(ModeChanges, ' ');
            if (Errata)
            {
                *Errata = '\0';
                Errata++;
                // NOTE: String parsing!
                char *ErrataLoop = strchr(Errata, ' ');
                ErrataElements[0] = Errata;
                int i = 0;
                while (ErrataLoop)
                {
                    ErrataLoop++;
                    ErrataElements[++i] = ErrataLoop;
                    ErrataLoop = strchr(ErrataLoop, ' ');
                }
                i = 0;
                while (ErrataElements[i])
                {
                    char *Temp = strchr(ErrataElements[i++], ' ');
                    if (Temp)
                    {
                        *Temp = '\0';
                    }
                }
            }

            ChanList = Connection->ChannelList;
            while (ChanList)
            {
                if (!strcmp(ChanList->Name, Channel))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            for (unsigned int i = 0; i < strlen(ModeChanges); ++i)
            {
                switch (ModeChanges[i])
                {
                    case '-':
                    {
                        IsPlus = false;
                    }break;
                    case '+':
                    {
                        IsPlus = true;
                    }break;
                    // NOTE: ban
                    case 'b':
                    {
                        if (IsPlus)
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            if (!ChanList->Bans)
                            {
                                ChanList->Bans = BANALLOC;
                                ZERO(ChanList->Bans, ban_item);
                            }

                            ban_item *BanList = ChanList->Bans;
                            while(BanList->Setter)
                            {
                                if (!BanList->Next)
                                {
                                    BanList->Next = BANALLOC;
                                    ZERO(BanList->Next, ban_item);
                                }
                                BanList = BanList->Next;
                            }

                            BanList->Setter = STRINGALLOC(From.nick);
                            strcpy(BanList->Setter, From.nick);

                            BanList->Host = STRINGALLOC(Temp);
                            strcpy(BanList->Host, Temp);

                            BanList->Time = TimeVal.tv_sec; // NOTE: maybe?
                        }
                        else
                        {
                            ban_item *BanList = ChanList->Bans;
                            ban_item *Temp = ChanList->Bans;
                            while(BanList->Setter)
                            {
                                Temp = BanList;
                                if (!strcmp(BanList->Host, ErrataElements[ArrayElement]))
                                {
                                    break;
                                }
                                BanList = BanList->Next;
                            }
                            Temp->Next = BanList->Next;
                            free(BanList);
                            ArrayElement++;
                        }
                    }break;
                    // NOTE: limit
                    case 'l':
                    {
                        if (IsPlus)
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            ChanList->Limit = atol(Temp);
                            ChanList->Modes |= IRC_CHANNEL_MODE_LIMIT;
                        }
                        else
                        {
                            ChanList->Limit = 0;
                            ChanList->Modes ^= IRC_CHANNEL_MODE_LIMIT;
                        }
                    }break;
                    // NOTE: key
                    case 'k':
                    {
                        if (IsPlus)
                        {
                            // NOTE: should i hash this? *shrug*
                            char *Temp = ErrataElements[ArrayElement++];
                            ChanList->Password = STRINGALLOC(Temp);
                            strcpy(ChanList->Password, Temp);
                            ChanList->Modes |= IRC_CHANNEL_MODE_LOCKED;
                        }
                        else
                        {
                            free(ChanList->Password);
                            ChanList->Modes ^= IRC_CHANNEL_MODE_LOCKED;
                            ArrayElement++;
                        }
                    }break;
                    // NOTE: op
                    case 'o':
                    {
                        if (IsPlus)
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            UserList = ChanList->Users;
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, Temp))
                                {
                                    break;
                                }
                                UserList = UserList->Next;
                            }
                            UserList->Modes |= IRC_USER_MODE_OP;
                        }
                        else
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            UserList = ChanList->Users;
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, Temp))
                                {
                                    break;
                                }
                                UserList = UserList->Next;
                            }
                            UserList->Modes ^= IRC_USER_MODE_OP;
                        }
                    }break;
                    // NOTE: voice
                    case 'v':
                    {
                        if (IsPlus)
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            UserList = ChanList->Users;
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, Temp))
                                {
                                    break;
                                }
                                UserList = UserList->Next;
                            }

                            UserList->Modes |= IRC_USER_MODE_VOICE;
                        }
                        else
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            UserList = ChanList->Users;
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, Temp))
                                {
                                    break;
                                }
                                UserList = UserList->Next;
                            }
                            UserList->Modes ^= IRC_USER_MODE_VOICE;
                        }

                    }break;
                    // NOTE: halfop
                    case 'h':
                    {
                        if (IsPlus)
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            UserList = ChanList->Users;
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, Temp))
                                {
                                    break;
                                }
                                UserList = UserList->Next;
                            }

                            UserList->Modes |= IRC_USER_MODE_HALFOP;
                        }
                        else
                        {
                            char *Temp = ErrataElements[ArrayElement++];
                            UserList = ChanList->Users;
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, Temp))
                                {
                                    break;
                                }
                                UserList = UserList->Next;
                            }
                            UserList->Modes ^= IRC_USER_MODE_HALFOP;
                        }
                    }break;
                    // NOTE: invite only
                    case 'i':
                    {
                        if (IsPlus)
                        {
                            ChanList->Modes |= IRC_CHANNEL_MODE_INVITEONLY;
                        }
                        else
                        {
                            ChanList->Modes ^= IRC_CHANNEL_MODE_INVITEONLY;
                        }
                    }break;
                    // NOTE: topic protection
                    case 't':
                    {
                        if (IsPlus)
                        {
                            ChanList->Modes |= IRC_CHANNEL_MODE_TOPICPROTECTION;
                        }
                        else
                        {
                            ChanList->Modes ^= IRC_CHANNEL_MODE_TOPICPROTECTION;
                        }
                    }break;
                    // NOTE: no colors
                    case 'c':
                    {
                        if (IsPlus)
                        {
                            ChanList->Modes |= IRC_CHANNEL_MODE_COLORS;
                        }
                        else
                        {
                            ChanList->Modes ^= IRC_CHANNEL_MODE_COLORS;
                        }
                    }break;
                    // NOTE: moderated
                    case 'm':
                    {
                        if (IsPlus)
                        {
                            ChanList->Modes |= IRC_CHANNEL_MODE_MODERATED;
                        }
                        else
                        {
                            ChanList->Modes ^= IRC_CHANNEL_MODE_MODERATED;
                        }
                    }break;
                    // NOTE: no external messages
                    case 'n':
                    {
                        if (IsPlus)
                        {
                            ChanList->Modes |= IRC_CHANNEL_MODE_NOEXTERNALMESSAGES;
                        }
                        else
                        {
                            ChanList->Modes ^= IRC_CHANNEL_MODE_NOEXTERNALMESSAGES;
                        }
                    }break;
                    // NOTE: registered only
                    case 'r':
                    {
                        if (IsPlus)
                        {
                            ChanList->Modes |= IRC_CHANNEL_MODE_REGISTEREDONLY;
                        }
                        else
                        {
                            ChanList->Modes ^= IRC_CHANNEL_MODE_REGISTEREDONLY;
                        }
                    }break;
                    default:
                    {
                        printf("Unhandled mode: %c\n", ModeChanges[i]);
                    }break;
                }
            }
        }
        // NOTE: PRIVMSG for building IAL.
        else if (!strcmp(Command, "PRIVMSG"))
        {
            int privmsg = 0;
        }
        // NOTE: Avalable modes etc
        else if (!strcmp(Command, "005"))
        {

        }
        // NOTE: Channel modes list
        else if (!strcmp(Command, "324"))
        {
            char *Nick;
            char *Channel;
            char *Modes;
            char *Errata;
            char *ErrataElements[10];

            Nick = Parameters;

            if (!Nick)
            {
                return;
            }
            Channel = strchr(Nick, ' ');
            *Channel = '\0';
            Channel++;

            Modes = strchr(Channel, ' ');
            *Modes = '\0';
            Modes++;

            Errata = strchr(Modes, ' ');
            if (Errata)
            {
                *Errata = '\0';
                Errata++;
                // NOTE: String parsing!
                char *ErrataLoop = strchr(Errata, ' ');
                ErrataElements[0] = Errata;
                int i = 0;
                while (ErrataLoop)
                {
                    ErrataLoop++;
                    ErrataElements[++i] = ErrataLoop;
                    ErrataLoop = strchr(ErrataLoop, ' ');
                }
                i = 0;
                while (ErrataElements[i])
                {
                    char *Temp = strchr(ErrataElements[i++], ' ');
                    if (Temp)
                    {
                        *Temp = '\0';
                    }
                }
            }

            ChanList = Connection->ChannelList;
            while (ChanList)
            {
                if (!strcmp(ChanList->Name, Channel))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            // modes are always + here!
            int ArrayElement = 0;
            for (int i = 0; i < strlen(Modes); i++)
            {
                switch (Modes[i])
                {
                    // NOTE: limit
                    case 'l':
                    {
                        char *Temp = ErrataElements[ArrayElement++];
                        ChanList->Limit = atol(Temp);
                        ChanList->Modes |= IRC_CHANNEL_MODE_LIMIT;
                    }break;
                    // NOTE: key
                    case 'k':
                    {
                        // NOTE: should i hash this? *shrug*
                        char *Temp = ErrataElements[ArrayElement++];
                        ChanList->Password = STRINGALLOC(Temp);
                        strcpy(ChanList->Password, Temp);
                        ChanList->Modes |= IRC_CHANNEL_MODE_LOCKED;
                    }break;
                    // NOTE: invite only
                    case 'i':
                    {
                        ChanList->Modes |= IRC_CHANNEL_MODE_INVITEONLY;
                    }break;
                    // NOTE: topic protection
                    case 't':
                    {
                        ChanList->Modes |= IRC_CHANNEL_MODE_TOPICPROTECTION;
                    }break;
                    // NOTE: no colors
                    case 'c':
                    {
                        ChanList->Modes |= IRC_CHANNEL_MODE_COLORS;
                    }break;
                    // NOTE: moderated
                    case 'm':
                    {
                        ChanList->Modes |= IRC_CHANNEL_MODE_MODERATED;
                    }break;
                    // NOTE: no external messages
                    case 'n':
                    {
                        ChanList->Modes |= IRC_CHANNEL_MODE_NOEXTERNALMESSAGES;
                    }break;
                    // NOTE: registered only
                    case 'r':
                    {
                        ChanList->Modes |= IRC_CHANNEL_MODE_REGISTEREDONLY;
                    }break;
                    default:
                    {
                        printf("Unhandled mode: %c\n", Modes[i]);
                    }break;
                }
            }
        }

        // NOTE: Channel creation date
        else if (!strcmp(Command, "329"))
        {
            int skljfec = 0;
            ChanName = strchr(Parameters, ' ');
            *ChanName = '\0';
            ChanName++;

            TimeStr = strchr(ChanName, ' ');
            *TimeStr = '\0';
            TimeStr++;

            Time = atol(TimeStr);

            ChanList = Connection->ChannelList;
            while(ChanList)
            {
                if (!strcmp(ChanList->Name, ChanName))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            ChanList->CreationTime = Time;
        }
        // NOTE: Channel topic.
        else if (!strcmp(Command, "332"))
        {
            if (!Parameters)
            {
                return;
            }

            Recipient = Parameters;

            Channel = strchr(Recipient, ' ');
            *Channel = '\0';
            Channel++;

            Topic = strchr(Channel, ' ');
            *Topic = '\0';
            Topic += 2;

            ChanList = Connection->ChannelList;
            while (ChanList)
            {
                if (!strcmp(ChanList->Name, Channel))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            ChanList->Topic = STRINGALLOC(Topic);
            strcpy(ChanList->Topic, Topic);
        }
        // NOTE: Channel topic time and setter.
        else if (!strcmp(Command, "333"))
        {
            Recipient = Parameters;

            Channel = strchr(Recipient, ' ');
            *Channel = '\0';
            Channel++;

            Setter = strchr(Channel, ' ');
            *Setter = '\0';
            Setter++;

            TimeStr = strchr(Setter, ' ');
            *TimeStr = '\0';
            TimeStr++;

            Time = atol(TimeStr);

            ChanList = Connection->ChannelList;
            while (ChanList)
            {
                if (!strcmp(ChanList->Name, Channel))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }
            ChanList->TopicTime = Time;
            ChanList->TopicSetByName = STRINGALLOC(Setter);
            strcpy(ChanList->TopicSetByName, Setter);

        }
        // NOTE: Channel who list reply
        else if(!strcmp(Command, "352"))
        {            
            // "effect0r`cpp #scriptij effect0r`c GlobalGamers-A0915A2C.triad.res.rr.com
            // *.globalgamers.net effect0r`cpp H :0 effect0r`cpp"
            int whom = 0;
            server_data WhoReply = { 0 };
            Recipient = Parameters;

            ChanName = strchr(Parameters, ' ');
            *ChanName = '\0';
            ChanName++;

            WhoReply.ident = strchr(ChanName, ' ');
            *WhoReply.ident = '\0';
            WhoReply.ident++;

            WhoReply.host = strchr(WhoReply.ident, ' ');
            *WhoReply.host = '\0';
            WhoReply.host++;

            char *Server = strchr(WhoReply.host, ' ');
            *Server = '\0';
            Server++;

            WhoReply.nick = strchr(Server, ' ');
            *WhoReply.nick = '\0';
            WhoReply.nick++;

            char *Flags = strchr(WhoReply.nick, ' ');
            *Flags = '\0';
            Flags++;

            char *Hops = strchr(Flags, ' ');
            *Hops = '\0';
            Hops++;            

            ChanList = Connection->ChannelList;
            while(ChanList)
            {
                if (!strcmp(ChanList->Name, ChanName))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            UserList = ChanList->Users;
            while(UserList)
            {
                if (!strcmp(UserList->Nick, WhoReply.nick))
                {
                    break;
                }
                UserList = UserList->Next;
            }

            UserList->Host = STRINGALLOC(WhoReply.host);
            strcpy(UserList->Host, WhoReply.host);

            UserList->Ident = STRINGALLOC(WhoReply.ident);
            strcpy(UserList->Ident, WhoReply.ident);

            if (strlen(Flags) > 1)
            {
                //[H|G][*| ][@|+| ][x]
                for (int z = 0;
                     z < strlen(Flags);
                     ++z)
                {
                    if (Flags[z] == 'H' ||
                        Flags[z] == 'G')
                    {
                        continue;
                    }
                    else
                    {
                        switch(Flags[z])
                        {
                            case '@':
                            {
                                UserList->Modes |= IRC_USER_MODE_OP;
                            }break;
                            case '+':
                            {
                                UserList->Modes |= IRC_USER_MODE_VOICE;
                            }break;
                            case '*':
                            {
                                UserList->Modes |= IRC_USER_MODE_OPERATOR;
                            }break;
                            case 'x':
                            {
                                UserList->Modes |= IRC_USER_MODE_DEAF;
                            }break;                            
                        }
                    }
                }
            }
            
            int who = 0;
            
        }
        // NOTE: Channel names list.
        else if (!strcmp(Command, "353"))
        {
            ChanName = strchr(Parameters, '#');

            Data = strchr(ChanName, ' ');
            *Data = '\0';
            Data++;

            ChanList = Connection->ChannelList;

            while (ChanList)
            {
                if (!strcmp(ChanList->Name, ChanName))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            if (!ChanList->Users)
            {
                ChanList->Users = USERALLOC;
                ZERO(ChanList->Users, channel_user);
            }

            Position = strchr(Data, ':');
            if (Position)
            {
                *Position = '\0';
                Position++;
                while (strchr(Position, ' '))
                {
                    char *Temp;
                    Temp = strchr(Position, ' ');
                    *Temp = '\0';
                    Temp++;
                    User = ChanList->Users;
                    while (User->Nick)
                    {
                        if (!User->Next)
                        {
                            User->Next = USERALLOC;
                            ZERO(User->Next, channel_user);
                        }
                        User = User->Next;
                    }

                    if (Position[0] == '@')
                    {
                        User->Modes |= IRC_USER_MODE_OP;
                        Position++;
                    }
                    else if (Position[0] == '+')
                    {
                        User->Modes |= IRC_USER_MODE_VOICE;
                        Position++;
                    }
                    else if (Position[0] == '%')
                    {
                        User->Modes |= IRC_USER_MODE_HALFOP;
                        Position++;
                    }

                    User->Nick = STRINGALLOC(Position);
                    strcpy(User->Nick, Position);

                    Position = Temp;
                }

                User = ChanList->Users;
                while (User->Nick)
                {
                    if (!User->Next)
                    {
                        User->Next = USERALLOC;
                        ZERO(User->Next, channel_user);
                    }
                    User = User->Next;
                }

                if (Position[0] == '@')
                {
                    User->Modes |= IRC_USER_MODE_OP;
                    Position++;
                }
                else if (Position[0] == '+')
                {
                    User->Modes |= IRC_USER_MODE_VOICE;
                    Position++;
                }
                else if (Position[0] == '%')
                {
                    User->Modes |= IRC_USER_MODE_HALFOP;
                    Position++;
                }

                User->Nick = STRINGALLOC(Position);
                strcpy(User->Nick, Position);

                int i = 0; // remvoe me
            }
        }
        // NOTE: End of /NAMES list. Query channel for modes
        else if (!strcmp(Command, "366"))
        {
            // 324
            if (!Parameters)
            {
                return;
            }
            NewNick = Parameters;

            ChanName = strchr(NewNick, ' ');
            *ChanName = '\0';
            ChanName++;

            Data = strchr(ChanName, ' ');
            *Data = '\0';
            Data++;

            SendCommand(Connection, "MODE", ChanName);
            SendMode(Connection, "+b", ChanName);
        }
        // NOTE: Banlist
        else if(!strcmp(Command, "367"))
        {
            // NOTE: One ban per line, requestnick #channel address setter time
            int i = 0; // NOTE: REMOVE ME
            NewNick = Parameters;

            ChanName = strchr(NewNick, ' ');
            *ChanName = '\0';
            ChanName++;

            HostName = strchr(ChanName, ' ');
            *HostName = '\0';
            HostName++;

            Setter = strchr(HostName, ' ');
            *Setter = '\0';
            Setter++;

            TimeStr = strchr(Setter, ' ');
            *TimeStr = '\0';
            TimeStr++;

            Time = atol(TimeStr);

            ChanList = Connection->ChannelList;
            while(ChanList)
            {
                if (!strcmp(ChanList->Name, ChanName))
                {
                    break;
                }
                ChanList = ChanList->Next;
            }

            if (!ChanList->Bans)
            {
                ChanList->Bans = BANALLOC;
                ZERO(ChanList->Bans, ban_item);
            }

            ban_item *BanList = ChanList->Bans;
            while(BanList->Setter)
            {
                if (!BanList->Next)
                {
                    BanList->Next = BANALLOC;
                    ZERO(BanList->Next, ban_item);
                }
                BanList = BanList->Next;
            }

            BanList->Setter = STRINGALLOC(NewNick);
            strcpy(BanList->Setter, NewNick);

            BanList->Time = Time;

            BanList->Host = STRINGALLOC(HostName);
            strcpy(BanList->Host, HostName);

        }
        else if (!strcmp(Command, "NOTICE"))
        {

        }
        else if (!strcmp(Command, "PRIVMSG"))
        {

        }
        //TODO: Test this.
        else if (!strcmp(Command, "NICK"))
        {
            NewNick = strchr(Parameters, ':');
            *NewNick = '\0';
            NewNick++;

            if (!strcmp(From.nick, Connection->Nick))
            {
                //update your own nick
                free(Connection->Nick);
                Connection->Nick = STRINGALLOC(Parameters);
                strcpy(Connection->Nick, Parameters);
            }
            else
            {
                ChanList = Connection->ChannelList;
                channel_user * UserList;

                if (ChanList)
                {
                    while (ChanList)
                    {
                        UserList = ChanList->Users;
                        if (UserList)
                        {
                            while (UserList)
                            {
                                if (!strcmp(UserList->Nick, From.nick))
                                {
                                    free(UserList->Nick);
                                    UserList->Nick = STRINGALLOC(NewNick);
                                    strcpy(UserList->Nick, NewNick);

                                    if (!UserList->Ident)
                                    {
                                        UserList->Ident = STRINGALLOC(
                                                From.ident);
                                        strcpy(UserList->Ident, From.ident);
                                    }

                                    if (!UserList->Host)
                                    {
                                        UserList->Host = STRINGALLOC(From.host);
                                        strcpy(UserList->Host, From.host);
                                    }

                                }
                                UserList = UserList->Next;
                            }
                        }
                        ChanList = ChanList->Next;
                    }
                }
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
