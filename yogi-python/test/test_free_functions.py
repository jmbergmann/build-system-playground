import unittest
import yogi

class Test_test1(unittest.TestCase):
    def test_A(self):
        self.assertEqual(yogi.get_version(), '0.0.3-alpha')

if __name__ == '__main__':
    unittest.main()
