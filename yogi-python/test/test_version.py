import yogi
import unittest

class TestVersion(unittest.TestCase):
    def test_get_version(self):
        self.assertIsInstance(yogi.get_version(), str)

if __name__ == '__main__':
    unittest.main()
