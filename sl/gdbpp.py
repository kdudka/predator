import gdb.printing
import re

class pp_FldHandle(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return '#%d' % self.val['id_']

    def display_hint(self):
        return 'string'

class pp_SymHeap(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        gdb.parse_and_eval("dump_plot((class SymHeapCore *) %s)" % (self.val.address))
        return None

    def display_hint(self):
        return 'string'

def predator_lookup_fnc(val):
    lookup_tag = val.type.tag
    if lookup_tag == None:
        return None

    if re.compile('^FldHandle|PtrHandle$').match(lookup_tag):
        return pp_FldHandle(val)

# commented-out to prevent gdb from crashing
# <http://www.sourceware.org/bugzilla/show_bug.cgi?id=10344>
#    if re.compile('^SymHeap(Core)?$').match(lookup_tag):
#        return pp_SymHeap(val)

    return None

def register_printers():
    gdb.pretty_printers.append(predator_lookup_fnc)
