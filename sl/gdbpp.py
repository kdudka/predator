import gdb.printing
import re

class pp_ObjHandle(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return '#%d' % self.val['id_']

    def display_hint(self):
        return 'string'

def predator_lookup_fnc(val):
    lookup_tag = val.type.tag
    if lookup_tag == None:
        return None

    if re.compile('^ObjHandle|PtrHandle$').match(lookup_tag):
        return pp_ObjHandle(val)

    return None

def register_printers():
    gdb.pretty_printers.append(predator_lookup_fnc)
