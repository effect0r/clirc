#include "main.h"

map* MapNew(unsigned int size)
{
	map *Result = (map*)malloc(sizeof(map));
	Result->Size = size;
	Result->Used = 0;
	Result->Pairs = (pair*)malloc(sizeof(pair) * size);
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
	InsertPair.Key = STRINGALLOC(Key);
	InsertPair.Value = STRINGALLOC(Value);

	strcpy(InsertPair.Key, Key);
	strcpy(InsertPair.Value, Value);

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

void MapRemove(map *Map, char *Key)
{
	int Position = MapSearch(Map, Key);
	Map->Pairs[Position] = Map->Pairs[Map->Used--];
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

void ProcessInfoDB(irc_connection*);
#include "irc_connection.cpp"

static int CountQuery(void *data, int NumArgs, char **Rows, char **Columns)
{
	int NumQuotes = 0;
	for (int i = 0; i < NumArgs; ++i)
	{
		if (!strcmp(Columns[i], "count(*)"))
		{
			NumQuotes = atoi(Rows[i]);
		}
		printf("%s = %s\n", Columns[i], Rows[i] ? Rows[i] : "NULL");
	}
	irc_connection *Connection = (irc_connection*)data;
	Connection->ConfigInfo.QuoteList.TotalQuotes = NumQuotes;
	return 0;
}

void ProcessInfoDB(irc_connection *Connection)
{
	Connection->ConfigInfo.FaqCommandsMap = MapNew(10);
	char *InfoFN = Connection->ConfigInfo.FaqCommandsFileName;
	map *Map = Connection->ConfigInfo.FaqCommandsMap;

	int InfoDesc = open(InfoFN, O_RDONLY);

	struct stat InfoStats;
	if (stat(InfoFN, &InfoStats) != -1)
	{
		char *CommandsList = (char*)mmap(0, InfoStats.st_size, PROT_READ, MAP_SHARED, InfoDesc, 0);
		if (CommandsList)
		{
			char *ListCopy = (char*)malloc(sizeof(char)*strlen(CommandsList));
			memcpy(ListCopy, CommandsList, strlen(CommandsList));

			char *EndOfFile = &ListCopy[strlen(ListCopy)];
			char *Words = ListCopy;
			for (;;)
			{
				if ((Words == EndOfFile) ||
					(Words[0] == '\n'))
				{
					break;
				}
				char *Spam = strchr(Words, '\t');
				if (Spam)
				{
					*Spam++ = '\0';
				}
				char *NextLine = strchr(Spam, '\n');
				if (NextLine)
				{
					*NextLine++ = '\0';
				}
				int WordCount = CharCount(Words, ',');
				for (int i = 0; i < WordCount; ++i)
				{
					char *Word = strchr(Words, ',');
					if (Word)
					{
						*Word++ = '\0';
					}
					MapInsert(Map, Words, Spam);
					Words = Word;
				}
				Words = NextLine;
			}
			free(ListCopy);
			munmap(CommandsList, InfoStats.st_size);
		}
	}
	close(InfoDesc);
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
				if (!Conn->ConfigInfo.ConfigFileName)
				{
					Conn->ConfigInfo.ConfigFileName = (char*)malloc(sizeof(char) + strlen(FileName));
					strcpy(Conn->ConfigInfo.ConfigFileName, FileName);
				}
				char *DataCopy = (char*)malloc(sizeof(char)*strlen(FileData));
				memcpy(DataCopy, FileData, strlen(FileData)); 

				char *Header = DataCopy, *Items;
				char *EndOfFile = &DataCopy[strlen(DataCopy)];
				for(;;)
				{
					if (Header == EndOfFile)
					{
						break;
					}

					Items = strchr(Header, '=');
					char *NewLine = strchr(Items, '\n');
					if (Items)
					{
						*Items++ = '\0';

						if (Items[0] == '\n')
						{
							//nothing here
							*Items++ = '\0';
							Header = Items;
							continue;
						}
						else
						{
							NewLine = strchr(Items, '\n');
							if (NewLine)
							{
								*NewLine++ = '\0';
							}
							if (!strcmp(Header, "channels"))
							{
								int ChannelCount = CharCount(Items, ',');
								// NOTE(effect0r): Uh, make this better. Seriously.
								Conn->ConfigInfo.ChannelList = (char **)malloc(sizeof(char)*ChannelCount);
								for (int i = 0; i < ChannelCount; ++i)
								{
									char *Chan = strchr(Items, ',');
									if (Chan)
									{
										*Chan++ = '\0';
									}
									Conn->ConfigInfo.ChannelCount = ChannelCount;
									Conn->ConfigInfo.ChannelList[i] = STRINGALLOC(Items);
									strcpy(Conn->ConfigInfo.ChannelList[i], Items);
									Items = Chan;
								}
							}
							else if (!strcmp(Header, "prefix"))								
							{
								Conn->ConfigInfo.CommandPrefix = Items[0];
							}
							else if (!strcmp(Header, "nick"))								
							{
								Conn->ConfigInfo.Nick = STRINGALLOC(Items);
								strcpy(Conn->ConfigInfo.Nick, Items);
							}
							else if (!strcmp(Header, "server"))								
							{
								Conn->ConfigInfo.Server = STRINGALLOC(Items);
								strcpy(Conn->ConfigInfo.Server, Items);
							}
							else if (!strcmp(Header, "port"))
							{
								Conn->ConfigInfo.Port = STRINGALLOC(Items);
								strcpy(Conn->ConfigInfo.Port, Items);
							}
							else if (!strcmp(Header, "pass"))
							{
								Conn->ConfigInfo.Pass = STRINGALLOC(Items);
								strcpy(Conn->ConfigInfo.Pass, Items);
							}
							else if (!strcmp(Header, "user"))
							{
								Conn->ConfigInfo.User = STRINGALLOC(Items);
								strcpy(Conn->ConfigInfo.User, Items);
							}
							else if (!strcmp(Header, "admin"))
							{
								Conn->ConfigInfo.Admin = STRINGALLOC(Items);
								strcpy(Conn->ConfigInfo.Admin, Items);
							}
							else if (!strcmp(Header, "whitelisted"))
							{
								char *CurrentUser = Items;
								char *EndOfFile = strchr(Items, '\n');
								int TotalUsers = CharCount(Items, ',');
								Conn->ConfigInfo.WhiteListCount = TotalUsers;
								Conn->ConfigInfo.WhiteList = (char**)malloc(sizeof(char) * TotalUsers);
								for (int i = 0; i < TotalUsers; ++i)
								{
									char *NextUser = strchr(CurrentUser, ',');
									if (NextUser)
									{
										*NextUser++ = '\0';
									}
									Conn->ConfigInfo.WhiteList[i] = STRINGALLOC(CurrentUser);
									strcpy(Conn->ConfigInfo.WhiteList[i], CurrentUser);
									CurrentUser = NextUser;
								}
							}
							else if (!strcmp(Header, "quotedb"))
							{
								char *DbFilename = Items;
								int rc = sqlite3_open(DbFilename, &Conn->ConfigInfo.QuoteList.QuoteDB);
								if (rc == SQLITE_OK)
								{
									printf("DB opened successfully\n");
									char *ErrorMsg = 0;
									char *Query = "SELECT count(*) FROM QUOTE;";
									int rc = sqlite3_exec(Conn->ConfigInfo.QuoteList.QuoteDB, Query, CountQuery, (void*)Conn, &ErrorMsg);
									if (rc != SQLITE_OK) 
									{
										sqlite3_free(ErrorMsg);
									}
								}
							}
							else if (!strcmp(Header, "infodb"))
							{
								Conn->ConfigInfo.FaqCommandsFileName = (char*)malloc(sizeof(char) * strlen(Items));
								strcpy(Conn->ConfigInfo.FaqCommandsFileName, Items);

								ProcessInfoDB(Conn);
							}
						}
					}
					Header = NewLine;
				}

				munmap(FileData, Stats.st_size);
				free(DataCopy);
				close(descriptor);
			}
		}
	}
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
