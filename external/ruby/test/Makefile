EMBPATH=../ruby_build_output
LDFLAGS=-L${EMBPATH}/lib -lruby-static -ldl -lobjc -lreadline -lncurses \
                         -ltermcap -lssl -lcrypto -liconv -lz 
CFLAGS=-I$(EMBPATH)/include -g -Wall

test: test.o
	$(CC) -o test test.o $(LDFLAGS)

clean:
	rm -f test.o *~

distclean: clean
	rm -f test
