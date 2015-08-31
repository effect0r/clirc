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

	memcpy(InsertPair.Key, Key, strlen(Key));
	memcpy(InsertPair.Value, Value, strlen(Value));

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
	int Result = 0;
	for (unsigned i = 0; i < strlen(String); ++i)
	{
		char At = String[i];
		if (At == Check)
		{
			Result++;
		}
	}
	return Result;
}

#include "irc_connection.cpp"

void OpenFile(char *FileName, irc_connection *Conn)
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

				char *Header = DataCopy, *Items;
				char *EndOfFile = &DataCopy[strlen(DataCopy)];
				for(;;)
				{
					if (Header == EndOfFile)
					{
						break;
					}

					Items = strchr(Header, '=');
					if (Items)
					{
						*Items++ = '\0';
					}
					if (Items[0] == '\n')
					{
						//nothing here
						*Items++ = '\0';
						Header = Items;
						continue;
					}
					else
					{
						char *NewLine = strchr(Items, '\n');
						if (NewLine)
						{
							*NewLine++ = '\0';
						}
						if (!strcmp(Header, "channels"))
						{
							int ChannelCount = CharCount(Items, ',') + 1;
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
								memcpy(Conn->ConfigInfo.ChannelList[i], Items, strlen(Items));
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
							memcpy(Conn->ConfigInfo.Nick, Items, strlen(Items));
						}
						else if (!strcmp(Header, "server"))								
						{
							Conn->ConfigInfo.Server = STRINGALLOC(Items);
							memcpy(Conn->ConfigInfo.Server, Items, strlen(Items));
						}
						else if (!strcmp(Header, "port"))
						{
							Conn->ConfigInfo.Port = STRINGALLOC(Items);
							memcpy(Conn->ConfigInfo.Port, Items, strlen(Items));
						}
						else if (!strcmp(Header, "pass"))
						{
							Conn->ConfigInfo.Pass = STRINGALLOC(Items);
							memcpy(Conn->ConfigInfo.Pass, Items, strlen(Items));
						}
						else if (!strcmp(Header, "user"))
						{
							Conn->ConfigInfo.User = STRINGALLOC(Items);
							memcpy(Conn->ConfigInfo.User, Items, strlen(Items));
						}
						else if (!strcmp(Header, "admins"))
						{
							Conn->ConfigInfo.Admin = STRINGALLOC(Items);
							memcpy(Conn->ConfigInfo.Admin, Items, strlen(Items));
						}
						else if (!strcmp(Header, "whitelisted"))
						{
							char *CurrentUser = Items;
							char *EndOfFile = strchr(Items, '\n');
							int TotalUsers = CharCount(Items, ',') + 1;
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
								memcpy(Conn->ConfigInfo.WhiteList[i], CurrentUser, strlen(CurrentUser));
								CurrentUser = NextUser;
							}
						}
						else if (!strcmp(Header, "quotedb"))
						{
							char *DbFilename = Items;
							int rc = sqlite3_open(DbFilename, &Conn->ConfigInfo.QuoteList);
							if (rc)
							{
								//error
							}
							else 
							{
								printf("DB opened successfully\n");
							}
						}
						else if (!strcmp(Header, "infodb"))
						{
							int InfoDesc = open(Items, O_RDONLY);
							struct stat InfoStats;
							if (stat(Items, &InfoStats) != -1)
							{
								char *CommandsList= (char*)mmap(0, InfoStats.st_size, PROT_READ, MAP_SHARED, InfoDesc, 0);
								if (CommandsList)
								{
									char *ListCopy = (char*)malloc(sizeof(char)*strlen(CommandsList));
									memcpy(ListCopy, CommandsList, strlen(CommandsList));

									char *EndOfFile = &ListCopy[strlen(ListCopy)];
									char *Line = ListCopy;
									for (;;)
									{
										if (Line == EndOfFile)
										{
											break;
										}
										char *Spam = strchr(Line, '=');
										if (Spam)
										{
											*Spam++ = '\0';
										}
										char *NextLine = strchr(Spam, '\n');
										if (NextLine)
										{
											*NextLine++ = '\0';
										}
										int WordCount = CharCount(Line, ',') + 1;
										for (int i = 0; i < WordCount; ++i)
										{
											char *Word = strchr(Line, ',');
											if (Word)
											{
												*Word++ = '\0';
											}
											MapInsert(Conn->ConfigInfo.FaqCommandsMap, Line, Spam);
											Line = Word;
										}
										Line = NextLine;
									}
									free(ListCopy);
									munmap(CommandsList, InfoStats.st_size);
									close(InfoDesc);
								}
							}
						}
						Header = NewLine;
					}
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
	Conn.ConfigInfo.FaqCommandsMap = MapNew(10);

	OpenFile(argv[1], &Conn);

	Connect(&Conn);

	MessageLoop(&Conn);

	Conn = { 0 };
	return 0;

}
