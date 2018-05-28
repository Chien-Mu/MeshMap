CFLAGS := -Wall -Werror -s -O2 
CFLAGS += -I$(ROOTDIR)/lib/libcmm

LDFLAGS := -L$(ROOTDIR)/lib/libcmm -lcmm -lm

MAP:= cmmmap
TEST:= cmmtest
OBJS_MAP := meshpos.o main.o
OBJS_test := client.o


all: $(MAP)

$(MAP): $(OBJS_MAP)
	$(CC) -o $(MAP) $(OBJS_MAP) $(LDFLAGS)

$(TEST): $(OBJS_TEST)
	$(CC) -o $(TEST) $(OBJS_TEST) $(LDFLAGS)

romfs:
	$(ROMFSINST) /bin/$(MAP)
#	$(ROMFSINST) /bin/$(TEST)

clean:
	rm -rf $(OBJS_MAP) $(OBJS_TEST) $(MAP) $(TEST)
