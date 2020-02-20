# Copyright (c) 2018 by Thomas A. Early N7TAE

BINDIR=/usr/local/bin
CFGDIR=/usr/local/etc

# choose this if you want debugging help
#CPPFLAGS=-g -ggdb -W -Wall -I/usr/include -std=c++11 -DCFG_DIR=\"$(CFGDIR)\"
# or, you can choose this for a much smaller executable without debugging help
CPPFLAGS=-W -Wall -I/usr/include -std=c++11 -DCFG_DIR=\"$(CFGDIR)\"

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

sgsremote :  $(OBJS)
	g++ $(CPPFLAGS) -o sgsremote $(OBJS) -L/usr/lib -lconfig++ -lssl -lcrypto

%.o : %.cpp
	g++ $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(OBJS) $(DEPS) sgsremote

-include $(DEPS)

# install and uninstall need root priviledges
install : sgsremote
	/bin/cp sgsremote $(BINDIR)
	/bin/cp sgsremote.conf $(CFGDIR)
uninstall :
	/bin/rm $(BINDIR)/sgsremote
	/bin/rm $(CFGDIR)/sgsremote.conf
