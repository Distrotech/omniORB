# Library version information

OMNITHREAD_VERSION = 3.2
OMNITHREAD_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNITHREAD_VERSION)))
OMNITHREAD_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNITHREAD_VERSION)))

OMNIORB_VERSION = 4.0.7
OMNIORB_VERSION_HEX = 0x040007F1
OMNIORB_MAJOR_VERSION = $(word 1,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MINOR_VERSION = $(word 2,$(subst ., ,$(OMNIORB_VERSION)))
OMNIORB_MICRO_VERSION = $(word 3,$(subst ., ,$(OMNIORB_VERSION)))

OMNIPY_VERSION = 2.7
OMNIPY_MAJOR = $(word 1,$(subst ., ,$(OMNIPY_VERSION)))
OMNIPY_MINOR = $(word 2,$(subst ., ,$(OMNIPY_VERSION)))
