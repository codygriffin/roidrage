#-------------------------------------------------------------------------------
#
# roidrage::Makefile
#
#  The MIT License (MIT)
#
#  Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
# 
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#

#CXX     := /usr/local/bin/g++-4.9

all::
.PHONY: all

BIN_DIR := ./bin
SRC_DIR := ./src
INC_DIR := ./inc
TGT_DIR := ./tgt

#XXX Move to targets.mak
# No cross-compilation... yet
OS          := $(shell uname -s)
HOST        := $(OS)-$(shell uname -m)
TGT_DIR     := ./tgt/$(HOST)
DB 	:= lldb

include mak/helpers.mak
include mak/sources.mak
#include mak/boost.mak
include mak/application.mak
include mak/staticlib.mak
include mak/sharedlib.mak

ROIDRAGE_CXXFLAGS    := -std=gnu++0x -O0 -g -DGLFW_INCLUDE_GLCOREARB -DGL_ERROR_CHECK -DGLM_FORCE_RADIANS
ROIDRAGE_CXXFLAGS    += -Iinc/ -Iboson/inc -Ikernel/inc -Igl/inc -Iglm/ -Iglm/gtx -Iglm/gtc
ROIDRAGE_OBJCXXFLAGS := -std=gnu++0x -O0 -g -x objective-c++
ROIDRAGE_OBJCXXFLAGS += -Iinc/ -Iboson/inc -Ikernel/inc -Igl/inc -Iglm/ -Iglm/gtx -Iglm/gtc
ROIDRAGE_LDFLAGS     := -lglfw3 
ROIDRAGE_LDFLAGS     += -framework OpenGL -framework Cocoa -framework CoreVideo -framework IOKit
ROIDRAGE_SRCS        := $(shell find gl     -name *.cpp) \
												$(shell find kernel -name *.cpp) \
												$(shell find boson  -name *.cpp) 

# really dumb HACK
OBJCPP_SRCS          := src/AssetManager.mm

#-------------------------------------------------------------------------------

ALPHA_CXXFLAGS    := $(ROIDRAGE_CXXFLAGS) -Imain/inc
alpha_OBJCXXFLAGS := $(ROIDRAGE_OBJCXXFLAGS)
ALPHA_LDFLAGS     := $(ROIDRAGE_LDFLAGS)
ALPHA_SRCS        := $(ROIDRAGE_SRCS) \
										 $(shell find alpha -name *.cpp) \
										 $(shell find main -name *.cpp) # This is a deprecated path - 
																								    # everything should be moved 
																								    # to more specific or more general dirs

# Create all the targets for our application, and specify any compiler or linker dependencies
$(eval $(call APPLICATION,alpha))

#-------------------------------------------------------------------------------

BETA_CXXFLAGS     := $(ROIDRAGE_CXXFLAGS) -Ibeta/inc 
beta_OBJCXXFLAGS  := $(ROIDRAGE_OBJCXXFLAGS)
BETA_LDFLAGS      := $(ROIDRAGE_LDFLAGS)
BETA_SRCS         := $(ROIDRAGE_SRCS) \
										 $(shell find beta -name *.cpp)

# Create all the targets for our application, and specify any compiler or linker dependencies
$(eval $(call APPLICATION,beta))

#-------------------------------------------------------------------------------
