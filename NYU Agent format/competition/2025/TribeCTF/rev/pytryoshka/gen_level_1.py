from fill_template import fill
from pathlib import Path
from random import random, randrange
import ctypes
import sys


template = Path("level_1.template").read_text()


level_0 = Path("generated/level_0.py").read_bytes()

chunk_size = 256
steps = "[" + ", ".join([f"slice({i},{i + 3})" for i in range(0, chunk_size, 4)]) + "]"
jumps = []

for i in range(0, len(level_0), chunk_size):
    text = level_0[i:i+chunk_size]
    padded = len(text) * 4 // 3
    payload = sum(
        int.from_bytes(text[3 * i : 3 * i + 3], "little") << (30 * i)
        for i in range((len(text)+2) // 3)
    )
    out = f'b"\\x00" * {padded}'

    jumps.append(f"_{i} = ctypes.memmove(id(out := {out})+32,id({payload})+24,{padded}) == 'silly' or out.replace(b'\\x00', b'')\n")

lines_fill = f"""
import ctypes

{"".join(jumps)}

(lambda: ...).__globals__["__builtins__"].exec(({"+".join("_"+str(i) for i in range(0, len(level_0), chunk_size))}).decode())


"""

"""
typedef struct _PyLongValue {
    uintptr_t lv_tag; /* Number of digits, sign and flags */ // 8
    digit ob_digit[1];
} _PyLongValue;

struct _longobject {
    PyObject_HEAD // 16
    _PyLongValue long_value;
};

typedef struct {
    PyObject_VAR_HEAD // 24 bytes
    Py_DEPRECATED(3.11) Py_hash_t ob_shash; // 8 bytes
    char ob_sval[1];
} PyBytesObject;

typedef struct {
    PyObject head; // 16
    Py_ssize_t len; // 8
    Py_hash_t hash; // 8
    struct { } state; // 8 (with padding)
} PyAsciiObject; // 48
"""

filled = fill(template, {"lines": lines_fill})
Path("generated/level_1.py").write_text(filled)
