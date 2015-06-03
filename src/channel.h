#if !defined(CHANNEL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice:  $
   ======================================================================== */

#define CHANNEL_H

#include <string>
#include <list>
#include <deque>

class Channel
{
public:
    Channel();
    ~Channel();
    std::string getName() const;
    std::string getTopic() const;
    std::string getTopicSetBy() const;
    std::string getPassword() const;
    long getTopicSetTime() const;
    long getCreationTime() const;
    long getModes() const;
    long getLimit() const;

    void setName(const std::string newName);
    void setTopic(const std::string newTopic);
    void setTopicSetBy(const std::string newTopicSetBy);
    void setPassword(const std::string newPassword);
    void setTopicSetTime(const long newTime);
    void setCreationTime(const long newTime);
    void setModes(const long newModes);
    void setLimit(const long newLimit);
private:
    std::string Name;
    std::string Topic;
    std::string TopicSetBy;
    std::string Password;
    long TopicSetTime;
    long CreationTime;
    long Modes;
    long Limit;

    std::list<ban_item> BanList;
    std::deque<channel_users> UserList;
}

#endif
