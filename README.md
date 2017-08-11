# BoostAsioChat
A simple irc inspired chat tcp server/client in order to get into Boost Asio. 
## Requirements 
In order to build this project you will need [boost](http://www.boost.org/) 
(libs: boost_system, boost_thread, boost_serialization) c++11 and [cmake](https://cmake.org/download/). 
## Building 
```
cmake . && make
```
The binaries will be placed under _build/bin/client_ and _build/bin/server_. 
## Usage
### Server 
To run the server:
```
./Server <port>
```
Where __\<port\>__ is the port-number. 
#### Commands
* List all users currently on the server
```
/users
```
* Chek if a user with the name __\<name\>__ exists and what channels he/she joined
```
/u <name>
/user <name>
```
* List all channels 
```
/channels
```
* Chek if a channel with the name __\<name\>__ exists and what users are joined to it
```
/c <name>
/channel <name>
```
* Quit (stopping the server) 
```
/q
/quit
```
### Client 
To run the client:
```
./Client <host> <port>
```
Where __\<host\>__ is the hostname (asio-chat.com for example) __\<port\>__ is the port-number. 
When running the client you will need to pick a vaild name (3 to 20 alphanumeric characters starting with [a-z A-Z]).
In order to talk in a chat channel or to another user use /t \<name\> (see below).  
#### Commands
* Change the user or channel you are talking to. Once entered your following messages will be 
delivered to the user or channel with the name __\<name\>__.
```
/t <name>
/target <name>
```
* Join the channel with the name __\<name\>__. (If it does not exist it will be created)
```
/j <name>
/join <name>
```
* Leave the channel with the name __\<name\>__. (If the user was the last member the channel will be destroyed)
```
/leave <name>
```
* Change the user name to __\<newname\>__. The name has to be valid (No users or channels with the same name). 
```
/nick <newname>
```
* Quit (stopping the client) 
```
/q
/quit
```
