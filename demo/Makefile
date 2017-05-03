BIN=.
CC = g++

#FLAGS= -g -O0 -Wall -Wno-unused-result -std=gnu99 -pedantic
FLAGS= -O3  -Wall -Wno-unused-result  -pedantic -Wno-write-strings -fsanitize=address
#-std=gnu99
FLLIB    = -L../lib -lFL -lm
FLFLAGS  = -I../include

libift:
	cd ../; $(MAKE) ; cd -\


$@.cpp: libft $@.cpp
	make
	$(CC) $(FLAGS) $@.cpp -o $(BIN)/$@ $(FLFLAGS) $(FLLIB) ; \

#Removing everything without an extension
clean:	
	find . -type f -executable -exec rm '{}' \;

cleanImages:
	rm *.ppm *.pgm *.gif

cleanText:
	rm *.txt
