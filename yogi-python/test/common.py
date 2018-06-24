import os.path
import re

from typing import Union


yogi_core_h = None  # type: str

with open(os.path.join(os.path.dirname(__file__),
                       "../../yogi-core/include/yogi_core.h")) as f:
    yogi_core_h = f.read()


def get_core_macro(macro_name: str) -> Union[str, int]:
    regex = re.compile("#define {} ([^ \\s]+).*".format(macro_name))
    m = re.search(regex, yogi_core_h)
    if not m:
        raise Exception("Macro {} not found in yogi_core.h".format(macro_name))
    return eval(m.group(1))
