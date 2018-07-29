import yogi
import unittest
import datetime

from .common import TestCase


class TestTime(TestCase):
    def test_get_current_time(self):
        t = yogi.get_current_time()
        self.assertIsInstance(t, datetime.datetime)
        now = datetime.datetime.now(datetime.timezone.utc)
        delta = datetime.timedelta(seconds=1)
        self.assertGreater(t, now - delta)
        self.assertLess(t, now + delta)

    def test_format_time(self):
        t = datetime.datetime.fromtimestamp(1234356789.123456789)
        s = yogi.format_time(t)
        self.assertEqual(s, "2009-02-11T12:53:09.123Z")

        t = datetime.datetime.fromtimestamp(1234356789.123456789)
        s = yogi.format_time(t, "%Y%m%d%H%M%S%3%6%9")
        self.assertTrue(s.startswith("20090211125309123"))

    def test_parse_time(self):
        t = yogi.parse_time("2009-02-11T12:53:09.123Z")
        self.assertAlmostEqual(t.timestamp(), 1234356789.123456789, delta=1e-3)

        t = yogi.parse_time("20090211125309123456789", "%Y%m%d%H%M%S%3%6%9")
        self.assertAlmostEqual(t.timestamp(), 1234356789.123456789, delta=1e-3)

if __name__ == '__main__':
    unittest.main()
