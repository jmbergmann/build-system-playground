import os.path
import re
import unittest

from typing import Union


yogi_core_h = None  # type: str

with open(os.path.join(os.path.dirname(__file__),
                       "../../yogi-core/include/yogi_core.h")) as f:
    yogi_core_h = f.read()


class TestCase(unittest.TestCase):
    def get_core_macro(self, macro_name: str) -> Union[str, int]:
        regex = re.compile("#define {} ([^ \\s]+).*".format(macro_name))
        m = re.search(regex, yogi_core_h)
        if not m:
            raise Exception(
                "Macro {} not found in yogi_core.h".format(macro_name))
        return eval(m.group(1))

    def assertEnumElementMatches(self, macro_prefix, elem):
        macro_name = macro_prefix + elem.name
        macro_val = self.get_core_macro(macro_name)
        self.assertEqual(elem, macro_val)

    def assertEnumMatches(self, macro_prefix, enum):
        for elem in enum:
            self.assertEnumElementMatches(macro_prefix, elem)

    def assertFlagMatches(self, macro_prefix, enum_elem):
        self.assertEnumElementMatches(macro_prefix, enum_elem)

    def assertFlagCombinationMatches(self, macro_prefix, enum_elem,
                                     exceptions):
        macro_name = macro_prefix + enum_elem.name
        regex = re.compile("#define {} .*".format(macro_name))
        macro_found = bool(re.search(regex, yogi_core_h))
        self.assertTrue(macro_found)

        expected = 0
        for ev in type(enum_elem):
            if ev is not enum_elem and ev not in exceptions:
                expected |= ev
        self.assertEqual(enum_elem, expected)
