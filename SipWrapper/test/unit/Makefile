# Last updated by Sam Chen, 2011-06-27 11:19:06
# Makefile for Sip SDK library
# 

USER_DEPTH = ../..

SYSVER	 := $(shell /bin/rpm -qf /etc/redhat-release |cut -d- -f1,3)

ifeq ($(SYSVER), centos-4)
	PLATFORM  := CentOS44
endif

ifeq ($(SYSVER), centos-5)
    PLATFORM  := CentOS53
endif

CXX = c++
CXX_FLAGS = -g -D__EXTENSIONS__  -D_UNIFY_BASELIB  -DUNIX -DLINUX -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_FILE_OFFSET_BITS=64  -Wreturn-type -D_REENTRANT -D__BUILD_MT_SERVER__ -D__STRIP_ASSERT__

ifeq ($(COVERAGE), 1)
    CXX_FLAGS += -fprofile-arcs -ftest-coverage
endif

BIN_DIR=$(USER_DEPTH)/bin
LIB_DIR=$(USER_DEPTH)/lib
LIB_H=-I$(USER_DEPTH)/sipstack/resiprocate-1.7
INCLUDE=-I$(USER_DEPTH)/h -I$(USER_DEPTH)/src $(LIB_H)

SIP_STACK_DIR=./sipstack

LD_FLAGS=-L$(LIB_DIR) -lsipsdk\
-lpthread -lresip -lrutil -lares\
-lrt -lexpat -lssl -lcrypto -lkrb5 -lk5crypto -lresolv -lcom_err -lz

TEST_OBJS=\
	AttributeImplTest.o \
	ConnectionImplTest.o \
	MediaDescriptionImplTest.o \
	SessionDescriptionImplTest.o \
	SipCallImplTest.o \
	SipMessageImplTest.o \
	SipProviderImplTest.o \
	ResipStackAdapterImplTest.o \
	SipCallMgrTest.o \
	SessionTimerTest.o \
	Test.o
	
unittest: $(TEST_OBJS)
#	cd ../.. && make
	$(CXX) $(CXX_FLAGS) -o $@ $^ $(LD_FLAGS) /usr/local/lib/libgtest.a /usr/local/lib/libgmock.a
	
%.o: %.cpp
	$(CXX) $(CXX_FLAGS)  -c $< -o $@ $(INCLUDE)

clean:
	rm -rf *.o unittest
	rm -rf $(USER_DEPTH)/src/*.gcno
