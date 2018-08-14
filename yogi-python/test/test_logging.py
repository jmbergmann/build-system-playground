import yogi
import unittest
import datetime
import tempfile
import shutil
import os.path

from .common import TestCase


class TestLogging(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp_dir = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.temp_dir)

    def tearDown(self):
        yogi.log_to_console(None)
        yogi.log_to_hook(None)
        yogi.log_to_file(None)

    def test_verbosity_enum(self):
        self.assertEnumMatches("YOGI_VB_", yogi.Verbosity)

    def test_stream_enum(self):
        self.assertEnumMatches("YOGI_ST_", yogi.Stream)

    def test_log_to_console(self):
        yogi.log_to_console(yogi.Verbosity.INFO, yogi.Stream.STDERR, True)
        yogi.app_logger.log(yogi.Verbosity.WARNING, "Warning message")
        yogi.log_to_console(yogi.Verbosity.DEBUG, yogi.Stream.STDERR, False,
                            "%S.%3", "$t - $m")
        yogi.app_logger.log(yogi.Verbosity.ERROR, "Error message")

    def test_log_to_hook(self):
        fn_severity = None
        fn_timestamp = None
        fn_tid = None
        fn_file = None
        fn_line = None
        fn_comp = None
        fn_msg = None
        fn_called = False

        def fn(severity, timestamp, tid, file, line, comp, msg):
            nonlocal fn_severity, fn_timestamp, fn_tid, fn_file, fn_line, \
                fn_comp, fn_msg, fn_called
            fn_severity = severity
            fn_timestamp = timestamp
            fn_tid = tid
            fn_file = file
            fn_line = line
            fn_comp = comp
            fn_msg = msg
            fn_called = True

        yogi.log_to_hook(yogi.Verbosity.DEBUG, fn)
        yogi.app_logger.log(yogi.Verbosity.WARNING, "A warning")
        self.assertTrue(fn_called)
        self.assertIsInstance(fn_severity, yogi.Verbosity)
        self.assertEqual(fn_severity, yogi.Verbosity.WARNING)
        self.assertIsInstance(fn_timestamp, yogi.Timestamp)
        self.assertLessEqual(fn_timestamp, yogi.get_current_time())
        self.assertIsInstance(fn_tid, int)
        self.assertGreater(fn_tid, 0)
        self.assertIsInstance(fn_file, str)
        self.assertEqual(fn_file, __file__)
        self.assertIsInstance(fn_line, int)
        self.assertGreater(fn_line, 0)
        self.assertIsInstance(fn_comp, str)
        self.assertEqual(fn_comp, "App")
        self.assertIsInstance(fn_msg, str)
        self.assertEqual(fn_msg, "A warning")

    def test_log_to_file(self):
        file_prefix = os.path.join(self.temp_dir, "logfile_%Y_")

        filename = yogi.log_to_file(yogi.Verbosity.INFO, file_prefix + "1")
        self.assertFalse("%Y" in filename)
        self.assertTrue(os.path.exists(filename))

        filename = yogi.log_to_file(yogi.Verbosity.INFO, file_prefix + "2",
                                    "%S.%3", "$t - $m")
        self.assertFalse("%Y" in filename)
        self.assertTrue(os.path.exists(filename))

    def test_set_components_verbosity(self):
        yogi.Logger.set_components_verbosity("App", yogi.Verbosity.DEBUG)
        self.assertEqual(yogi.app_logger.verbosity, yogi.Verbosity.DEBUG)
        yogi.Logger.set_components_verbosity("App", yogi.Verbosity.INFO)
        self.assertEqual(yogi.app_logger.verbosity, yogi.Verbosity.INFO)

    def test_logger_verbosity(self):
        logger = yogi.Logger("My logger")
        self.assertEqual(logger.verbosity, yogi.Verbosity.INFO)
        logger.verbosity = yogi.Verbosity.FATAL
        self.assertEqual(logger.verbosity, yogi.Verbosity.FATAL)

    def test_logger_log(self):
        logger = yogi.Logger("My logger")

        fn_called = False
        fn_severity = None
        fn_file = None
        fn_line = None
        fn_comp = None
        fn_msg = None

        def fn(severity, timestamp, tid, file, line, comp, msg):
            nonlocal fn_severity, fn_file, fn_line, fn_comp, fn_msg, fn_called
            fn_severity = severity
            fn_file = file
            fn_line = line
            fn_comp = comp
            fn_msg = msg
            fn_called = True

        yogi.log_to_hook(yogi.Verbosity.INFO, fn)
        logger.log(yogi.Verbosity.WARNING, "Hey dude")
        self.assertTrue(fn_called)
        self.assertEqual(fn_severity, yogi.Verbosity.WARNING)
        self.assertEqual(fn_comp, "My logger")
        self.assertEqual(fn_msg, "Hey dude")
        self.assertEqual(fn_file, __file__)
        self.assertGreater(fn_line, 0)

        fn_called = False

        def fn2(severity, timestamp, tid, file, line, comp, msg):
            nonlocal fn_file, fn_line, fn_called
            fn_file = file
            fn_line = line
            fn_called = True

        yogi.log_to_hook(yogi.Verbosity.INFO, fn2)
        logger.log(yogi.Verbosity.WARNING, "Hey dude",
                   file="my file", line=123)
        self.assertTrue(fn_called)
        self.assertEqual(fn_file, "my file")
        self.assertEqual(fn_line, 123)

    def test_app_logger(self):
        self.assertIsInstance(yogi.app_logger, yogi.AppLogger)

        logger = yogi.AppLogger()
        logger.verbosity = yogi.Verbosity.WARNING
        self.assertEqual(yogi.app_logger.verbosity, logger.verbosity)

        logger.verbosity = yogi.Verbosity.TRACE
        self.assertEqual(yogi.app_logger.verbosity, logger.verbosity)


if __name__ == '__main__':
    unittest.main()
