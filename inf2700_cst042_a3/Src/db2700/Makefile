CC = gcc
INCLUDES = 
LIBS = 
CFLAGS = -g -Wall

TARGET = front test
OBJ_DIR = ./_obj
DOC_DIR = ./doc
HEADERS = pmsg.h pager.h schema.h interpreter.h test_data_gen.h testpager.h testschema.h
OBJS = \
	$(OBJ_DIR)/pmsg.o \
	$(OBJ_DIR)/pager.o \
	$(OBJ_DIR)/schema.o \
	$(OBJ_DIR)/interpreter.o
TEST_OBJS = \
	$(OBJ_DIR)/test_data_gen.o \
	$(OBJ_DIR)/testpager.o \
	$(OBJ_DIR)/testschema.o

# Main target
all: $(TARGET)

front: $(OBJS) front.c
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) front.c -o run_$@

test: $(OBJS) $(TEST_OBJS) testmain.c
	$(CC) $(CFLAGS) $(OBJS) $(TEST_OBJS) $(LIBS) testmain.c -o run_$@

$(OBJ_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

.PHONY: doc clean cleandoc cleantest
doc:
	doxygen Doxyfile

cleanall: clean cleandoc cleantest

clean:
	rm -f run_front run_test
	rm -f $(OBJS) $(TEST_OBJS)

cleandoc:
	rm -fr $(DOC_DIR)/html

cleantest:
	rm -f ./tests/testdb/*
	rm -f ./tests/testfront/*
