import yogi
import unittest

from .common import TestCase


class TestErrors(TestCase):
    def test_error_code_enum(self):
        for ec in yogi.ErrorCode:
            prefix = "YOGI_"
            if ec is not yogi.ErrorCode.OK:
                prefix += "ERR_"
            self.assertEnumElementMatches(prefix, ec)

    def test_result(self):
        self.assertFalse(yogi.Result(-1))
        self.assertTrue(yogi.Result(0))
        self.assertTrue(yogi.Result(1))
        self.assertEqual(yogi.Result(10).value, 10)
        self.assertEqual(yogi.Result(3), yogi.Result(3))
        self.assertNotEqual(yogi.Result(2), yogi.Result(3))
        self.assertGreater(len(str(yogi.Result(-3))), 5)
        self.assertNotEqual(hash(yogi.Result(1)), hash(yogi.Result(2)))
        self.assertIsInstance(yogi.Result(-1).error_code, yogi.ErrorCode)
        self.assertEqual(yogi.Result(0).error_code, yogi.Result(1).error_code)

    def test_failure(self):
        self.assertFalse(yogi.Failure(-1))
        self.assertRaises(AssertionError, lambda: yogi.Failure(0))
        self.assertRaises(AssertionError, lambda: yogi.Failure(1))
        self.assertEqual(str(yogi.Failure(-1)), str(yogi.Result(-1)))
        self.assertEqual(yogi.Failure(-1), yogi.Result(-1))
        self.assertIsInstance(yogi.Failure(-1), yogi.Result)

    def test_descriptive_failure(self):
        self.assertEqual(yogi.DescriptiveFailure(-4, "ab").description, "ab")

    def test_success(self):
        self.assertTrue(yogi.Success(1))
        self.assertTrue(yogi.Success(0))
        self.assertRaises(AssertionError, lambda: yogi.Success(-1))
        self.assertEqual(str(yogi.Success(1)), str(yogi.Result(1)))
        self.assertEqual(yogi.Success(1), yogi.Result(1))
        self.assertIsInstance(yogi.Success(1), yogi.Result)


if __name__ == '__main__':
    unittest.main()
