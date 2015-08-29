/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice: $
   ======================================================================== */
#include "main.h"
#include "irc_connection.cpp"

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("**********CPP IRC BOT*********\n");
		printf("\tUsage:\n\t\tclirc server nick username realname port pass\n");
		return 0;
	}
	else 
	{
		char *Server, *Nick, *Username, *Realname, *Pass, *Port;
		if (argv[1]) 
		{
			Server = argv[1];
			printf("%s\n", Server);
		}
		else
		{
			printf("Server is required!\n");
			return 0;
		}
		if (argv[2]) 
		{
			Nick = argv[2];
			printf("%s\n", Nick);
		}
		else
		{
			printf("Nick is required!\n");
			return 0;
		}
		if (argv[3]) 
		{
			Username = argv[3];
			printf("%s\n", Username);
		}
		else
		{
			printf("Username is required!\n");
			return 0;
		}
		if (argv[4]) 
		{
			Realname = argv[4];
			printf("%s\n", Realname);
		}
		else
		{
			printf("Realname is required!\n");
			return 0;
		}
		if (argv[5]) 
		{
			Port = argv[5];
			printf("%s\n", Port);
		}
		else
		{
			printf("Port is required!\n");
			return 0;
		}
		if (argv[6]) 
		{
			Pass = argv[6];
		}
		
		irc_connection Conn = { 0 };

		Connect(&Conn, Server, Nick, Username, Realname, Pass, Port);
		MessageLoop(&Conn);

		Conn = { 0 };
		return 0;
	}
}
