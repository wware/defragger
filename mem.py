# A litle library of memory diagnostics

# In no particular order of usefulness:
# https://pymotw.com/2/gc/
# https://code.tutsplus.com/tutorials/understand-how-much-memory-your-python-objects-use--cms-25609
# https://www.huyng.com/posts/python-performance-analysis
# http://stackoverflow.com/questions/23369937

import gc
import os
import sys
import pprint
import resource
import Queue
from collections import namedtuple
from types import InstanceType

def _get_obj_type(obj):
    objtype = type(obj)
    if type(obj) is InstanceType:
        objtype = obj.__class__
    return objtype

def _short_typename(obj):
    return _get_obj_type(obj).__name__

def _long_typename(obj):
    objtype = _get_obj_type(obj)
    name = objtype.__name__
    module = getattr(objtype, '__module__', None)
    if module:
        return '%s.%s' % (module, name)
    else:
        return name

def info(typename, objects, exclude=None):
    if exclude is None:
        exclude = []
    count = sz = 0
    if '.' in typename:
        _typename = _long_typename
    else:
        _typename = _short_typename
    for o in objects:
        if o in exclude:
            continue
        if _typename(o) == typename:
            count += 1
            sz += sys.getsizeof(o)
    return (count, sz / (1024. * 1024.))

def all_info(objects):
    s = set()
    for o in objects:
        s.add(_long_typename(o))
    n = sz = 0
    for typename in s:
        _n, _sz = info(typename, objects)
        n += _n
        sz += _sz
    return (n, sz)


CycleFound = Exception

def find_cycles(obj):
    seen = set()
    to_process = Queue.Queue()
    to_process.put(obj)
    while not to_process.empty():
        next_guy = to_process.get()
        seen.add(id(next_guy))
        for r in gc.get_referents(next_guy):
            if isinstance(r, basestring) or isinstance(r, type):
                # Ignore strings and classes
                pass
            elif id(r) in seen:
                raise CycleFound(pprint.pformat(r))
            else:
                to_process.put(r)

Memory = namedtuple('Memory', ['pid', 'used'])

def memory_usage():
    return Memory(
        os.getpid(),
        resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024.)
