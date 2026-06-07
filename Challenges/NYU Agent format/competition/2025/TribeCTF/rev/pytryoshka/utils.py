import ast
from random import Random
from string import ascii_lowercase


def hexify_string(s):
    return "".join(r"\x" + f"{ord(c):02x}" for c in s)


def get_string_locs(code: str) -> list[tuple[str, int]]:
    tree = ast.parse(code)
    locs = []
    ct = 1
    linenos = {(ct := ct + 1): i + 1 for i, c in enumerate(code) if c == "\n"} | {1: 0}
    print(linenos)

    class GetStrings(ast.NodeVisitor):
        def visit_JoinedStr(self, node: ast.JoinedStr):
            return node

        def visit_Constant(self, node: ast.Constant):
            if isinstance(node, ast.Constant) and isinstance(node.value, str):
                locs.append((node.value, linenos[node.lineno] + node.col_offset + 1))
            return super().visit_Constant(node)

    GetStrings().visit(tree)
    return sorted(locs, key=lambda d: d[1])


def hexify(code: str) -> str:
    print("\n".join(code.splitlines()[:-3]))
    string_positions = get_string_locs(code)
    print(code.index("f'"))
    print(string_positions)
    result = []
    pos = 0
    for value, loc in string_positions:
        literal = hexify_string(value)
        result.append(code[pos:loc])
        result.append(literal)
        pos = loc + len(value)
    result.append(code[pos:])
    print("\n".join("".join(result).splitlines()[:-3]))
    return "".join(result)


def obfuscate_names(template: str, names=None) -> str:
    r = Random(b"obfuscation")
    names = names or {}
    tree = ast.parse(template)
    for node in ast.walk(tree):
        if isinstance(node, (ast.AnnAssign, ast.NamedExpr)):
            assert isinstance(node.target, ast.Name)
            if node.target.id not in names:
                names[node.target.id] = "".join(
                    r.choice(ascii_lowercase) for _ in range(10)
                )
            node.target.id = names[node.target.id]
        elif isinstance(node, ast.Assign):
            for name in node.targets:
                assert isinstance(name, ast.Name)
                if name.id not in names:
                    names[name.id] = "".join(
                        r.choice(ascii_lowercase) for _ in range(10)
                    )
                name.id = names[name.id]
        elif isinstance(node, ast.FunctionDef):
            assert isinstance(node.name, ast.Name)
            if node.name.id not in names:
                names[node.name.id] = "".join(
                    r.choice(ascii_lowercase) for _ in range(10)
                )
            node.name.id = names[node.name.id]
        elif isinstance(node, ast.Name) and node.id in names:
            node.id = names[node.id]
    return ast.unparse(tree)


class Underscorer(ast.NodeTransformer):
    def __init__(self):
        self.new_names = {}
        self.counter = 1
        self.functions = set()
        self.literals = set()

    def visit_Call(self, node: ast.Call):
        assert isinstance(node.func, ast.Name)
        name = node.func.id

        if name not in self.new_names:
            self.functions.add(name)
            self.new_names[name] = "_" * self.counter
            self.counter += 1
        node.func.id = self.new_names[name]
        for i, arg in enumerate(node.args):
            node.args[i] = self.visit(arg)
        for i, arg in enumerate(node.keywords):
            node.keywords[i] = self.visit(arg)
        return node

    def visit_Assign(self, node):
        node.value = self.visit(node.value)
        for i, t in enumerate(node.targets):
            assert isinstance(t, ast.Name)
            node.targets[i] = self.visit_Name(t)
        return node

    def visit_Name(self, node):
        if node.id not in self.new_names:
            self.new_names[node.id] = "_" * self.counter
            self.counter += 1
        node.id = self.new_names[node.id]
        return node

    def visit_Constant(self, node):
        r = repr(node.value)
        if r not in self.new_names:
            self.new_names[r] = "_" * self.counter
            self.counter += 1
            self.literals.add(r)
        return ast.Name(id=self.new_names[r])

    def visit_List(self, node):
        for i, elem in enumerate(node.elts):
            node.elts[i] = self.visit(elem)

        u = ast.unparse(node)
        if u not in self.new_names:
            self.new_names[u] = "_" * self.counter
            self.counter += 1
            self.literals.add(u)

        return ast.Name(id=self.new_names[u])


def underscoreify(template: str) -> str:
    tree: ast.Module = ast.parse(template)
    u = Underscorer()
    u.visit(tree)

    first_not_assign = next(
        i for i, stmt in enumerate(tree.body) if not isinstance(stmt, ast.Assign)
    )
    assigns, rest = tree.body[:first_not_assign], tree.body[first_not_assign:]

    new_lines = [f"{u.new_names[elem]} = {elem}" for elem in u.functions | u.literals]
    old_assigns = ast.unparse(ast.Module(assigns, []))
    old_rest = ast.unparse(ast.Module(rest, []))
    pyramid = "\n".join(
        sorted(
            old_assigns.splitlines() + new_lines,
            key=lambda line: len(line.split(" ")[0]),
        )
    )
    return f"{pyramid}\n\n{old_rest}"
