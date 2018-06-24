import yogi
import unittest

from .common import TestCase


class TestBranches(TestCase):
    def test_branch_events_enum(self):
        for ev in yogi.BranchEvents:
            if ev is yogi.BranchEvents.ALL:
                self.assertFlagCombinationMatches("YOGI_BEV_", ev, [])
            else:
                self.assertFlagMatches("YOGI_BEV_", ev)


if __name__ == '__main__':
    unittest.main()
