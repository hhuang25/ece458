Compile: make
This compiles the client and server for part 1, as well as pwd_client for part 2.

Testing part 1
	run server: ./server.out [challenge-length]
	run client: ./client.out
The [challenge-length] is optional. By default it is 2 characters.
The server will print out that it started and prints out the number of challenge characters (before hex encoding). The server and client will both print the hex characters that are sent and received.

Testing part 2
	run: ./pwd_client.out <password_guess>
<password_guess> is the best guess of the password. It may be left empty.
It will output the statistics for each character (mean, variance, N).
The output will also give you a summary of what it thinks the password begins with (best guess) and how confident the program is.
For convenience, it will also print the mean of the next longest running time.
Keep running ./pwd_client.out <password_guess> where <password_guess> is the output of the "best guess" of the program until the password is cracked.

