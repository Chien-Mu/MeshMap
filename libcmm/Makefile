CFLAGS := -fPIC
#LDFLAGS := -L. -lcmm
LDFLAGS :=

LIBCMM:= libcmm.so
CLIENT:= cmmcli
#OBJS := main.o socket.o packet.o mesh.o utils.o platform.o JsonConvert.o
OBJS_CLI := client.o libcmm.o
OBJS_LIBCMM := libcmm.o


all: $(LIBCMM)

$(CLIENT): $(OBJS_CLI)
	$(CC) -o $@ $(OBJS_CLI) $(LDFLAGS)

$(LIBCMM): $(OBJS_LIBCMM)
	$(CC) -shared -o $@ $(OBJS_LIBCMM)
	#$(CC) -c -fPIC libcmm.c $(CFLAGS) 

romfs:
	$(ROMFSINST) /lib/$(LIBCMM)
	#$(ROMFSINST) /bin/$(CLIENT)


clean:
	rm -rf $(LIBCMM) $(OBJS_LIBCMM) $(CLIENT) $(OBJS_CLI) 
