#include "main.h"
#include "sqlite3.cpp"
#include "irc_connection.cpp"

map MapNew(unsigned int size)
{
	map Result = {0};
	Result.Size = size;
	Result.Used = 0;
	Result.Pairs = (pair*)malloc(sizeof(pair) * size);
	return Result;
}

int MapSearch(map *Map, char *Key)
{
	int Result = -1;
	for(unsigned int i = 0; i < Map->Used; ++i)
	{
		pair Current = Map->Pairs[i];
		if (!strcmp(Current.Key, Key))
		{
			Result = i;
			break;
		}
	}
	return Result;
}

void MapInsert(map *Map, char *Key, char *Value)
{
	pair InsertPair;
	InsertPair.Key = Key;
	InsertPair.Value = Value;

	if (Map->Used == Map->Size)
	{
		pair *PairCopy = (pair*)malloc(sizeof(pair)*(2*Map->Size));
		memcpy(PairCopy, Map->Pairs, sizeof(pair)*Map->Size);

		free(Map->Pairs);
		Map->Size *= 2;
		Map->Pairs = PairCopy;
	}

	Map->Pairs[Map->Used++] = InsertPair;
}

int CharCount(char *String, char Check)
{
	// NOTE(cory): 
	// a,b,c,
	// 	return 3
	// a,b,c
	// 	return 3
	int Result = 1;
	for (unsigned i = 0; i < strlen(String); ++i)
	{
		char At = String[i];
		if ((At == Check) &&
				(i != strlen(String)-1))
		{
			Result++;
		}
	}
	return Result;
}

void OpenFile(irc_connection *Conn, char *FileName)
{
	struct stat Stats;
	int descriptor = open(FileName, O_RDONLY);
	if (descriptor > 0)
	{
		if (stat(FileName, &Stats) != -1)
		{
			char *FileData = (char*)mmap(0, Stats.st_size, PROT_READ, MAP_SHARED, descriptor, 0);
			if (FileData)
			{
				char *DataCopy = (char*)malloc(sizeof(char)*strlen(FileData));
				memcpy(DataCopy, FileData, strlen(FileData)); 

				char *Line = DataCopy;

				while(*Line && (*Line != '\n'))
				{
					char *Item = Line;
					char *Value = strchr(Line, '=');
					char *NextLine = strchr(Line, '\n');
					if (Value)
					{
						*Value++ = '\0';
					}
					if (NextLine)
					{
						*NextLine++ = '\0';
					}

					if (!strcmp(Item, "database"))
					{
						int rc = sqlite3_open(Value, &Conn->ConfigInfo.Database);
						if (rc == SQLITE_OK)
						{
							printf("DB opened successfully\n");
						}
					}
					else if (!strcmp(Item, "prefix"))
					{
						Conn->ConfigInfo.CommandPrefix = Value[0];
					}
					else if (!strcmp(Item, "nick"))
					{
						Conn->ConfigInfo.Nick = STRINGALLOC(Value);
						strcpy(Conn->ConfigInfo.Nick, Value);
					}
					else if (!strcmp(Item, "server"))
					{
						Conn->ConfigInfo.Server = STRINGALLOC(Value);
						strcpy(Conn->ConfigInfo.Server, Value);
					}
					else if (!strcmp(Item, "port"))
					{
						Conn->ConfigInfo.Port = STRINGALLOC(Value);
						strcpy(Conn->ConfigInfo.Port, Value);
					}
					else if (!strcmp(Item, "pass"))
					{
						Conn->ConfigInfo.Pass = STRINGALLOC(Value);
						strcpy(Conn->ConfigInfo.Pass, Value);
					}
					else if (!strcmp(Item, "user"))
					{
						Conn->ConfigInfo.User = STRINGALLOC(Value);
						strcpy(Conn->ConfigInfo.User, Value);
					}
					else if (!strcmp(Item, "admins"))
					{
						char *Admin = Value;
						int AdminCount = CharCount(Admin, ',');
						for (int i = 0; i < AdminCount; ++i)
						{
							char *NextAdmin = strchr(Admin, ',');
							if (NextAdmin)
							{
								*NextAdmin++ = '\0';
							}
							Conn->ConfigInfo.Admins[i] = (char*)malloc(sizeof(char) * (strlen(Admin) + 1));
							strcpy(Conn->ConfigInfo.Admins[i], Admin);

							Admin = NextAdmin;
						}
						Conn->ConfigInfo.AdminCount = AdminCount;
					}
					else
					{
						//Invalid option, ignore.
					}
					Line = NextLine;
				}

				free(DataCopy);
			}
			munmap(FileData, Stats.st_size);
		}
	}
	close(descriptor);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Error. supply the configureation file. See the docs for more info\n");
		return 0;
	}

	irc_connection Conn = { 0 };

	OpenFile(&Conn, argv[1]);

	Connect(&Conn);

	MessageLoop(&Conn);

	Conn = { 0 };
	return 0;

}
