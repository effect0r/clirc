/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice: $
   ======================================================================== */
#include "channel.h"

Channel::Channel()
{
    Name("");
    Topic("");
    TopicSetBy("");
    Password("");
    TopicSetTime = 0;
    CreationTime = 0;
    Modes = 0;
    Limit = 0;
    
}
