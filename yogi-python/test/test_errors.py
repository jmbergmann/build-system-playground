import yogi
import unittest

class TestErrors(unittest.TestCase):
    def test_result(self):
        self.assertFalse(yogi.Result(-1))
        self.assertTrue(yogi.Result(0))
        self.assertTrue(yogi.Result(1))
        self.assertEqual(yogi.Result(10).value, 10)
        self.assertEqual(yogi.Result(3), yogi.Result(3))
        self.assertNotEqual(yogi.Result(2), yogi.Result(3))
        self.assertGreater(len(str(yogi.Result(-3))), 5)
        self.assertNotEqual(hash(yogi.Result(1)), hash(yogi.Result(2)))

    def test_failure(self):
        self.assertFalse(yogi.Failure(-1))
        self.assertRaises(AssertionError, lambda: yogi.Failure(0))
        self.assertRaises(AssertionError, lambda: yogi.Failure(1))
        self.assertEqual(str(yogi.Failure(-1)), str(yogi.Result(-1)))
        self.assertEqual(yogi.Failure(-1), yogi.Result(-1))
        self.assertIsInstance(yogi.Failure(-1), yogi.Result)

    def test_success(self):
        self.assertTrue(yogi.Success(1))
        self.assertTrue(yogi.Success(0))
        self.assertRaises(AssertionError, lambda: yogi.Success(-1))
        self.assertEqual(str(yogi.Success(1)), str(yogi.Result(1)))
        self.assertEqual(yogi.Success(1), yogi.Result(1))
        self.assertIsInstance(yogi.Success(1), yogi.Result)

if __name__ == '__main__':
    unittest.main()
