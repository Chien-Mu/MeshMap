OBJS = main.o meshpos.o
PROG = meshMap

${PROG}: ${OBJS}
	gcc ${OBJS} -o $@

clean:
	rm ${OBJS} ${PROG}

