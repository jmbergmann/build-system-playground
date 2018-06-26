import yogi
import unittest
import datetime

from .common import TestCase


class TestTimer(TestCase):
    def setUp(self):
        self.context = yogi.Context()
        self.timer = yogi.Timer(self.context)

    def test_start(self):
        fn_res = None
        fn_called = False

        def fn(res):
            nonlocal fn_res, fn_called
            fn_res = res
            fn_called = True

        self.timer.start(datetime.timedelta(microseconds=1000), fn)
        while not fn_called:
            self.context.run_one()

        self.assertIsInstance(fn_res, yogi.Success)
        self.assertEqual(fn_res, yogi.Success())

    def test_cancel(self):
        fn_res = None
        fn_called = False

        def fn(res):
            nonlocal fn_res, fn_called
            fn_res = res
            fn_called = True

        self.assertFalse(self.timer.cancel())
        self.timer.start(None, fn)
        self.assertTrue(self.timer.cancel())
        while not fn_called:
            self.context.run_one()

        self.assertFalse(self.timer.cancel())
        self.assertIsInstance(fn_res, yogi.Failure)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.CANCELED)


if __name__ == '__main__':
    unittest.main()
