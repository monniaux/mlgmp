# Use GNU Make !
RANLIB= ranlib

OCAML_LIBDIR:= $(shell ocamlc -where)
# GMP_INCLUDES= -I/opt/gmp-4.1.2/include -I/users/absint2/local/include -I$(HOME)/packages/gmp/include

# GMP_LIBDIR=/opt/gmp-4.1.2/lib
DESTDIR= $(OCAML_LIBDIR)/gmp

#RLIBFLAGS= -cclib "-Wl,-rpath $(GMP_LIBDIR)" # Linux, FreeBSD
#RLIBFLAGS= -cclib "-Wl,-R $(GMP_LIBDIR)" # Solaris
# RLIBFLAGS= # MacOS X

# LIBFLAGS= -cclib -L. -cclib -L$(GMP_LIBDIR) $(RLIBFLAGS) \
#	-cclib -lmpfr -cclib -lgmp -cclib -L$(DESTDIR)
LIBFLAGS = -cclib -L$(shell pwd) -cclib -lgmp -cclib -lmpfr

#CC= icc
CFLAGS_MISC= -Wall -Wno-unused -Werror -g -O3
#CFLAGS_MISC=
CFLAGS_INCLUDE= -I $(OCAML_LIBDIR) $(GMP_INCLUDES)
CFLAGS= $(CFLAGS_MISC) $(CFLAGS_INCLUDE)

OCAMLC= ocamlc -g
OCAMLOPT= ocamlopt
OCAMLFLAGS=

CMODULES= mlgmp_z.c mlgmp_q.c mlgmp_f.c mlgmp_fr.c mlgmp_random.c mlgmp_misc.c
CMODULES_O= $(CMODULES:%.c=%.o)

LIBS= libmlgmp.a gmp.a gmp.cma gmp.cmxa gmp.cmi

PROGRAMS= essai essai.opt toplevel\
	test_suite test_suite.opt
TESTS= test_suite test_suite.opt

all:	$(LIBS) tests

install: all
	-mkdir $(DESTDIR)
	cp $(LIBS) gmp.mli $(DESTDIR)

tests:	$(LIBS) $(TESTS)
	./test_suite
	./test_suite.opt

%.i: %.c
	$(CC) $(CFLAGS) -E $*.c > $*.i

%.cmo: %.ml %.cmi
	$(OCAMLC) $(OCAMLFLAGS) -c $*.ml

%.cmx: %.ml %.cmi
	$(OCAMLOPT) $(OCAMLFLAGS) -c $*.ml

%.cmo: %.ml
	$(OCAMLC) $(OCAMLFLAGS) -c $*.ml

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLFLAGS) -c $*.ml

%.cmi: %.mli
	$(OCAMLC) $(OCAMLFLAGS) -c $*.mli

$(CMODULES_O): conversions.c config.h

libmlgmp.a: $(CMODULES_O)
	$(AR) -rc $@ $+
	$(RANLIB) $@

gmp.cma: gmp.cmo libmlgmp.a
	$(OCAMLC) $(OCAMLFLAGS) -a gmp.cmo -cclib -lmlgmp $(LIBFLAGS) -o $@

gmp.a gmp.cmxa: gmp.cmx libmlgmp.a
	$(OCAMLOPT) $(OCAMLFLAGS) -a gmp.cmx -cclib -lmlgmp  $(LIBFLAGS) -o $@

pretty_gmp.cmo: pretty_gmp.cmi gmp.cmo

toplevel: gmp.cma creal.cmo pretty_gmp.cmo install_pp.cmo creal_pp.cmo install_creal_pp.cmo
	ocamlmktop -custom $+ -o $@

essai:	gmp.cma essai.cmo
	$(OCAMLC) -custom $+ -o $@

essai.opt:	gmp.cmxa essai.cmx
	$(OCAMLOPT) $+ -o $@

test_suite:	gmp.cma test_suite.cmo
	$(OCAMLC) -custom $+ -o $@

test_suite.opt:	gmp.cmxa test_suite.cmx
	$(OCAMLOPT) $+ -o $@

clean:
	rm -f *.o *.cm* $(PROGRAMS) *.a

depend:
	ocamldep *.ml *.mli > depend

.PHONY: clean

include	depend
