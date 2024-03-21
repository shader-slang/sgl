from glob import glob
from pathlib import Path
import json
import re

DIR = Path(__file__).parent

EMBED_REGEX = r"^\[comment\]\: <> \(embed (.*?)\)\n```(\S*\n[^`]*)```"

def process_notebook(path):
    doc = json.load(open(path))
    doc_changed = False

    # Iterate through all markdown cells.
    for cell in doc["cells"]:
        if cell["cell_type"] == "markdown":
            source = cell["source"]
            source = "".join(source)
            offset = 0
            changed = False
            for m in re.finditer(EMBED_REGEX, source, re.MULTILINE):
                old_snippet = m[2]
                new_snippet = open(DIR / m[1]).read()
                new_snippet = f"C#\n// {m[1]}\n\n" + new_snippet
                if not new_snippet.endswith("\n"):
                    new_snippet += "\n"
                source = source[0:m.start(2) + offset] + new_snippet + source[m.end(2) + offset:]
                offset = len(new_snippet) - len(old_snippet)
                changed = True
            if changed:
                source = [line + "\n" for line in source.split("\n")]
                cell["source"] = source
                doc_changed = True

    if doc_changed:
        json.dump(doc, open(path, "w"), indent=1)

    pass


notebooks = glob(str(DIR / "*.ipynb"))
for notebook in notebooks:
    process_notebook(notebook)
