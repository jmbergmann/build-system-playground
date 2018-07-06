using System;
using Xunit;

namespace test
{
    public class BranchTest : TestCase
    {
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
        }

        [Fact]
        public void GetConnectedBranches()
        {
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
