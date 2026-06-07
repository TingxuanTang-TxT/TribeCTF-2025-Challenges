#!/usr/bin/env python3 

from pathlib import Path
from subprocess import Popen, PIPE
from os import makedirs
from shutil import copytree, copy, rmtree

crate = Path("wasm/").absolute()

cmd = Popen(["cargo", "build", "--release"], cwd=crate, stdout=PIPE, stderr=PIPE)
cmd.communicate()
assert cmd.returncode == 0

module = Path("wasm/target/wasm32-unknown-unknown/release/wasm.wasm").read_bytes()

cmd = Popen(
    ["gleam", "run", "-m", "lustre/dev", "build", "--ext=.js"], stdout=PIPE, stderr=PIPE
)
cmd.communicate()
assert cmd.returncode == 0

makedirs("dist/site", exist_ok=True)
rmtree(Path("dist/src"))
rmtree(Path("dist/site/priv"))
copytree(Path("src/"), Path("dist/src"))
copytree(Path("priv/"), Path("dist/site/priv"))

copy(Path("index.html"), Path("dist/site/index.html"))
copy(
    Path("wasm/target/wasm32-unknown-unknown/release/wasm.wasm"),
    Path("dist/site/wasm.wasm"),
)
