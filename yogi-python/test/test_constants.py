import yogi
import unittest

class TestConstants(unittest.TestCase):
    def test_version_number(self):
        c = yogi.Constants.VERSION_NUMBER
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 4)
        self.assertEqual(c, "{}.{}.{}".format(yogi.Constants.VERSION_MAJOR, yogi.Constants.VERSION_MINOR, yogi.Constants.VERSION_PATCH))

    def test_version_major(self):
        c = yogi.Constants.VERSION_MAJOR
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 0)

    def test_version_minor(self):
        c = yogi.Constants.VERSION_MINOR
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 0)

    def test_version_patch(self):
        c = yogi.Constants.VERSION_PATCH
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 0)

    def test_default_adv_address(self):
        c = yogi.Constants.DEFAULT_ADV_ADDRESS
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 1)

    def test_default_adv_port(self):
        c = yogi.Constants.DEFAULT_ADV_PORT
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 1024)

    def test_default_adv_interval(self):
        c = yogi.Constants.DEFAULT_ADV_INTERVAL
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 1000000)

    def test_default_connection_timeout(self):
        c = yogi.Constants.DEFAULT_CONNECTION_TIMEOUT
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 1000000)

    def test_default_logger_verbosity(self):
        c = yogi.Constants.DEFAULT_LOGGER_VERBOSITY
        self.assertIsInstance(c, yogi.Verbosity)

    def test_default_log_time_format(self):
        c = yogi.Constants.DEFAULT_LOG_TIME_FORMAT
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 1)

    def test_default_log_format(self):
        c = yogi.Constants.DEFAULT_LOG_TIME_FORMAT
        self.assertIsInstance(c, str)
        self.assertGreater(len(c), 5)

    def test_max_message_size(self):
        c = yogi.Constants.DEFAULT_ADV_INTERVAL
        self.assertIsInstance(c, int)
        self.assertGreaterEqual(c, 1000)

if __name__ == '__main__':
    unittest.main()
