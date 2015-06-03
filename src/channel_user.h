#if !defined(CHANNEL_USER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice:  $
   ======================================================================== */

#define CHANNEL_USER_H

#include <iostream>
#include <list>

class ChannelUser
{
public:
    ChannelUser();
    ~ChannelUser();
    std::string getNick();
    std::string getHost();
    std::string getIdent();
    void setNick(const std::string newNick);
    void setHost(const std::string newHost);
    void setIdent(const std::string NewIdent);
    long getModes();
    void setModes(long value);
private:
    std::string Nick;
    std::string Host;
    std::string Ident;
    long Modes;    
}
#endif
