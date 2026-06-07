from fill_template import fill
from pathlib import Path
from utils import underscoreify

flag = b"tribectf{_kandoryoukou__524_}"
prime = 277768995366174730356023524943561131681
inverses = [pow(b, -1, prime) for b in flag]

template = Path("level_0.template").read_text()
filled = fill(template, {"inverses": str(inverses)})
under = underscoreify(filled)

Path("generated/level_0.py").write_text(under)
