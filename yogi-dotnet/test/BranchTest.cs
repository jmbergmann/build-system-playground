using System;
using System.Net;
using Xunit;

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
        public void Info()
        {
            var branch = new Yogi.Branch(context, "My Branch", "Stuff", "My Network", "Password",
                                         "/some/path", "239.255.0.1", 12345,
                                         TimeSpan.FromSeconds(7), Yogi.InfiniteTimeSpan);

           Yogi.LocalBranchInfo info = branch.Info;
           Assert.IsType<Guid>(info.Uuid);
           Assert.Equal("My Branch", info.Name);
           Assert.Equal("Stuff", info.Description);
           Assert.Equal("My Network", info.NetName);
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
           Assert.Equal(Yogi.InfiniteTimeSpan, info.Timeout);
        }

        [Fact]
        public void GetConnectedBranches()
        {
            var branch = new Yogi.Branch(context, "My Branch");
            var branch_a = new Yogi.Branch(context, "A");
            var branch_b = new Yogi.Branch(context, "B");

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
        }

        [Fact]
        public void CancelAwaitEvent()
        {
        }
    }
}
