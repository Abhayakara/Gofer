CFLAGS=-g -gdwarf-2 -Wall -Werror -arch x86_64

TARGETS=	libgofer.a gofer

LIBSRCS=tree.c near.c searchfile.c combine.c expr.c filelist.c errwarn.c \
	unicode.c not.c
LIBOBJS=tree.o near.o searchfile.o combine.o expr.o filelist.o errwarn.o \
	unicode.o not.o

RANLIB=ranlib

SRCS=lex.c main.c parse.c
OBJS=lex.o main.o parse.o

CC = gcc

#RM = del

all:	$(TARGETS)

libgofer.a:	$(LIBOBJS)
	$(RM) libgofer.a
	ar cr libgofer.a $(LIBOBJS)
	$(RANLIB) libgofer.a

gofer:	$(OBJS) libgofer.a
	$(CC) -o $@ $(OBJS) libgofer.a

clean:
	rm -f *.o *~
