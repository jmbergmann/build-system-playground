import yogi
import unittest
import datetime

from .common import TestCase


class TestTime(TestCase):
    def test_get_current_time(self):
        dt = yogi.get_current_time().to_datetime()
        now = datetime.datetime.now(datetime.timezone.utc)
        delta = datetime.timedelta(seconds=1)
        self.assertGreater(dt, now - delta)
        self.assertLess(dt, now + delta)


if __name__ == '__main__':
    unittest.main()
