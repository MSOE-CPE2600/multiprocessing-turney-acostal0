CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=-ljpeg
SOURCES= mandel.c jpegrw.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mandel
MOVIE_EXECUTABLE=mandelmovie
MOVIE_SOURCE=mandelmovie.c

all: $(SOURCES) $(EXECUTABLE) $(MOVIE_EXECUTABLE)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(MOVIE_EXECUTABLE): $(MOVIE_SOURCE)
	$(CC) $(MOVIE_SOURCE) -o $(MOVIE_EXECUTABLE)

.c.o: 
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(MOVIE_EXECUTABLE) *.d
