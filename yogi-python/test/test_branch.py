import yogi
import unittest
import socket
import os

from .common import TestCase


class TestBranches(TestCase):
    def setUp(self):
        self.context = yogi.Context()

    def test_branch_events_enum(self):
        for ev in yogi.BranchEvents:
            if ev is yogi.BranchEvents.ALL:
                self.assertFlagCombinationMatches("YOGI_BEV_", ev, [])
            else:
                self.assertFlagMatches("YOGI_BEV_", ev)

    def test_info(self):
        branch = yogi.Branch(self.context, "My Branch", "Stuff", "My Network",
                             "Password", "/some/path", "239.255.0.1", 12345,
                             7.0, float("inf"))
        info = branch.info
        self.assertEqual(len(info["uuid"]), 36)
        self.assertEqual(info["name"], "My Branch")
        self.assertEqual(info["description"], "Stuff")
        self.assertEqual(info["net_name"], "My Network")
        self.assertEqual(info["path"], "/some/path")
        self.assertEqual(info["hostname"], socket.gethostname())
        self.assertEqual(info["pid"], os.getpid())
        self.assertEqual(info["advertising_address"], "239.255.0.1")
        self.assertEqual(info["advertising_port"], 12345)
        self.assertEqual(info["advertising_interval"], 7.0)
        self.assertGreater(len(info["tcp_server_address"]), 1)
        self.assertGreater(info["tcp_server_port"], 0)
        self.assertLessEqual(info["start_time"], yogi.get_current_time())
        self.assertEqual(info["timeout"], float("inf"))

        for key in info:
            self.assertEqual(getattr(branch, key), info[key])

    def test_get_connected_branches(self):
        branch = yogi.Branch(self.context, "My Branch")
        branch_a = yogi.Branch(self.context, "A")
        branch_b = yogi.Branch(self.context, "B")

        while len(branch.get_connected_branches()) < 2:
            self.context.run_one()
        branches = branch.get_connected_branches()

        for brn in [branch_a, branch_b]:
            self.assertTrue(brn.uuid in branches)
            self.assertEqual(branches[brn.uuid]["name"], brn.name)
            self.assertGreater(len(branches[brn.uuid]), 10)

    def test_await_event(self):
        branch = yogi.Branch(self.context, "My Branch")
        branch_a = yogi.Branch(self.context, "A")

        fn_res = None
        fn_event = None
        fn_evres = None
        fn_info = None
        fn_called = False

        def fn(res, event, evres, info):
            nonlocal fn_res, fn_event, fn_evres, fn_info, fn_called
            fn_res = res
            fn_event = event
            fn_evres = evres
            fn_info = info
            fn_called = True

        branch.await_event(yogi.BranchEvents.BRANCH_QUERIED, fn)

        while not fn_called:
            self.context.run_one()

        self.assertIsInstance(fn_res, yogi.Success)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.OK)
        self.assertIsInstance(fn_event, yogi.BranchEvents)
        self.assertEqual(fn_event, yogi.BranchEvents.BRANCH_QUERIED)
        self.assertIsInstance(fn_evres, yogi.Success)
        self.assertEqual(fn_evres.error_code, yogi.ErrorCode.OK)
        self.assertIsInstance(fn_info, dict)
        self.assertEqual(fn_info["uuid"], branch_a.uuid)
        self.assertEqual(fn_info["start_time"], branch_a.start_time)
        self.assertEqual(fn_info["timeout"], branch_a.timeout)


    def test_cancel_await_event(self):
        branch = yogi.Branch(self.context, "My Branch")

        fn_res = None
        fn_event = None
        fn_evres = None
        fn_info = None
        fn_called = False

        def fn(res, event, evres, info):
            nonlocal fn_res, fn_event, fn_evres, fn_info, fn_called
            fn_res = res
            fn_event = event
            fn_evres = evres
            fn_info = info
            fn_called = True

        branch.await_event(yogi.BranchEvents.ALL, fn)
        branch.cancel_await_event()

        while not fn_called:
            self.context.poll()

        self.assertIsInstance(fn_res, yogi.Failure)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.CANCELED)
        self.assertIsInstance(fn_event, yogi.BranchEvents)
        self.assertEqual(fn_event, yogi.BranchEvents.NONE)
        self.assertIsInstance(fn_evres, yogi.Success)
        self.assertEqual(fn_evres.error_code, yogi.ErrorCode.OK)
        self.assertEqual(fn_info, None)


if __name__ == '__main__':
    unittest.main()
