CC ?= $(CROSS_COMPILE)gcc

CFLAGS += -Wall -I../ta/include -I./include
CFLAGS += -I$(OPTEE_CLIENT_EXPORT)/include
CFLAGS += -fstack-protector-strong
LDADD += -lteec -L$(OPTEE_CLIENT_EXPORT)/lib

SRCS = main.c
OBJS = $(patsubst %.c,$(O)/%.o,$(SRCS))

#BINARY = optee_example_hello_world
BINARY = optee_logger

.PHONY: all install
all: $(BINARY) install

$(BINARY): $(OBJS)
	$(CC) -o $(O)/$@ $< $(LDADD)

$(O)/%.o: %.c
	mkdir -p $(O)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(BINARY)
	mkdir -p $(OPTEE_CLIENT_EXPORT)/sbin
	cp $(O)/$(BINARY) $(OPTEE_CLIENT_EXPORT)/sbin

.PHONY: clean
clean:
	rm -f $(OBJS) $(O)/$(BINARY) $(OPTEE_CLIENT_EXPORT)/sbin/$(BINARY)
