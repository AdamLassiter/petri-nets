# c compiler:
CC       = gcc
# compiler flags:
CFLAGS   = -g -Wall -fbounds-check
# linker flags
LDFLAGS  = 
# obj file includes
OBJFILES = llist.o rbtree.o ndarray.o formula.o petri_net.o
# build target executable:
TARGET   = petri_net

# default to build target
all: $(TARGET)
# build dependency of target on objects
$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

# dependencies per object
llist.o: llist.h
rbtree.o: rbtree.h
ndarray.o: ndarray.h
formula.o: formula.h llist.h
petri_net.o: petri_net.h formula.h ndarray.h llist.h rbtree.h

# remove products of make
.PHONY: clean
clean:
	$(RM) $(OBJFILES) $(TARGET) core
