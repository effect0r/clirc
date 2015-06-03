/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice: $
   ======================================================================== */
#include "channel_user.h"

ChannelUser::ChannelUser()
{
    Nick("");
    Host("");
    Ident("");
    Modes = 0;
}

ChannelUser::~ChannelUser()
{
}

std::string
ChannelUser::getNick()
{
    return Nick;
}

std::string
ChannelUser::getHost()
{
    return Host;
}

std::string
ChannelUser::getIdent()
{
    return Udent;
}

long
ChannelUser::getModes()
{
    return Modes;
}

void
ChannelUser::setNick(const std::string newNick)
{
    Nick = newNick;
}

void
ChannelUser::setHost(const std::string newIdent)
{
    Nick = newIdent;
}

void
ChannelUser::setIdent(const std::string newHost)
{
    Nick = newIdent;
}

