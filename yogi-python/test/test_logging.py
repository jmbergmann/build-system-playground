import yogi
import unittest

from .common import TestCase


class TestLogging(TestCase):
    def test_verbosity_enum(self):
        self.assertEnumMatches("YOGI_VB_", yogi.Verbosity)

    def test_stream_enum(self):
        self.assertEnumMatches("YOGI_ST_", yogi.Stream)


if __name__ == '__main__':
    unittest.main()
