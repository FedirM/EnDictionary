
CFLAGS=-g -Wall
#CFLAGS=-g

all: main.c
	echo "Start building...."
	c99 $(CFLAGS) main.c -o envoca `mysql_config --cflags --libs && pkg-config --cflags --libs gtk+-3.0`
clean:
	rm envoca
