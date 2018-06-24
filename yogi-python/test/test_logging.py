import yogi
import unittest

from .common import get_core_macro


class TestLogging(unittest.TestCase):
    def test_verbosity_enum(self):
        for vb in yogi.Verbosity:
            macro_name = "YOGI_VB_" + vb.name
            macro_val = get_core_macro(macro_name)
            self.assertEqual(vb, macro_val)

    def test_stream_enum(self):
        for st in yogi.Stream:
            macro_name = "YOGI_ST_" + st.name
            macro_val = get_core_macro(macro_name)
            self.assertEqual(st, macro_val)


if __name__ == '__main__':
    unittest.main()
