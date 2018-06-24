import yogi
import unittest

from .common import TestCase


class TestConfigurations(TestCase):
    def test_configuration_options_enum(self):
        self.assertEnumMatches("YOGI_CFG_", yogi.ConfigurationOptions)

    def test_command_line_options_enum(self):
        for opt in yogi.CommandLineOptions:
            if opt is yogi.CommandLineOptions.ALL:
                self.assertFlagCombinationMatches("YOGI_CLO_", opt, [])
            elif opt is yogi.CommandLineOptions.BRANCH_ALL:
                exc = [x for x in yogi.CommandLineOptions
                       if not x.name.startswith("BRANCH_")]
                self.assertFlagCombinationMatches("YOGI_CLO_", opt, exc)
            else:
                self.assertFlagMatches("YOGI_CLO_", opt)


if __name__ == '__main__':
    unittest.main()
