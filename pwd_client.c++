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

int main(int argc, char *argv[])
{
	string possiblePwd = "";
	if(argc == 2){
		possiblePwd = argv[1];
	}
	srand(time(NULL));
	bool finished = false;
	int letters = 0;
	if((finished = password_ok (possiblePwd)) == true) cout<<"password correct: " <<possiblePwd <<endl;
	//static const char alphabet[] = "ghijnoptuvwabcxyzdefqrsklm";
	//static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
	string alphabet = "abcdefghijklmnopqrstuvwxyz";
	while(!finished){
		uint64_t timevalues [26] = {};
		uint64_t squarevalues [26] = {};
		int times_picked [26] = {};
		//destroy possible caching
		for(int i = 0; i < 100000; i++){
			int chosen_char = rand()%26;
			random_shuffle(alphabet.begin(),alphabet.end());
			const char &current = alphabet.at(chosen_char);
			const string &pwd_attempt = possiblePwd + current;
            password_ok (pwd_attempt);
            
		}
		srand(time(NULL));
		for(int i = 0; i < 2600000; i++){
            int chosen_char = rand()%26;
            random_shuffle(alphabet.begin(),alphabet.end());
			char current = alphabet.at(chosen_char);
			
			const string &pwd_attempt = possiblePwd + current;
			uint64_t start = rdtsc();
			finished = password_ok (pwd_attempt);
			uint64_t end = rdtsc();
			if(finished){
				cout<<"password guessed correct: " <<pwd_attempt <<endl;
				return 0;
			}
			uint64_t difference = end - start;
			//cout<< alphabet[chosen_char]<< ": time of " <<difference <<endl; 
			timevalues[((char)alphabet.at(chosen_char))-97] += difference;
			squarevalues[((char)alphabet.at(chosen_char))-97] += (difference*difference);
			times_picked[((char)alphabet.at(chosen_char))-97] = times_picked[((char)alphabet.at(chosen_char))-97] + 1;
			
		}
		//initialize to "a"
		double highest_mean = 0.0;
		double highest_var = 0.0;
		char highest_char = 'a';
		double nexthighest_mean = 0.0;
		int highest_picktime = 0;
		alphabet = "abcdefghijklmnopqrstuvwxyz";
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
		cout<<"next longest time: " << nexthighest_mean <<endl;
		
		if(highest_mean/nexthighest_mean < 1.01 ||confidence99/highest_mean > 0.03 || (highest_mean - confidence99) < nexthighest_mean){
			cout<< "Not confident about attempted password, try a shorter guess?" <<endl;
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
	return pwd == "mypassword";
    //return strcmp(pwd.c_str(), "mypassword") == 0;
}

