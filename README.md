# clirc

A C/C++ IRC bot devoted to channel control with a quote system and stream-time support for FAQs. Designed to be fairly 
modular, fast, and dynamically controllable/editable. Very much a WIP seeing as I'm still in school (last semester!).

After I graduate, work will pick up on this quickly, hopefully

## Config Options:
	prefix		- The command prefix, usually !.
	nick 		- The bot's nickname
	server		- The server to connect to.
	port		- The server's port.
	pass		- Your password to the server.
	user		- The userID of your bot.
	database	- The sqlite3 DB file.
	admins		- A comma separated list of the admins.
## Usage:
	The config system is set up to be as easy as possible.
	Everything dealing with the actual moderation is done within IRC. 
	The bot joins a channel with it's own name, 
	and from there anyone in the admins line can add and remove channels. 
	Within the channel, a user is designated as the owner. 
	The owner can add and remove triggers. 
	Triggers are things like !who and !what, which describe 
	your stream in terms of the word used. 
	Quotes can be added if wanted, assuming it's a person 
	who was either the admin of the channel OR on the channel's whitelist. 
	To add people to the whitelist, see the commands section.

## Commands
	[rm]channel <channel name> [<channel owner>] (admin only): 
		Adds/removes channel To the bot with channel owner as it's owner. 
		Bot automatically joins if added, and parts if removed.
	[rm]trigger <trigger> [<trigger reply>] (channel admin/whitelist only): 
		Adds/removes a trigger from the triggers list.
	[de]whitelist <user> (channel admin only) :
		Adds/removes a user from the whitelist.
## About Me
	I'm a 30 year old software developer who really loves getting 
	to the metal of programming. I'm not one for libraries, but I'm not 
	opposed to using them when appropriate.
