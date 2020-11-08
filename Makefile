CC = g++
CFLAGS = -g -Wall -Wextra -std=c++11
IP = 192.168.1.76

all: pd user as

pd:
	$(CC) $(CFLAGS) pd.cpp -o pd
	./pd $(IP)

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