import yogi
import unittest
import datetime


class TestTime(unittest.TestCase):
    def test_get_current_time(self):
        t = yogi.get_current_time()
        self.assertIsInstance(t, datetime.datetime)
        now = datetime.datetime.now()
        delta = datetime.timedelta(seconds=1)
        self.assertGreater(t, now - delta)
        self.assertLess(t, now + delta)


if __name__ == '__main__':
    unittest.main()
