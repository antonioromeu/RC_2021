CC = g++
CFLAGS = -Wall -std=c++11
IP = 192.168.2.46

all: pd user

pd:
	$(CC) $(CFLAGS) pd.cpp -o pd
	./pd $(IP) -n tejo.tecnico.ulisboa.pt -p 58011

user:
	$(CC) $(CFLAGS) user.cpp -o user
	./user -n tejo.tecnico.ulisboa.pt -p 58011

clean:
	@echo Cleaning...
	rm -f pd user

# run: pd
# 	./pd $(IP) -n tejo.tecnico.ulisboa.pt -p 58011

# run: user
# 	./user -n tejo.tecnico.ulisboa.pt -p 58011
