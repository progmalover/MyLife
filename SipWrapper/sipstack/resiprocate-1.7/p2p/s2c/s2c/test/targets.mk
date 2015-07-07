#
#    targets.mk
#
#    $Source: /cvs/hfcvs/server/Atlas2/sipSDK/sipstack/resiprocate-1.7/p2p/s2c/s2c/test/targets.mk,v $
#    $Revision: 1.1.1.1 $
#    $Date: 2011/07/05 04:45:11 $
#
#    Copyright (C) 2008, Network Resonance, Inc.
#    All Rights Reserved.
#

# S2C_S2C_TEST_ARCHIVE:
#    source files to compile to object and to place in a library archive
#
# S2C_S2C_TEST_ARCHIVE_NAME:
#    the library name to use to archive the object files
#
# S2C_S2C_TEST_ARCHIVE_TYPE:
#    either 'static', 'dynamic', or 'jni' (the default is 'static')
#
# S2C_S2C_TEST_AUTOGENERATED:
#    whether this targets.mk is autogenerated (either 'true' or 'false')
#
# S2C_S2C_TEST_COMPILE:
#    source files to compile to object
#
# S2C_S2C_TEST_COPY:
#    files to copy to the build directory
#
# S2C_S2C_TEST_GLOBAL_CFLAGS:
#    preprocessor and compile-time flags specific to compiling
#    everything
#
# S2C_S2C_TEST_GLOBAL_LDFLAGS:
#    link-time flags specific to linking everything
#
# S2C_S2C_TEST_LINK:
#    source files compile and link
#
# S2C_S2C_TEST_LOCAL_CFLAGS:
#    preprocessor and compile-time flags specific to compiling only
#    the files in this module directory
#
# S2C_S2C_TEST_LOCAL_LDFLAGS:
#    link-time flags specific to linking only the files in
#    this module directory
#
# S2C_S2C_TEST_PREFIX:
#    defines the module name, which also serves as the
#    prefix for all the variable names defined in this file
#
# S2C_S2C_TEST_SUBDIRS:
#    subdirectories containing additional targets.mk files
#
S2C_S2C_TEST_ARCHIVE              = auto.s2c enum.s2c select.s2c struct.s2c
S2C_S2C_TEST_ARCHIVE_NAME         = s2ctest
S2C_S2C_TEST_ARCHIVE_TYPE         =
S2C_S2C_TEST_AUTOGENERATED        = true
S2C_S2C_TEST_COMPILE              =
S2C_S2C_TEST_COPY                 =
S2C_S2C_TEST_GLOBAL_CFLAGS        =
S2C_S2C_TEST_GLOBAL_LDFLAGS       =
S2C_S2C_TEST_LINK                 = test_s2c.cxx
S2C_S2C_TEST_LOCAL_CFLAGS         = -I$(RESIPROCATE_SRCDIR) \
                                    -I$(RESIPROCATE_SRCDIR)/p2p/
S2C_S2C_TEST_LOCAL_LDFLAGS        = -L$(RESIPROCATE_SRCDIR)p2p/obj.$(RESIPROCATE_DEBUG)$(RESIPROCATE_PLATFORM) \
                                    -L$(RESIPROCATE_SRCDIR)rutil/obj.$(RESIPROCATE_DEBUG)$(RESIPROCATE_PLATFORM) \
                                    -L. -lp2p -lrutil -ls2ctest s2c_native.o
S2C_S2C_TEST_PREFIX               = S2C_S2C_TEST
S2C_S2C_TEST_SUBDIRS              =



#
#    AUTOMATICALLY-GENERATED SECTION
#



#
#    LOCAL ENVIRONMENT
#
S2C_S2C_TEST_BUILD                = auto.$(OBJSUFFIX) auto.d enum.$(OBJSUFFIX) \
                                    enum.d libs2ctest.$(ARSUFFIX) \
                                    select.$(OBJSUFFIX) select.d \
                                    struct.$(OBJSUFFIX) struct.d test_s2c \
                                    test_s2c.$(OBJSUFFIX) test_s2c.d



#
#    GLOBAL ENVIRONMENT
#
GLOBAL_BUILD                     += $(S2C_S2C_TEST_BUILD)
GLOBAL_CFLAGS                    += $(S2C_S2C_TEST_GLOBAL_CFLAGS) \
                                 -I$(S2C_S2C_TEST_SRCDIR)
GLOBAL_LDFLAGS                   += $(S2C_S2C_TEST_GLOBAL_LDFLAGS)



#
#    GENERIC DEPENDENCIES
#
all:                                $(S2C_S2C_TEST_BUILD)
depend:                             auto.d
depend:                             enum.d
depend:                             select.d
depend:                             struct.d
depend:                             test_s2c.d



#
#    BUILD DEPENDENCIES
#

auto.$(OBJSUFFIX): $(S2C_S2C_TEST_SRCDIR)auto.s2c auto.d
	$(COMPILE.s2c) $@ $< $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

auto.d: $(S2C_S2C_TEST_SRCDIR)auto.s2c
	$(COMPILE.s2c) $@ $< -MM -MG $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

enum.$(OBJSUFFIX): $(S2C_S2C_TEST_SRCDIR)enum.s2c enum.d
	$(COMPILE.s2c) $@ $< $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

enum.d: $(S2C_S2C_TEST_SRCDIR)enum.s2c
	$(COMPILE.s2c) $@ $< -MM -MG $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

select.$(OBJSUFFIX): $(S2C_S2C_TEST_SRCDIR)select.s2c select.d
	$(COMPILE.s2c) $@ $< $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

select.d: $(S2C_S2C_TEST_SRCDIR)select.s2c
	$(COMPILE.s2c) $@ $< -MM -MG $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

struct.$(OBJSUFFIX): $(S2C_S2C_TEST_SRCDIR)struct.s2c struct.d
	$(COMPILE.s2c) $@ $< $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

struct.d: $(S2C_S2C_TEST_SRCDIR)struct.s2c
	$(COMPILE.s2c) $@ $< -MM -MG $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

test_s2c.$(OBJSUFFIX): $(S2C_S2C_TEST_SRCDIR)test_s2c.cxx test_s2c.d
	$(COMPILE.cxx) $@ $< $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

test_s2c.d: $(S2C_S2C_TEST_SRCDIR)test_s2c.cxx
	$(COMPILE.cxx) $@ $< -MM -MG $(S2C_S2C_TEST_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

libs2ctest.$(ARSUFFIX): auto.$(OBJSUFFIX)

libs2ctest.$(ARSUFFIX): enum.$(OBJSUFFIX)

libs2ctest.$(ARSUFFIX): select.$(OBJSUFFIX)

libs2ctest.$(ARSUFFIX): struct.$(OBJSUFFIX)

libs2ctest.$(ARSUFFIX): 
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

test_s2c: test_s2c.$(OBJSUFFIX) $(GLOBAL_LIBNAME)
	$(LINK.cxx) $@ test_s2c.$(OBJSUFFIX) $(S2C_S2C_TEST_LOCAL_LDFLAGS) $(GLOBAL_LDFLAGS)
