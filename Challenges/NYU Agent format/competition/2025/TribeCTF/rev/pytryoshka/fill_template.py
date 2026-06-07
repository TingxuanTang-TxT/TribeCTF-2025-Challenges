import re


def fill(template: str, patterns: dict[str, str]):
    parts = re.split(r"(%\w+%)", template)
    for i, part in enumerate(parts):
        if part and part[0] == part[-1] == "%":
            part = part.strip("%")
            if (replace := patterns.get(part)) is not None:
                parts[i] = replace
            else:
                err = f"missing fill for %{part}%"
                raise ValueError(err)
    return "".join(parts)

