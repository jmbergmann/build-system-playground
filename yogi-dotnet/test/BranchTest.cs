/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Net;
using Xunit;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace test
{
    public class BranchTest : TestCase
    {
        Yogi.Context context = new Yogi.Context();

        [Fact]
        public void BranchEventsEnum()
        {
            foreach (var elem in GetEnumElements<Yogi.BranchEvents>())
            {
                if (elem == Yogi.BranchEvents.All)
                {
                    AssertFlagCombinationMatches("YOGI_BEV_", elem);
                }
                else
                {
                    AssertFlagMatches("YOGI_BEV_", elem);
                }
            }
        }

        [Fact]
        public void CreateWithSubSection()
        {
            var branch = new Yogi.Branch(context, "{\"branch\":{\"name\":\"Cow\"}}", "/branch");
            Assert.Equal("Cow", branch.Name);
        }

        [Fact]
        public void Info()
        {
            var props = new JObject();
            props["name"] = "My Branch";
            props["description"] = "Stuff";
            props["network_name"] = "My Network";
            props["network_password"] = "Password";
            props["path"] = "/some/path";
            props["advertising_address"] = "239.255.0.1";
            props["advertising_port"] = 12345;
            props["advertising_interval"] = 7;
            props["timeout"] = -1;

            var branch = new Yogi.Branch(context, props);

            var info = branch.Info;
            Assert.IsType<Guid>(info.Uuid);
            Assert.Equal("My Branch", info.Name);
            Assert.Equal("Stuff", info.Description);
            Assert.Equal("My Network", info.NetworkName);
            Assert.Equal("/some/path", info.Path);
            Assert.Equal(Dns.GetHostName(), info.Hostname);
            Assert.Equal(System.Diagnostics.Process.GetCurrentProcess().Id, info.Pid);
            Assert.IsType<IPAddress>(info.AdvertisingAddress);
            Assert.Equal("239.255.0.1", info.AdvertisingAddress.ToString());
            Assert.Equal(12345, info.AdvertisingPort);
            Assert.Equal(7, info.AdvertisingInterval.TotalSeconds, precision: 5);
            Assert.IsType<IPAddress>(info.TcpServerAddress);
            Assert.True(info.TcpServerPort > 0);
            Assert.True(info.StartTime < Yogi.CurrentTime);
            Assert.Equal(Yogi.Duration.Infinity, info.Timeout);
            Assert.False(info.GhostMode);

            Assert.Equal(info.Uuid, branch.Uuid);
            Assert.Equal(info.Name, branch.Name);
            Assert.Equal(info.Description, branch.Description);
            Assert.Equal(info.NetworkName, branch.NetName);
            Assert.Equal(info.Path, branch.Path);
            Assert.Equal(info.Hostname, branch.Hostname);
            Assert.Equal(info.Pid, branch.Pid);
            Assert.Equal(info.AdvertisingAddress, branch.AdvertisingAddress);
            Assert.Equal(info.AdvertisingPort, branch.AdvertisingPort);
            Assert.Equal(info.AdvertisingInterval, branch.AdvertisingInterval);
            Assert.Equal(info.TcpServerAddress, branch.TcpServerAddress);
            Assert.Equal(info.TcpServerPort, branch.TcpServerPort);
            Assert.Equal(info.StartTime, branch.StartTime);
            Assert.Equal(info.Timeout, branch.Timeout);
            Assert.Equal(info.GhostMode, branch.GhostMode);
        }

        [Fact]
        public void GetConnectedBranches()
        {
            var branch = new Yogi.Branch(context, "{\"name\":\"My Branch\"}");
            var branch_a = new Yogi.Branch(context,"{\"name\":\"A\"}");
            var branch_b = new Yogi.Branch(context, "{\"name\":\"B\"}");

            while (!branch.GetConnectedBranches().ContainsKey(branch_a.Uuid)
                || !branch.GetConnectedBranches().ContainsKey(branch_b.Uuid))
            {
                context.RunOne();
            }
            var branches = branch.GetConnectedBranches();

            Assert.True(branches.ContainsKey(branch_a.Uuid));
            Assert.Equal(branches[branch_a.Uuid].Name, branch_a.Name);
            Assert.IsType<Yogi.RemoteBranchInfo>(branches[branch_a.Uuid]);

            Assert.True(branches.ContainsKey(branch_b.Uuid));
            Assert.Equal(branches[branch_b.Uuid].Name, branch_b.Name);
            Assert.IsType<Yogi.RemoteBranchInfo>(branches[branch_b.Uuid]);
        }

        [Fact]
        public void AwaitEvent()
        {
            var branch = new Yogi.Branch(context, "{\"name\":\"My Branch\"}");
            var branch_a = new Yogi.Branch(context, "{\"name\":\"A\"}");

            var events = Yogi.BranchEvents.BranchQueried | Yogi.BranchEvents.ConnectionLost;
            bool called = false;
            branch.AwaitEvent(events, (res, ev, evres, info) => {
                Assert.IsType<Yogi.Success>(res);
                Assert.Equal(Yogi.ErrorCode.Ok, res.ErrorCode);
                Assert.IsType<Yogi.BranchEvents>(ev);
                Assert.Equal(Yogi.BranchEvents.BranchQueried, ev);
                Assert.IsType<Yogi.Success>(evres);
                Assert.Equal(Yogi.ErrorCode.Ok, evres.ErrorCode);
                Assert.IsType<Yogi.BranchQueriedEventInfo>(info);
                Assert.Equal(branch_a.Uuid, info.Uuid);
                Assert.Equal(branch_a.StartTime, (info as Yogi.BranchQueriedEventInfo).StartTime);
                Assert.Equal(branch_a.Timeout, (info as Yogi.BranchQueriedEventInfo).Timeout);
                Assert.Equal(branch_a.GhostMode, (info as Yogi.BranchQueriedEventInfo).GhostMode);
                called = true;
            });

            GC.Collect();

            while (!called)
            {
                context.RunOne();
            }

            Assert.True(called);

            GC.KeepAlive(branch);
            GC.KeepAlive(branch_a);
        }

        [Fact]
        public void CancelAwaitEvent()
        {
            var branch = new Yogi.Branch(context, "{\"name\":\"My Branch\"}");

            bool called = false;
            branch.AwaitEvent(Yogi.BranchEvents.All, (res, ev, evres, info) => {
                Assert.IsType<Yogi.Failure>(res);
                Assert.Equal(Yogi.ErrorCode.Canceled, res.ErrorCode);
                Assert.IsType<Yogi.BranchEvents>(ev);
                Assert.Equal(Yogi.BranchEvents.None, ev);
                Assert.IsType<Yogi.Success>(evres);
                Assert.Equal(Yogi.ErrorCode.Ok, evres.ErrorCode);
                Assert.Null(info);
                called = true;
            });

            branch.CancelAwaitEvent();
            context.Poll();
            Assert.True(called);

            GC.KeepAlive(branch);
        }
    }
}
