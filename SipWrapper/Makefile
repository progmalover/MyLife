# Old style makefile
# ------------------
# In old days, we don't have autoreconf/automake/libtool tools, so we have to
# use old style makefile here.
# Our target is to compile SipWrapper and reSIProcate into C++ style library.
# So this makefile process reSIProcate first, then process src with resip, at
# last process libsipsdk.a .
# Notice we have to process XX.a, because product need static library.
#
# Makefile for Sip SDK library
# 
include ./Make.defines
#include ./util/Makefile

SIP_STACK_DIR=./sipstack
RESIP_STACK_DIR=$(SIP_STACK_DIR)/resiprocate-1.7

UTIL_DIR=./util
UTIL_SRC=$(UTIL_DIR)/src

BIN_DIR=./bin
LIB_DIR=./lib
LIB_H=-I./util/h -I$(RESIP_STACK_DIR)
INCLUDE=-I./h -I./src $(LIB_H)

LD_FLAGS=-L$(LIB_DIR) -lresip -lrutil -lares $(LDFLAGS_GLOBAL)

TARGET=./lib/libsipsdk.a
#TEST=./bin/testApp
TEST=

OBJS =  \
	src/AttributeImpl.o \
	src/ConnectionImpl.o \
	src/MediaDescriptionImpl.o \
	src/SessionDescriptionImpl.o\
	src/SipCallImpl.o\
	src/SipMessageImpl.o\
	src/SipProviderImpl.o\
	src/ResipStackAdapterImpl.o\
	src/SipCallMgr.o\
	src/SessionTimer.o\
	src/SipTrace.o \
	src/SipUtil.o

all: stack $(TARGET) testApp

stack:
	@echo "Build resip stack library ..."
	@(cd $(RESIP_STACK_DIR) && make resiprocate) || exit 1;
	@mkdir -p lib
	@(cp -f $(RESIP_STACK_DIR)/resip/stack/obj.debug.Linux.i686/libresip.a lib/libresip.a)
	@(cp -f $(RESIP_STACK_DIR)/rutil/obj.debug.Linux.i686/librutil.a  lib/librutil.a)
	@(cp -f $(RESIP_STACK_DIR)/contrib/ares-build.Linux.i686/libares.a  lib/libares.a)
	@echo "Complete."

$(UTIL_SRC)/%.o: $(UTIL_SRC)/%.cpp
	$(CXX) $(CXX_FLAGS)  -c $< -o $@ -I$(UTIL_DIR)/h
	
$(TARGET): $(OBJS)
	ar -rs $@ $^

src/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS)  -c $< -o $@ $(INCLUDE)

test/%.o: test/%.cpp
	$(CXX) $(CXX_FLAGS)  -c $< -o $@ $(INCLUDE)

testApp: $(TARGET) test/testApp.o 
	@if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR); fi
	$(CXX) $(CXX_FLAGS) -o $(BIN_DIR)/$@ test/testApp.o $(INCLUDE) $(TARGET) $(LD_FLAGS) 

clean:
	@(cd $(RESIP_STACK_DIR) && make cleanall) || exit 1;
	rm -f lib/*.a
	rm -f src/*.o
	rm -f test/*.o
	rm -f $(TARGET)
	rm -rf $(BIN_DIR)
	rm -rf $(UTIL_DIR)/src/*.o
