#
# x86_win95.mk - make variables and rules specific to Windows 95.
#

WindowsNT = 1
x86Processor = 1

BINDIR = bin/x86_win32
LIBDIR = lib/x86_win32

ABSTOP = $(shell cd $(TOP); pwd)

#
# Python set-up
#
# You must set a path to a Python 1.5.2 interpreter. If you do not
# wish to make a complete installation, you may download a minimal
# Python from ftp://ftp.uk.research.att.com/pub/omniORB/python/
# In that case, uncomment the first line below.

#PYTHON = $(ABSTOP)/$(BINDIR)/omnipython
#PYTHON = //c/progra~1/Python/python


#
# Include general win32 things
#

include $(THIS_IMPORT_TREE)/mk/win32.mk



IMPORT_CPPFLAGS += -D__x86__


# Default directory for the omniNames log files.
OMNINAMES_LOG_DEFAULT_LOCATION = C:\\temp
