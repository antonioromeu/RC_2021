CC = g++
CFLAGS = -g -Wall -Wextra -std=c++11
IP = 193.136.128.104

all: pd user

pd:
	$(CC) $(CFLAGS) pd.cpp -o pd
	./pd $(IP) -n tejo.tecnico.ulisboa.pt -p 58011

user:
	$(CC) $(CFLAGS) user.cpp -o user
	./user -n tejo.tecnico.ulisboa.pt -p 58011 -m tejo.tecnico.ulisboa.pt -q 59000

clean:
	@echo Cleaning...
	rm -f pd user