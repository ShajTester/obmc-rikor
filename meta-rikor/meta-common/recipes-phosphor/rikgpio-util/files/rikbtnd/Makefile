


all: rikbtnd

CFLAGS += -Wall -Werror -pthread -lgpio

rikgpio: rikbtnd.c 
	$(CC) $(CFLAGS) -pthread -lgpio -std=c99 -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o rikbtnd

