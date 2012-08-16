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


class PollerImpl (Messaging.Poller):
    def __init__(self, poller):
        self._poller = poller

    # *** HERE: methods defined in IDL


class ExceptionHolderImpl (Messaging.ExceptionHolder):
    # *** HERE
    pass
