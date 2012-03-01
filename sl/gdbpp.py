import gdb.printing
import re

def have_ref_heap():
    return not not gdb.parse_and_eval("symdump_ref_heap")

def is_valid_ent(val):
    return not not gdb.parse_and_eval( \
            "%d < symdump_ref_heap->d.ents.ents_.size() " \
            "&& symdump_ref_heap->d.ents.ents_[%d]" \
            % (val, val))

def is_ent_of_type(type, val):
    if not is_valid_ent(val):
        return False

    lowType = gdb.lookup_type("AbstractHeapEntity").pointer()
    reqType = gdb.lookup_type(type).pointer()
    ptr = gdb.parse_and_eval("symdump_ref_heap->d.ents.ents_[%d]" % val)
    return not not ptr.cast(lowType).dynamic_cast(reqType)

def is_object_id(val):
    return is_ent_of_type("BlockEntity", val)

def is_value_id(val):
    return is_ent_of_type("BaseValue", val)

class pp_ObjHandle(object):
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

class pp_TObjId(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        str = "((class BlockEntity *)symdump_ref_heap->d.ents.ents_[%d])->code" % self.val
        return "%d [%s]" % (self.val, gdb.parse_and_eval(str))

    def display_hint(self):
        return 'string'

class pp_TValId(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        str = "symdump_ref_heap->valTarget(%d)" % self.val
        return "%d [%s]" % (self.val, gdb.parse_and_eval(str))

    def display_hint(self):
        return 'string'

def predator_lookup_fnc(val):
    lookup_tag = val.type.tag
    if lookup_tag == None:
        return None

    if re.compile('^ObjHandle|PtrHandle$').match(lookup_tag):
        return pp_ObjHandle(val)

    if re.compile('^SymHeap(Core)?$').match(lookup_tag):
        return pp_SymHeap(val)

    if re.compile('^TObjId$').match(lookup_tag) \
            and have_ref_heap() \
            and is_object_id(val):
        return pp_TObjId(val)

    if re.compile('^TValId$').match(lookup_tag) \
            and have_ref_heap() \
            and is_value_id(val):
        return pp_TValId(val)

    return None

def register_printers():
    gdb.pretty_printers.append(predator_lookup_fnc)
