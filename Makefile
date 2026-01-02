CFLAGS= -Wall -Werror
DEBUG_FLAGS= -g -fsanitize=address -O0 -DDEBUG
RELEASE_FLAGS= -Ofast


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

lea : CFLAGS+=$(DEBUG_FLAGS)
lea : mygrep_lea.c
	gcc $(CFLAGS) mygrep_lea.c -o mygrep_lea