# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

import yogi
import unittest
import socket
import os
import uuid

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

    def test_create_with_sub_section(self):
        branch = yogi.Branch(
            self.context, '{"branch":{"name":"Cow"}}', "/branch")
        self.assertEqual(branch.name, "Cow")

    def test_info(self):
        props = {
            "name": "My Branch",
            "description": "Stuff",
            "network_name": "My Network",
            "network_password": "Password",
            "path": "/some/path",
            "interface_address": "192.168.1.123",
            "advertising_address": "239.255.0.1",
            "advertising_port": 12345,
            "advertising_interval": 7,
            "timeout": -1
        }

        branch = yogi.Branch(self.context, props)

        info = branch.info
        self.assertIsInstance(info, yogi.LocalBranchInfo)
        self.assertIsInstance(info.uuid, uuid.UUID)
        self.assertEqual(info.name, "My Branch")
        self.assertEqual(info.description, "Stuff")
        self.assertEqual(info.network_name, "My Network")
        self.assertEqual(info.path, "/some/path")
        self.assertEqual(info.hostname, socket.gethostname())
        self.assertEqual(info.pid, os.getpid())
        self.assertEqual(info.interface_address, "192.168.1.123")
        self.assertEqual(info.advertising_address, "239.255.0.1")
        self.assertEqual(info.advertising_port, 12345)
        self.assertEqual(info.advertising_interval, 7.0)
        self.assertGreater(len(info.tcp_server_address), 1)
        self.assertGreater(info.tcp_server_port, 0)
        self.assertLessEqual(info.start_time, yogi.get_current_time())
        self.assertEqual(info.timeout, float("inf"))
        self.assertEqual(info.ghost_mode, False)
        self.assertGreater(info.tx_queue_size, 1000)
        self.assertGreater(info.rx_queue_size, 1000)

        for key in info._info:
            self.assertEqual(getattr(branch, key), info._info[key])

    def test_get_connected_branches(self):
        branch = yogi.Branch(self.context, '{"name":"My Branch"}')
        branch_a = yogi.Branch(self.context, '{"name":"A"}')
        branch_b = yogi.Branch(self.context, '{"name":"B"}')

        while len(branch.get_connected_branches()) < 2:
            self.context.run_one()
        branches = branch.get_connected_branches()

        for brn in [branch_a, branch_b]:
            self.assertTrue(brn.uuid in branches)
            self.assertEqual(branches[brn.uuid].name, brn.name)
            self.assertIsInstance(branches[brn.uuid], yogi.RemoteBranchInfo)

    def test_await_event(self):
        branch = yogi.Branch(self.context, '{"name":"My Branch"}')
        branch_a = yogi.Branch(self.context, '{"name":"A"}')

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

        branch.await_event_async(yogi.BranchEvents.BRANCH_QUERIED |
                                 yogi.BranchEvents.CONNECTION_LOST, fn)

        while not fn_called:
            self.context.run_one()

        self.assertIsInstance(fn_res, yogi.Success)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.OK)
        self.assertIsInstance(fn_event, yogi.BranchEvents)
        self.assertEqual(fn_event, yogi.BranchEvents.BRANCH_QUERIED)
        self.assertIsInstance(fn_evres, yogi.Success)
        self.assertEqual(fn_evres.error_code, yogi.ErrorCode.OK)
        self.assertIsInstance(fn_info, yogi.BranchQueriedEventInfo)
        self.assertEqual(fn_info.uuid, branch_a.uuid)
        self.assertEqual(fn_info.start_time, branch_a.start_time)
        self.assertEqual(fn_info.timeout, branch_a.timeout)
        self.assertEqual(fn_info.ghost_mode, branch_a.ghost_mode)

    def test_cancel_await_event(self):
        branch = yogi.Branch(self.context, '{"name":"My Branch"}')

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

        branch.await_event_async(yogi.BranchEvents.ALL, fn)
        branch.cancel_await_event()

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
