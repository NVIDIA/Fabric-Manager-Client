IDIR := ∕usr∕include
CXXFLAGS = -I $(IDIR) $(shell pkg-config --cflags jsoncpp)

LDIR := ∕usr∕lib
LDFLAGS= -L$(LDIR) -lnvfm -ljsoncpp

# Directories searched for libnvfm.so before linking, in the order the first
# match decides the verdict. Override this to point at a Fabric Manager
# development package installed outside the system paths:
#   make NVFM_LIB_DIRS=/opt/nvidia/lib
NVFM_LIB_DIRS ?= /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib64 /usr/lib /usr/local/lib

fmpm: fmpm.o
	@nvfm=''; \
	for dir in $(NVFM_LIB_DIRS); do \
		if [ -e "$$dir/libnvfm.so" ] || [ -L "$$dir/libnvfm.so" ]; then \
			nvfm="$$dir/libnvfm.so"; break; \
		fi; \
	done; \
	if [ -z "$$nvfm" ]; then \
		printf '%s\n' \
		  "Makefile: libnvfm.so was not found in any of: $(NVFM_LIB_DIRS)" \
		  "Makefile: Install the Fabric Manager development package: nvidia-fabricmanager-dev on Ubuntu," \
		  "Makefile: nvidia-fabricmanager-devel on RHEL. If it is already installed elsewhere, say where:" \
		  "Makefile:   make NVFM_LIB_DIRS=/path/to/lib" >&2; \
		exit 1; \
	elif [ ! -e "$$nvfm" ]; then \
		printf '%s\n' \
		  "Makefile: $$nvfm is a symlink to $$(readlink "$$nvfm" 2>/dev/null), which is not installed." \
		  "Makefile: The Fabric Manager development package here ships the linker symlink without the" \
		  "Makefile: library it names, so -lnvfm cannot resolve. Confirm with:" \
		  "Makefile:   dpkg -c nvidia-fabricmanager-dev_<version>.deb | grep libnvfm" \
		  "Makefile: A version-suffixed package such as nvidia-fabricmanager-dev-575 carries libnvfm.so.1." >&2; \
		exit 1; \
	fi
	g++ -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	-@rm -f fmpm.o
	-@rm -f fmpm
