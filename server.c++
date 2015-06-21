/********************************************************************
 * Author:  Carlos Moreno
 * Created: 2015-06-04
 * 
 * Description:
 * 
 *      This is the file used for question 2 of assignment 1.  You
 *      may also use it as sample / starting point to create the 
 *      server for question 1.  In particular, you are allowed to 
 *      submit your code containing verbatim fragments from this 
 *      file.
 * 
 * Copytight and permissions:
 *      This file is for the exclusive purpose of our ECE-458 
 *      assignment 2, and you are not allowed to use it for any 
 *      other purpose.
 * 
 ********************************************************************/

#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>

#include<cstdlib>
#include <fstream>  
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <wait.h>
#include <unistd.h>
#include "crypto.h"

class connection_closed {};
class socket_error {};

void listen_connections (int port);
void process_connection (int client_socket);
string read_packet (int client_socket);

int main (int na, char * arg[])
{
    listen_connections (10458);

    return 0;
}

void listen_connections (int port)
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len;

    server_socket = socket (AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons (port);

    bind (server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen (server_socket, 5);

    while (true)
    {
        client_len = sizeof(client_address);
        client_socket = accept (server_socket,
                                (struct sockaddr *) &client_address,
                                &client_len);

        pid_t pid = fork();
        if (pid == 0)           // if we're the child process
        {
            close (server_socket);    // only the parent listens for new connections

            if (fork() == 0)    // detach grandchild process -- parent returns immediately
            {
                usleep (10000); // Allow the parent to finish, so that the grandparent
                                // can continue listening for connections ASAP

                process_connection (client_socket);
            }

            return;
        }

        else if (pid > 0)       // parent process; close the socket and continue
        {
            int status = 0;
            waitpid (pid, &status, 0);
            close (client_socket);
        }

        else
        {
            cerr << "ERROR on fork()" << endl;
            return;
        }
    }
}

void process_connection (int client_socket)
{
    try
    {
        map<string,string> passwords;
        passwords["user1"] = "password1";
        passwords["user2"] = "password2";
            // For the real server, this will be populated with the 
            // usernames and passwords from a text file.

        const string & username = read_packet (client_socket);
        const string & db_password = passwords[username];
		
		const string & R = cgipp::sha256("start");
		cout<<R <<endl;
        char str[17];
        static const char alphanum[] = "0123456789abcdef";
        /*
        for(int i = 0; i < 17; ++i){
            str[i] = alphanum[rand()%(sizeof(alphanum)-1)];
        }
		*/
		FILE *fin;
		if ((fin = fopen("/dev/urandom", "r")) == NULL) {
                fprintf(stderr, "%s: unable to open file\n", "/dev/urandom");
                return;
        }
        int len = 16;
        unsigned char buffer[len];
        char hexbuf[len*2+1];
        if(fread(buffer, 1, sizeof(buffer), fin) == sizeof(buffer)){
        	
        	hexbuf[len*2+1] = 0;
        	for (int i = 0; i < sizeof(buffer); i++)
			{
				sprintf(&hexbuf[2 * i], "%02x", buffer[i]);
			}
        }
        fclose(fin);
        cout<<"hex: " <<hexbuf<<endl;
		
		send (client_socket, "ok\n", 4, MSG_NOSIGNAL);
		
        while (true)
        {
            const string & password = read_packet (client_socket);
            if (password == db_password)
            {
                send (client_socket, "ok\n", 4, MSG_NOSIGNAL);
            }
            else
            {
                send (client_socket, "failed\n", 8, MSG_NOSIGNAL);
            }
        }

        close (client_socket);
    }
    catch (connection_closed)
    {
    }
    catch (socket_error)
    {
        cerr << "Socket error" << endl;
    }
}


string read_packet (int client_socket)
{
    string msg;

    const int size = 8192;
    char buffer[size];

    while (true)
    {
        int bytes_read = recv (client_socket, buffer, sizeof(buffer) - 2, 0);
            // Though extremely unlikely in our setting --- connection from 
            // localhost, transmitting a small packet at a time --- this code 
            // takes care of fragmentation  (one packet arriving could have 
            // just one fragment of the transmitted message)

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            buffer[bytes_read + 1] = '\0';

            const char * packet = buffer;
            while (*packet != '\0')
            {
                msg += packet;
                packet += strlen(packet) + 1;

                if (msg.length() > 1 && msg[msg.length() - 1] == '\n')
                {
                    istringstream buf(msg);
                    string msg_token;
                    buf >> msg_token;
                    return msg_token;
                }
            }
        }

        else if (bytes_read == 0)
        {
            close (client_socket);
            throw connection_closed();
        }

        else
        {
            cerr << "Error " << errno << endl;
            throw socket_error();
        }
    }

    throw connection_closed();
}
