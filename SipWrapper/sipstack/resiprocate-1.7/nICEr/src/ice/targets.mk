# Copyright (c) 2007, Adobe Systems, Incorporated
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 
# * Neither the name of Adobe Systems, Network Resonance nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# ICE_ICE_ARCHIVE:
#    source files to compile to object and to place in a library archive
#
# ICE_ICE_ARCHIVE_NAME:
#    the library name to use to archive the object files
#
# ICE_ICE_ARCHIVE_TYPE:
#    either 'static', 'dynamic', or 'jni' (the default is 'static')
#
# ICE_ICE_AUTOGENERATED:
#    whether this targets.mk is autogenerated (either 'true' or 'false')
#
# ICE_ICE_COMPILE:
#    source files to compile to object
#
# ICE_ICE_COPY:
#    files to copy to the build directory
#
# ICE_ICE_GLOBAL_CFLAGS:
#    preprocessor and compile-time flags specific to compiling
#    everything
#
# ICE_ICE_GLOBAL_LDFLAGS:
#    link-time flags specific to linking everything
#
# ICE_ICE_LINK:
#    source files compile and link
#
# ICE_ICE_LOCAL_CFLAGS:
#    preprocessor and compile-time flags specific to compiling only
#    the files in this module directory
#
# ICE_ICE_LOCAL_LDFLAGS:
#    link-time flags specific to linking only the files in
#    this module directory
#
# ICE_ICE_PREFIX:
#    defines the module name, which also serves as the
#    prefix for all the variable names defined in this file
#
# ICE_ICE_SUBDIRS:
#    subdirectories containing additional targets.mk files
#
ICE_ICE_ARCHIVE                   = ice_candidate.c ice_candidate_pair.c \
                                    ice_codeword.c ice_component.c ice_ctx.c \
                                    ice_media_stream.c ice_parser.c \
                                    ice_peer_ctx.c ice_socket.c
ICE_ICE_ARCHIVE_NAME              =
ICE_ICE_ARCHIVE_TYPE              =
ICE_ICE_AUTOGENERATED             = true
ICE_ICE_COMPILE                   =
ICE_ICE_COPY                      =
ICE_ICE_GLOBAL_CFLAGS             =
ICE_ICE_GLOBAL_LDFLAGS            =
ICE_ICE_LINK                      =
ICE_ICE_LOCAL_CFLAGS              = -DUSE_TURN
ICE_ICE_LOCAL_LDFLAGS             =
ICE_ICE_PREFIX                    = ICE_ICE
ICE_ICE_SUBDIRS                   =



#
#    AUTOMATICALLY-GENERATED SECTION
#



#
#    LOCAL ENVIRONMENT
#
ICE_ICE_BUILD                     = ice_candidate.$(OBJSUFFIX) ice_candidate.d \
                                    ice_candidate_pair.$(OBJSUFFIX) \
                                    ice_candidate_pair.d \
                                    ice_codeword.$(OBJSUFFIX) ice_codeword.d \
                                    ice_component.$(OBJSUFFIX) ice_component.d \
                                    ice_ctx.$(OBJSUFFIX) ice_ctx.d \
                                    ice_media_stream.$(OBJSUFFIX) \
                                    ice_media_stream.d ice_parser.$(OBJSUFFIX) \
                                    ice_parser.d ice_peer_ctx.$(OBJSUFFIX) \
                                    ice_peer_ctx.d ice_socket.$(OBJSUFFIX) \
                                    ice_socket.d



#
#    GLOBAL ENVIRONMENT
#
GLOBAL_BUILD                     += $(ICE_ICE_BUILD)
GLOBAL_CFLAGS                    += $(ICE_ICE_GLOBAL_CFLAGS) -I$(ICE_ICE_SRCDIR)
GLOBAL_LDFLAGS                   += $(ICE_ICE_GLOBAL_LDFLAGS)



#
#    GENERIC DEPENDENCIES
#
all:                                $(ICE_ICE_BUILD)
depend:                             ice_candidate.d
depend:                             ice_candidate_pair.d
depend:                             ice_codeword.d
depend:                             ice_component.d
depend:                             ice_ctx.d
depend:                             ice_media_stream.d
depend:                             ice_parser.d
depend:                             ice_peer_ctx.d
depend:                             ice_socket.d



#
#    BUILD DEPENDENCIES
#

ice_candidate.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_candidate.c ice_candidate.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_candidate.d: $(ICE_ICE_SRCDIR)ice_candidate.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_candidate_pair.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_candidate_pair.c ice_candidate_pair.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_candidate_pair.d: $(ICE_ICE_SRCDIR)ice_candidate_pair.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_codeword.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_codeword.c ice_codeword.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_codeword.d: $(ICE_ICE_SRCDIR)ice_codeword.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_component.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_component.c ice_component.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_component.d: $(ICE_ICE_SRCDIR)ice_component.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_ctx.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_ctx.c ice_ctx.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_ctx.d: $(ICE_ICE_SRCDIR)ice_ctx.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_media_stream.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_media_stream.c ice_media_stream.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_media_stream.d: $(ICE_ICE_SRCDIR)ice_media_stream.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_parser.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_parser.c ice_parser.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_parser.d: $(ICE_ICE_SRCDIR)ice_parser.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_peer_ctx.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_peer_ctx.c ice_peer_ctx.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_peer_ctx.d: $(ICE_ICE_SRCDIR)ice_peer_ctx.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_socket.$(OBJSUFFIX): $(ICE_ICE_SRCDIR)ice_socket.c ice_socket.d
	$(COMPILE.c) $@ $< $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

ice_socket.d: $(ICE_ICE_SRCDIR)ice_socket.c
	$(COMPILE.c) $@ $< -MM -MG $(ICE_ICE_LOCAL_CFLAGS) $(GLOBAL_CFLAGS)

$(GLOBAL_LIBNAME): ice_candidate.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_candidate_pair.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_codeword.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_component.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_ctx.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_media_stream.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_parser.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_peer_ctx.$(OBJSUFFIX)

$(GLOBAL_LIBNAME): ice_socket.$(OBJSUFFIX)
