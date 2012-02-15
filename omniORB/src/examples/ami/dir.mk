CXXSRCS = echo_callback.cc echo_poller.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS)

CORBA_INTERFACES = echo_ami

OMNIORB_IDL += -Wbami

echo_callback = $(patsubst %,$(BinPattern),echo_callback)
echo_poller   = $(patsubst %,$(BinPattern),echo_poller)

all:: $(echo_callback) $(echo_poller)

clean::
	$(RM) $(echo_callback) $(echo_poller)

export:: $(echo_callback) $(echo_poller)
	@(module="echoexamples"; $(ExportExecutable))

$(echo_callback): echo_callback.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(echo_poller): echo_poller.o $(CORBA_STATIC_STUB_OBJS) $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
