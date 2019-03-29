hc : hc.c
	gcc -Wall -Wextra -o hc hc.c

.PHONY : clean

clean :
	rm -f hc
