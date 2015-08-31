#if !defined(MAIN_H)
/* ========================================================================
 $File: $
 $Date: $
 $Revision: $
 $Creator: Cory Henderlite $
 $Notice:  $
 ======================================================================== */

#define MAIN_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "irc_connection.h"
//#include <netdb.h>

#define MAX_CHANNELS 20
#define MAX_USERS 1000
#define internal static
#define LENGTH 1024

#define STRINGALLOC(x) (char*)malloc(strlen(x) * sizeof(char))
#define CHANNELALLOC (channel*)malloc(sizeof(channel))
#define USERALLOC (channel_user*)malloc(sizeof(channel_user))
#define ZERO(x, type) memset(x, 0, sizeof(type))
#define ARRAYCOUNT(array) sizeof((array)) / sizeof((array)[0])
#define BANALLOC (ban_item*)malloc(sizeof(ban_item))

#endif
