#!/usr/bin/python3.11

from pathlib import Path
import ast
from sys import argv

code = Path("../generated/level_2.py").read_text()
raw_bytes = next(
    node.value
    for node in ast.walk(ast.parse(code))
    if isinstance(node, ast.Constant)
    and isinstance(node.value, bytes)
    and len(node.value) > 100
)

# magic for python 3.11
magic = (3495).to_bytes(2, "little") + b"\r\n"
# make 3 empty uint32 fields
header = magic + b"\x00" * 12
Path(argv[1]).write_bytes(header + raw_bytes)
