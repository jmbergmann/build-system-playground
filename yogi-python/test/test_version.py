import yogi
import unittest


class TestVersion(unittest.TestCase):
    def test_get_version(self):
        version = yogi.get_version()
        self.assertIsInstance(version, str)
        self.assertGreater(len(version), 4)


if __name__ == '__main__':
    unittest.main()
