# -*- Mode: Python; -*-
#                            Package   : omniORBpy
# ami.py                     Created on: 2012/06/27
#                            Author    : Duncan Grisby (dgrisby)
#
#    Copyright (C) 2012 Apasphere Ltd.
#
#    This file is part of the omniORBpy library
#
#    The omniORBpy library is free software; you can redistribute it
#    and/or modify it under the terms of the GNU Lesser General
#    Public License as published by the Free Software Foundation;
#    either version 2.1 of the License, or (at your option) any later
#    version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
#    MA 02111-1307, USA
#
# Description:
#    AMI support

import omniORB
Messaging = omniORB.openModule("Messaging")
CORBA     = omniORB.openModule("CORBA")

try:
    property
except NameError:
    def property(*args):
        return None


class PollerImpl (Messaging.Poller):
    def __init__(self, poller):
        self._poller = poller

    def is_ready(self, timeout):
        return self._poller.is_ready(timeout)

    def create_pollable_set(self):
        raise CORBA.NO_IMPLEMENT(0, CORBA.COMPLETED_NO)

    def _get_operation_target(self):
        return self._poller.operation_target()

    operation_target = property(_get_operation_target)

    def _get_operation_name(self):
        return self._poller.operation_name()

    operation_name = property(_get_operation_name)

    def _get_associated_handler(self):
        return self._poller.get_handler()

    def _set_associated_handler(self, handler):
        self._poller.set_handler(handler)

    associated_handler = property(_get_associated_handler,
                                  _set_associated_handler)

    def _get_is_from_poller(self):
        return self._poller.is_from_poller()

    is_from_poller = property(_get_is_from_poller)


class ExceptionHolderImpl (Messaging.ExceptionHolder):
    def __init__(self, poller):
        self._poller = poller

    def raise_exception(self):
        self._poller.raise_exception()
