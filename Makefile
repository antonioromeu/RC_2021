CC = g++
CFLAGS = -g -Wall -Wextra -std=c++11
#HOST = $(shell hostname -I)
#IP = $(HOST) | cut -d' ' -f2
IP = 193.136.128.103

all: pd user as

pd:
	$(CC) $(CFLAGS) pd.cpp -o pd
	./pd $(IP) -n tejo.tecnico.ulisboa.pt -p 58011

as:
	$(CC) $(CFLAGS) as.cpp -o as
	./as

user:
	$(CC) $(CFLAGS) user.cpp -o user
	./user -n tejo.tecnico.ulisboa.pt -p 58011 -m tejo.tecnico.ulisboa.pt -q 59000

clean:
	@echo Cleaning...
	rm -f pd user as
	rm -r USERS
	rm -r *.dSYM

