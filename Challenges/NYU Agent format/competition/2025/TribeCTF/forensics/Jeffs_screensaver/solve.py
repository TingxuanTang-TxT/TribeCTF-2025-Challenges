from json import load
import zipfile


def times(block):
    return int(block["inputs"]["TIMES"][1][1])


def substack(block):
    return block["inputs"]["SUBSTACK"][1]


def duration(block):
    dur: str = block["inputs"]["DURATION"][1][1]
    return ".-"[int(dur) - 1] if dur.isnumeric() else " "


def follow_chain(first, blocks, skip=0) -> list[str]:
    for _ in range(skip):
        first = blocks[first]["next"]
    if first is None:
        return []
    block = blocks[first]
    opcode = block.get("opcode")
    match opcode:
        case None:
            return []
        case "event_whenflagclicked":
            return follow_chain(block["next"], blocks)
        case "control_forever":
            return follow_chain(substack(block), blocks)
        case "control_repeat":
            t = times(block)
            target = substack(block)
            inner = follow_chain(target, blocks)
            # print(inner)
            return inner * t + follow_chain(block["next"], blocks)
        case "control_wait":
            dur = duration(block)
            return [dur] + follow_chain(block["next"], blocks)
        case "looks_switchcostumeto":
            return [" "] + follow_chain(block["next"], blocks)
        case "motion_glidesecstoxy":
            # ignore delay immediately after gliding
            return ["\n"] + follow_chain(block["next"], blocks, skip=1)
        case (
            "motion_goto"
            | "motion_ifonedgebounce"
            | "motion_pointindirection"
            | "motion_turnright"
        ):
            return follow_chain(block["next"], blocks)
        case unknown:
            raise ValueError(str(unknown))

# scratch projects are just zip files, so load the json file from the project

project = zipfile.ZipFile("dist/ScreensaverMack").open("project.json")
j = load(project)
# find the moving Mack
blocks = next(t["blocks"] for t in j["targets"] if not t["isStage"])

# table of morse code values, taken from online
# fmt: off
morse = { 'A':'.-', 'B':'-...', 'C':'-.-.', 'D':'-..', 'E':'.', 'F':'..-.', 'G':'--.', 'H':'....', 'I':'..', 'J':'.---', 'K':'-.-', 'L':'.-..', 'M':'--', 'N':'-.', 'O':'---', 'P':'.--.', 'Q':'--.-', 'R':'.-.', 'S':'...', 'T':'-', 'U':'..-', 'V':'...-', 'W':'.--', 'X':'-..-', 'Y':'-.--', 'Z':'--..', '1':'.----', '2':'..---', '3':'...--', '4':'....-', '5':'.....', '6':'-....', '7':'--...', '8':'---..', '9':'----.', '0':'-----', ', ':'--..--', '.':'.-.-.-', '?':'..--..', '/':'-..-.', '-':'-....-', '(':'-.--.', ')':'-.--.-', ":": "---...", "'":".----."}
morse_lookup = {v: k for k, v in morse.items()}
# fmt: on

# scratch always starts on a "event_whenflagclicked" block
start = next(b for b in blocks if blocks[b].get("opcode") == "event_whenflagclicked")
res = "".join(follow_chain(start, blocks)).strip()

flag = " ".join(
    "".join(morse_lookup[code] for code in line.split(" ") if code)
    for line in res.split("\n")
)
print(flag)
