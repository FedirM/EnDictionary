
CFLAGS=-g -Wall

all: main.c
	echo "Start building...."
	c99 $(CFLAGS) main.c -o envoca `mysql_config --cflags --libs && pkg-config --cflags --libs gtk+-3.0`
	echo "Run: ./envoca"
clean:
	rm envoca
