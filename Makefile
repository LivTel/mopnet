#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
#
CC = gcc
CFLAGS   = -O3 -march=native -mtune=native 
INCLUDES = -I/star-2018A/include -I/usr/local/PI/include
LFLAGS   = -L/star-2018A/lib 
LIBS     = -lm -lrt -lpthread -latcore -latutility -lcfitsio -lpi_pi_gcs2
SRCS     =  mop_cam.c mop_fts.c mop_log.c mop_msg.c mop_opt.c mop_rot.c mop_utl.c mop_whl.c
OBJS     = $(SRCS:.c=.o)
DEPS     = $(OBJS:.o=.d)
MOPNET   = mopnet 
MOPCMD   = mopcmd

DEPFILE = .depends
DEPTOKEN = '\# MAKEDEPENDS'
DEPFLAGS = -Y -f $(DEPFILE) -s $(DEPTOKEN) 

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: clean depend

all:    $(MOPNET) $(MOPCMD) 
	@echo Done  

$(MOPNET): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) mopnet.c -o $(MOPNET) $(OBJS) $(LFLAGS) $(LIBS)

$(MOPCMD): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) mopcmd.c -o $(MOPCMD) $(OBJS) $(LFLAGS) $(LIBS)

-include $(DEPS)

# Compile sources  
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

depend:
	rm -f $(DEPFILE)
	make $(DEPFILE)

$(DEPFILE):
	@echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CFLAGS) -- $(SRCS) 

# Cleanup 
clean:
	$(RM) *.o $(MOPNET) $(MOPCMD) 

sinclude $(DEPFILE)

# Don't build dependencies
#depend: $(SRCS)
#	makedepend $(INCLUDES) $^

# DO NOT DELETE
