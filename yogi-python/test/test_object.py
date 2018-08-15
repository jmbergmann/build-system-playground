import yogi
import unittest

from .common import TestCase

from ctypes import c_void_p


class FakeObject:
    def __init__(self):
        self._handle = c_void_p()


class TestContext(TestCase):
    def test_format(self):
        obj = yogi.Context()

        s = obj.format()
        self.assertRegex(s, "Context \\[[1-9a-f][0-9a-f]+\\]")

        s = obj.format("$T-[$X]")
        self.assertRegex(s, "Context-\\[[1-9A-F][0-9A-F]+\\]")

        obj = FakeObject()
        s = yogi.Object.format(obj)
        self.assertTrue(s.startswith("INVALID"))

        s = yogi.Object.format(obj, nullstr="MOO")
        self.assertEqual(s, "MOO")

    def test_str(self):
        obj = yogi.Context()
        self.assertRegex(str(obj), "Context \\[[1-9a-f][0-9a-f]+\\]")


if __name__ == '__main__':
    unittest.main()
