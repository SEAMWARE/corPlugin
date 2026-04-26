#
# FILE            makefile
#
# AUTHOR          Ken Zangelin
#
# Copyright 2026 Seamware
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LIB_SO        = libswPlugin.so
LIB           = libswPlugin.a
CC            = gcc
INCLUDE       = -I..
DFLAGS        =
CFLAGS        = -O2 -Wall -fPIC -Wno-unused-function -fstack-protector-all $(DFLAGS) $(INCLUDE) -MMD -MP
LIB_SOURCES   = swPlugin.c
LIB_OBJS      = $(LIB_SOURCES:c=o)
LIB_DEPS      = $(LIB_SOURCES:c=d)

SO_LDFLAGS    = -L../kargs
SO_LIBS       = -lkargs -ldl -lpthread
SO_RPATH      = -Wl,-rpath,'$$ORIGIN/../kargs'

LIBS          = ../kargs/libkargs.a -ldl -lpthread

all: $(LIB_SO) $(LIB)

clean:
						rm -f *.o
						rm -f *.a
						rm -f *~
						rm -f *.so

install:    all

di:         install

ci:         clean install

$(LIB):			$(LIB_OBJS) $(LIB_SOURCES)
						ar r $(LIB) $(LIB_OBJS)
						ranlib $(LIB)

$(LIB_SO):	$(LIB_OBJS) $(LIB_SOURCES)
						$(CC) -shared $(LIB_OBJS) -o $(LIB_SO) $(SO_LDFLAGS) $(SO_LIBS) $(SO_RPATH)

%.o: %.c
						$(CC) $(CFLAGS) -c $< -o $@

%.i: %.c
						$(CC) $(CFLAGS) -c $^ -E > $@

-include $(LIB_DEPS)
