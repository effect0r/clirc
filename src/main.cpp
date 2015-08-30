/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Cory Henderlite $
   $Notice: $
   ======================================================================== */
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
				//process 
				//"channels=#effect0r,#rob\nprefix=.\nnick=effect0r-cpp\nserver=irc.globalgamers.net\nport=6667\npass=\nuser=effect0r-cpp\n"
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
						*Items = '\0';
						Items++;
					}
					if (Items[0] == '\n')
					{
						//nothing here
						*Items = '\0';
						Items++;
						Header = Items;
						continue;
					}
					else
					{
						char *NewLine = strchr(Items, '\n');
						if (NewLine)
						{
							*NewLine = '\0';
							NewLine++;
						}
						//THINGS!
						if (!strcmp(Header, "channels"))
						{
							int ChannelCount = CharCount(Items, ',');
							// NOTE(cory): Uh, make this better. Seriously.
							Conn->ConfigInfo.ChannelList = (char **)malloc(sizeof(char)*(ChannelCount+1));
							for (int i = 0; i < ChannelCount+1; ++i)
							{
								char *Chan = strchr(Items, ',');
								if (Chan)
								{
									*Chan = '\0';
									Chan++;
								}
								Conn->ConfigInfo.ChannelCount = ChannelCount+1;
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
							Conn->Nick = STRINGALLOC(Items);
							memcpy(Conn->Nick, Items, strlen(Items));
						}
						else if (!strcmp(Header, "server"))								
						{
							Conn->Server = STRINGALLOC(Items);
							memcpy(Conn->Server, Items, strlen(Items));
						}
						else if (!strcmp(Header, "port"))
						{
							Conn->Port = STRINGALLOC(Items);
							memcpy(Conn->Port, Items, strlen(Items));
						}
						else if (!strcmp(Header, "pass"))
						{
							Conn->Pass = STRINGALLOC(Items);
							memcpy(Conn->Pass, Items, strlen(Items));
						}
						else if (!strcmp(Header, "user"))
						{
							Conn->User = STRINGALLOC(Items);
							memcpy(Conn->User, Items, strlen(Items));
						}
						else if (!strcmp(Header, "quotedb"))
						{
							int QuoteDesc = open(Items, O_RDONLY);
							struct stat QuotesStat;
							if (stat(Items, &QuotesStat) != -1)
							{
								char *QuotesList = (char*)mmap(0, QuotesStat.st_size, PROT_READ, MAP_SHARED, QuoteDesc, 0);
								if (QuotesList)
								{
									//Process quotes
								}
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
									//Process commands: <comma separated list of triggers>=<what to say>\n
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
											*NextLine = '\0';
											NextLine++;
										}
										int WordCount = CharCount(Line, ',') + 1;
										for (int i = 0; i < WordCount; ++i)
										{
											char *Word = strchr(Line, ',');
											if (Word)
											{
												*Word = '\0';
												Word++;
											}
											MapInsert(Conn->Map, Line, Spam);
											Line = Word;
										}
										Line = NextLine;
									}

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
	Conn.Map = MapNew(10);

	OpenFile(argv[1], &Conn);

	Connect(&Conn);

	MessageLoop(&Conn);

	Conn = { 0 };
	return 0;

}
