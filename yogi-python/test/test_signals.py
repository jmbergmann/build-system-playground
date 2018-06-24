import yogi
import unittest

from .common import TestCase


class TestSignals(TestCase):
    def test_signals_enum(self):
        for sig in yogi.Signals:
            if sig is yogi.Signals.ALL:
                self.assertFlagCombinationMatches("YOGI_SIG_", sig, [])
            else:
                self.assertFlagMatches("YOGI_SIG_", sig)


if __name__ == '__main__':
    unittest.main()
