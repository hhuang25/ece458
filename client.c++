/********************************************************************
 * Author:  Carlos Moreno
 * Created: 2015-06-04
 * 
 * Description:
 * 
 *      This is a sample code to connect to a server through TCP.
 *      You are allowed to use this as a sample / starting point 
 *      for the assignment (both problems require a program that 
 *      connects to something)
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

#include <cstdlib>
#include <fstream> 
#include "crypto.h"
#include "encodings.h"
#include "stdint.h"
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <wait.h>
#include <unistd.h>

int socket_to_server (const char * IP, int port);
string read_packet (int socket);

class connection_closed {};
class socket_error {};


int main(int argc, char *argv[])
{
	//cout<<"begin client"<<endl;
    int socket = socket_to_server ("127.0.0.1", 12592);
        // The function expects an IP address, and not a 
        // hostname such as "localhost" or ecelinux1, etc.
	cout<<"begin client"<<endl;
	if(socket == -1){
		cout<<"socket is " <<socket << " please refresh and try again." << endl;
	}
	int len = 16; // this is string length before hex conversion
    if (socket != -1)
    {
        //send (socket, "user1\n", 7, MSG_NOSIGNAL);
        //usleep (10000);
        //send (socket, "password1\n", 11, MSG_NOSIGNAL);
		
		const string &hexRP = read_packet(socket);
		//cout <<"received " +  hexRP << endl;
		
		int spaceLocation = hexRP.find(' ', 0);
		//cout<<hexRP.length() << " " << spaceLocation<<endl;
		const string &R = cgipp::hex_decoded(hexRP.substr(0,spaceLocation));
		//cout <<"R is " +  R << endl;
		const string &P = cgipp::hex_decoded(hexRP.substr(spaceLocation+1, hexRP.length() - spaceLocation-2));
        
        //cout <<"P is " +  P << endl;
        //int pLength = hexRP.length() - len - 1;
        
        //FILE *fin;
        int iterations = 0;
        //char buffer[len];
        //char hexbuf[len*2+1];
        //string X;
        uint64_t rannum = rand();
        while(true){
        	/*
			if ((fin = fopen("/dev/urandom", "r")) == NULL) {
		            fprintf(stderr, "%s: unable to open file\n", "/dev/urandom");
		            break;
		    }
		    
		    if(fread(buffer, 1, sizeof(buffer), fin) == sizeof(buffer)){
		    	X = string(buffer,len);
		    	
		    	//hexbuf[len*2+1] = 0;
		    	//for (int i = 0; i < sizeof(buffer); i++)
				//{
				//	sprintf(&hexbuf[2 * i], "%02x", buffer[i]);
				//}
				
		    }
		    fclose(fin);
		    */
		    //cout<<"x is " <<X<<endl;
		    char rawX[16];
		    sprintf(rawX,"%016lx",rannum);
		    //cout<<"rawX: " <<rawX<<endl;
		    rannum++;
		    const string &X(rawX);
		    string total = R;
		    total.append(X);
		    total.append(R);
		    const string hash = cgipp::sha256(total);
		    ++iterations;
		    //cout<<hash.substr(0,pLength)<<" vs " <<P<<endl;
		    if(cgipp::hex_decoded(hash).substr(0,P.length()) == P){
		    	total = cgipp::hex_encoded(total);
		    	total.append("\n");
		    	send (socket, total.c_str(), total.length(), MSG_NOSIGNAL);
		    	cout<<iterations <<" iterations: " <<hash<<endl;
		    	break;
		    }
		    
        }
        usleep (10000);
        cout<<"did i get it?" <<endl;
        if(read_packet(socket) == "ok\n"){
        	cout<<"yay"<<endl;
        }else{
        	cout<<"oops"<<endl;
        }
        close(socket);
        cout<<"exit" <<endl;
    }

    return 0;
}

int socket_to_server (const char * IP, int port)
{
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr (IP);
    address.sin_port = htons(port);

    int sock = socket (AF_INET, SOCK_STREAM, 0);

    if (connect (sock, (struct sockaddr *) &address, sizeof(address)) == -1)
    {
        return -1;
    }

    return sock;
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
                    //istringstream buf(msg);
                    //string msg_token;
                    //buf >> msg_token;
                    return msg;
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
