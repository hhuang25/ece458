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

#include <inttypes.h>
#include <math.h>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <wait.h>
#include <unistd.h>

static __inline__ uint64_t rdtsc();
bool password_ok (const string &pwd);
int socket_to_server (const char * IP, int port);
string read_packet (int socket);

class connection_closed {};
class socket_error {};


int main(int argc, char *argv[])
{
	/*
    int socket = socket_to_server ("127.0.0.1", 10458);
        // The function expects an IP address, and not a 
        // hostname such as "localhost" or ecelinux1, etc.

    if (socket != -1)
    {
        send (socket, "user1\n", 7, MSG_NOSIGNAL);
        usleep (100000);
        send (socket, "password1\n", 11, MSG_NOSIGNAL);

        cout << read_packet (socket) << endl;
    }
	*/
	string possiblePwd = "";
	if(argc == 2){
		possiblePwd = argv[1];
	}
	srand(time(NULL));
	bool finished = false;
	int letters = 0;
	
	static const char alphabet[] = "ghijnoptuvwabcxyzdefqrsklm";
	
	while(!finished){
		uint64_t timevalues [26] = {};
		uint64_t squarevalues [26] = {};
		int times_picked [26] = {};
		//destroy possible caching
		for(int i = 0; i < 10000000; i++){
			int chosen_char = rand()%(sizeof(alphabet)-1);
			const char &current = alphabet[chosen_char];
			string pwd_attempt = possiblePwd + current;
			const uint64_t start = rdtsc();
            password_ok (pwd_attempt);
            const uint64_t end = rdtsc();
            
		}
		for(int i = 0; i < 10000000; i++){
            /*for(int j = 0; j < 10; j++){
			    int chosen_char = rand()%(sizeof(alphabet)-1);
			    const char &current = alphabet[chosen_char];
			    string pwd_attempt = possiblePwd + current;
			    //password_ok (pwd_attempt);
		    }*/

			int chosen_char = rand()%(sizeof(alphabet)-1);
			char current = alphabet[chosen_char];
			
			string pwd_attempt = possiblePwd + current;
			uint64_t start = rdtsc();
			finished = password_ok (pwd_attempt);
			uint64_t end = rdtsc();
			if(finished){
				cout<<"password guessed correct: " <<pwd_attempt <<endl;
				break;
			}
			// ... execution time is end – start
			uint64_t difference = end - start;
			//cout<< alphabet[chosen_char]<< ": time of " <<difference <<endl; 
			timevalues[chosen_char] += difference;
			squarevalues[chosen_char] += (difference*difference);
			times_picked[chosen_char] = times_picked[chosen_char] + 1;
			
		}
		if(finished) break;
		//initialize to "a"
		double highest_mean = 0.0;
		double highest_var = 0.0;
		char highest_char = 'a';
		double nexthighest_mean = 0.0;
		int highest_picktime = 0;
		for(int i = 0; i < 26; i++){
			double current_mean = (double)timevalues[i] / times_picked[i];
			double current_var = ((double)1/ (double)(times_picked[i] - 1)) * (double)(squarevalues[i] - times_picked[i] * (current_mean * current_mean));
			if(current_mean > nexthighest_mean && current_mean < highest_mean){
				nexthighest_mean = current_mean;
			}
			if(current_mean > highest_mean){
				nexthighest_mean = highest_mean;
				highest_mean = current_mean;
				highest_var = current_var;
				highest_picktime = times_picked[i];
				highest_char = alphabet[i];
			}
			cout<< alphabet[i] <<" mean: " <<current_mean << " ms, \tvariance: " <<current_var << " \tpicked " << times_picked[i] <<" times" <<endl;
		}
		double confidence95 = 1.96*sqrt(highest_var) / sqrt(highest_picktime);
		double confidence99 = 2.58*sqrt(highest_var) / sqrt(highest_picktime);
		cout<< "\nBEST GUESS: " << possiblePwd+highest_char << ", with time: " << highest_mean << " variance: " << highest_var <<endl;
		cout<<"\n95\% confidence interval: " <<highest_mean << " +- " <<confidence95 <<"\n99\% confidence interval: " <<highest_mean << " +- " <<confidence99<< endl; 
		cout<<"next best time: " << nexthighest_mean <<endl;
		
		if(highest_mean/nexthighest_mean < 1.01 ||confidence99/highest_mean > 0.02 || (highest_mean - confidence99) < nexthighest_mean){
			cout<< "You are on the wrong track, try a shorter guess" <<endl;
		}
		return 0;
		
	}
	
    return 0;
}

static __inline__ uint64_t rdtsc()
{
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

bool password_ok (const string &pwd)
{
	return strcmp(pwd.c_str(), "mypassword") == 0;
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
                    // istringstream buf(msg);
                    // string msg_token;
                    // buf >> msg_token;
                    return msg;  // msg_token;
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
