#!/bin/bash

PBC_PREFIX=/home/joseph/bbt/dpvs/lib/install

CC=gcc
CFLAGS=-O2 -Wall -I$(PBC_PREFIX)/include
LDFLAGS=-Wl,-rpath,$(PBC_PREFIX)/lib -L$(PBC_PREFIX)/lib -lgmp -lpbc

OBJ_DIR=obj
BIN_DIR=bin
SRC_DIR=src

TEST_IPE_OBJS=\
	$(OBJ_DIR)/test_ipe.o \
	$(OBJ_DIR)/dpvs.o \
	$(OBJ_DIR)/aahipe.o \
	$(OBJ_DIR)/util.o

TEST_CHEN_OBJS=\
	$(OBJ_DIR)/test_chen.o \
	$(OBJ_DIR)/ff_mat.o \
	$(OBJ_DIR)/group_mat.o \
	$(OBJ_DIR)/chen_ipe.o \
	$(OBJ_DIR)/util.o

SPEED_TEST_OBJS=\
	$(OBJ_DIR)/speed_test.o \
	$(OBJ_DIR)/ff_mat.o \
	$(OBJ_DIR)/group_mat.o \
	$(OBJ_DIR)/chen_ipe.o \
	$(OBJ_DIR)/util.o \
	$(OBJ_DIR)/bbt.o


TEST_IPE_BIN=$(BIN_DIR)/test_ipe
TEST_CHEN_BIN=$(BIN_DIR)/test_chen
SPEED_TEST_BIN=$(BIN_DIR)/speed_test

all: $(TEST_IPE_BIN) $(SPEED_TEST_BIN) $(TEST_CHEN_BIN) rebuild_cscope

$(TEST_IPE_BIN): $(TEST_IPE_OBJS) $(BIN_DIR) 
	$(CC) $(CFLAGS) -o $(TEST_IPE_BIN) $(TEST_IPE_OBJS) $(LDFLAGS)

$(TEST_CHEN_BIN): $(TEST_CHEN_OBJS) $(BIN_DIR) 
	$(CC) $(CFLAGS) -o $(TEST_CHEN_BIN) $(TEST_CHEN_OBJS) $(LDFLAGS)

$(SPEED_TEST_BIN): $(SPEED_TEST_OBJS) $(BIN_DIR) 
	$(CC) $(CFLAGS) -o $(SPEED_TEST_BIN) $(SPEED_TEST_OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)/*.o $(TEST_IPE_BIN)

rebuild_cscope:
	cscope -R -b
	
	



