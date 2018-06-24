import yogi
import unittest

from .common import get_yogi_core_macro


class TestErrors(unittest.TestCase):
    def test_error_codes(self):
        for ec in yogi.ErrorCode:
            if ec == yogi.ErrorCode.OK:
                macro_name = "YOGI_" + ec.name
            else:
                macro_name = "YOGI_ERR_" + ec.name
            macro_val = get_yogi_core_macro(macro_name)
            self.assertEqual(ec, macro_val)

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

    def test_success(self):
        self.assertTrue(yogi.Success(1))
        self.assertTrue(yogi.Success(0))
        self.assertRaises(AssertionError, lambda: yogi.Success(-1))
        self.assertEqual(str(yogi.Success(1)), str(yogi.Result(1)))
        self.assertEqual(yogi.Success(1), yogi.Result(1))
        self.assertIsInstance(yogi.Success(1), yogi.Result)


if __name__ == '__main__':
    unittest.main()
