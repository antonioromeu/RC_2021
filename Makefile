CC = g++
CFLAGS = -g -Wall -Wextra -std=c++11
# HOST = $(shell hostname -I)
# IP = $(HOST) | cut -d' ' -f2
# IP = 193.136.128.103
# ./AS [-p ASport] [-v]
# ./FS [-q FSport] [-n ASIP] [-p ASport] [-v]
# ./pd PDIP [-d PDport] [-n ASIP] [-p ASport]
# ./user [-n ASIP] [-p ASport] [-m FSIP] [-q FSport]
all: pd user as

pd:
	$(CC) $(CFLAGS) pd.cpp -o pd

as:
	$(CC) $(CFLAGS) as.cpp -o as

fs:
	$(CC) $(CFLAGS) fs.cpp -o fs

user:
	$(CC) $(CFLAGS) user.cpp -o user

clean:
	@echo Cleaning...
	rm -f pd user as fs
	rm -r USERS
	rm -r *.dSYM
