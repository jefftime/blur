.POSIX:
.SUFFIXES:

########################################
# Overrideable
########################################

CC=clang
CFLAGS=-g -std=c90 -pedantic-errors -Wall -Wconversion
LDFLAGS=-rdynamic
PLATFORM=LINUX
RENDER_BACKEND=VK
OBJ_SUFFIX=.o
SRC=src/tortuga.c \
	src/xrand.c \
	src/render_$(RENDER_BACKEND)_instance.c \
	src/render_$(RENDER_BACKEND)_memory.c \
	src/render_$(RENDER_BACKEND)_device.c \
	src/render_$(RENDER_BACKEND)_pass.c \
	src/render_$(RENDER_BACKEND)_shader.c \
	src/window_linux.c \
	src/keypoll_linux.c
SHADERS=src/shaders/default
STATICLIBS=libs/libxcb.a libs/libXdmcp.a libs/libXau.a

########################################
# Calculated
########################################

OBJ=$(SRC:.c=$(OBJ_SUFFIX))
DEP=$(SRC:.c=.d)
LIBS=-ldl -Wl,--start-group $(STATICLIBS) -Wl,--end-group
DEFINES=-DPLATFORM_$(PLATFORM) -DRENDER_BACKEND_$(RENDER_BACKEND)
SHADER_HEADERS=$(SHADERS:=_vert.h) $(SHADERS:=_frag.h)

all: tortuga

$(DEP): $(SHADER_HEADERS)

tortuga: .depend $(OBJ) $(SHADER_HEADERS)
	@echo LINK $@
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -rf $(DEP)
	rm -f tortuga
	rm -f src/shaders/*.h
	rm .depend

.depend: $(DEP)
	@cat $(DEP) > .depend
	@echo GEN .depend

include .depend
.SUFFIXES: .c .o .d .vert .frag .h
.c.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

.c.d:
	@echo GEN $@
	@$(CC) -MM -MF $@ -MQ $(@:.d=.o) $<

# Example:
#   $<					 - src/shaders/default.vert
#   $(<:.vert=)  - src/shdaers/default
#   $(<F:.vert=) - default
.vert.h:
	@echo V_SHADER $<
	@glslangValidator -V --vn $(<F:.vert=)_src -o $(<:.vert=).tmp $<
	@tail -n +3 $(<:.vert=).tmp > $(<:.vert=).h
	@echo "\n" >> $(<:.vert=).h
	@rm $(<:.vert=).tmp

.frag.h:
	@echo F_SHADER $<
	@glslangValidator -V --vn $(<F:.frag=)_src -o $(<:.frag=).tmp $<
	@tail -n +3 $(<:.frag=).tmp > $(<:.frag=).h
	@echo "\n" >> $(<:.frag=).h
	@rm $(<:.frag=).tmp
