PROG			= sine
CC				= gcc
CFLAGS		= -O2 

OBJS=$(PROG).o

all: $(PROG)

.PHONY: all

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(PROG) $(OBJS)

