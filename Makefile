CFLAGS= -Wall -Werror
DEBUG_FLAGS= -g -fsanitize=address -O0
RELEASE_FLAGS= -O2


debug : CFLAGS+=$(DEBUG_FLAGS)
debug : build
	echo "debug"


run : build
	./mygrep -E "a|a"

build : mygrep.c
	gcc $(CFLAGS) mygrep.c -o mygrep

release : CFLAGS+=$(RELEASE_FLAGS)
release : build
	echo "release"

test : debug
	./mygrep -E "(a|b)*ab(a|b)*"