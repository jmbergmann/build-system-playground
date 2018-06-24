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
        now = yogi.get_current_time()
        called = False

        def fn(severity, timestamp, tid, file, line, comp, msg):
            nonlocal called
            called = True

            self.assertIsInstance(severity, yogi.Verbosity)
            self.assertEqual(severity, yogi.Verbosity.WARNING)
            self.assertIsInstance(timestamp, datetime.datetime)
            self.assertGreaterEqual(timestamp, now)
            self.assertIsInstance(tid, int)
            self.assertGreater(tid, 0)
            self.assertIsInstance(file, str)
            self.assertEqual(file, __file__)
            self.assertIsInstance(line, int)
            self.assertGreater(line, 0)
            self.assertIsInstance(comp, str)
            self.assertEqual(comp, "App")
            self.assertIsInstance(msg, str)
            self.assertEqual(msg, "A warning")

        yogi.log_to_hook(yogi.Verbosity.DEBUG, fn)
        yogi.app_logger.log(yogi.Verbosity.WARNING, "A warning")
        self.assertTrue(called)

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

        called = False

        def fn(severity, timestamp, tid, file, line, comp, msg):
            nonlocal called
            called = True

            self.assertEqual(severity, yogi.Verbosity.WARNING)
            self.assertEqual(comp, "My logger")
            self.assertEqual(msg, "Hey dude")
            self.assertEqual(file, __file__)
            self.assertGreater(line, 0)

        yogi.log_to_hook(yogi.Verbosity.INFO, fn)
        logger.log(yogi.Verbosity.WARNING, "Hey dude")
        self.assertTrue(called)

        called = False

        def fn2(severity, timestamp, tid, file, line, comp, msg):
            nonlocal called
            called = True

            self.assertEqual(file, "my file")
            self.assertEqual(line, 123)

        yogi.log_to_hook(yogi.Verbosity.INFO, fn2)
        logger.log(yogi.Verbosity.WARNING, "Hey dude", file="my file", line=123)
        self.assertTrue(called)

    def test_app_logger(self):
        self.assertIsInstance(yogi.app_logger, yogi.AppLogger)

        logger = yogi.AppLogger()
        logger.verbosity = yogi.Verbosity.WARNING
        self.assertEqual(yogi.app_logger.verbosity, logger.verbosity)

        logger.verbosity = yogi.Verbosity.TRACE
        self.assertEqual(yogi.app_logger.verbosity, logger.verbosity)


if __name__ == '__main__':
    unittest.main()
