all: 320sh

320sh: 320sh.c
	gcc -Wall -Werror -g -o 320sh 320sh.c

debug: 320sh.c
	gcc -Wall -Werror -DDEBUG -g -o 320sh 320sh.c

clean:
	rm -f *~ *.o 320sh
