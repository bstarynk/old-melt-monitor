# file gdb-pprint.py
# very incomplete
# inspired by http://stackoverflow.com/a/23970415/841108
#####
#  Copyright (C)  2014 Free Software Foundation, Inc.
#    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
#    This file is part of GCC.
#  
#    GCC is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3, or (at your option)
#    any later version.
#  
#    GCC is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#    You should have received a copy of the GNU General Public License
#    along with GCC; see the file COPYING3.   If not see
#    <http://www.gnu.org/licenses/>.
######
def deref(reference):
    target = reference.dereference()
    if str(target.address) == '0x0':
        return 'NULL'
    else if str(target.address) == '0x1':
        return 'EMPTY'
    else:
        return target

class cstringprinter:
    def __init__(self, value, maxlen=4096):
        try:
            ends = gdb.selected_inferior().search_memory(value.address, maxlen, b'\0')
            if ends is not None:
                maxlen = ends - int(str(value.address), 16)
                self.size = str(maxlen)
            else:
                self.size = '%s+' % str(maxlen)
            self.data = bytearray(gdb.selected_inferior().read_memory(value.address, maxlen))
        except:
            self.data = None
    def to_string(self):
        if self.data is None:
            return 'NULL'
        else:
            return '\"%s\"(%s)' % (str(self.data).encode('string_escape').replace('"', '\\"').replace("'", "\\\\'"), self.size)

class momint_printer:
    def __init__(self, value):
        self.value = value.cast(gdb.lookup_type('struct momint_st'))
    def to_string(self):
        return '(momint_t)%s' % str(self.value['intval'])

class momdouble_printer:
    def __init__(self, value):
        self.value = value.cast(gdb.lookup_type('struct momdouble_st'))
    def to_string(self):
        return '(momdouble_t)%s' % (self.value['dblval'])

class momstring_printer:
    def __init__(self, value):
        self.value = value.cast(gdb.lookup_type('struct momstring_st'))
    def to_string(self):
        return '(momstring_t)%s' % (self.value['cstr'])
