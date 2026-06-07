import marshal
from fill_template import fill
from pathlib import Path
import struct
from utils import hexify, obfuscate_names
import sys

sys.set_int_max_str_digits(5000)


level_1_text = Path("generated/level_1.py").read_text()
level_1_code = compile(level_1_text, "level_1.py", "exec")

marshal_float = struct.unpack("<d", b"marshal\x00")[0]
marshal_fill = f"__builtins__.__dict__['__import__']('struct').pack('<d', {marshal_float})[:7].decode()"

loads = int.from_bytes(b"loads")
xor_with = int.from_bytes(b"withw")
xored = loads ^ xor_with
highest = max(xor_with.bit_length(), xored.bit_length())

loads_fill = f"int('0b'+''.join(str(int(a!=b)) for a,b in zip(f'{{{xor_with}:0{highest}b}}', f'{{{xored}:0{highest}b}}')),base=0).to_bytes(5).decode()"
template = Path("level_2.template").read_text()

filled = fill(
    template,
    {
        "level_1": str(marshal.dumps(level_1_code)),
        "marshal": marshal_fill,
        "loads": loads_fill,
    },
)
Path("generated/level_2.py").write_text(
    "#!/usr/bin/python3.11\n" + hexify(obfuscate_names(filled))
)
