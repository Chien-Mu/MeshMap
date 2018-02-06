OBJS = main.o meshpos.o
PROG = MeshMap

${PROG}: ${OBJS}
	gcc ${OBJS} -o $@

clean:
	rm ${OBJS} ${PROG}

