using System;
using System.Net;
using Xunit;

namespace test
{
    public class TimerTest : TestCase
    {
        Yogi.Context context = new Yogi.Context();

        [Fact]
        public void Start()
        {
            Yogi.Timer timer = new Yogi.Timer(context);

            bool called = false;
            timer.Start(TimeSpan.FromMilliseconds(1), (res) => {
                Assert.IsType<Yogi.Success>(res);
                Assert.Equal(Yogi.ErrorCode.Ok, res.ErrorCode);
                called = true;
            });

            GC.Collect();

            while (!called)
            {
                context.RunOne();
            }

            Assert.True(called);

            GC.KeepAlive(timer);
        }

        [Fact]
        public void Cancel()
        {
            Yogi.Timer timer = new Yogi.Timer(context);

            Assert.False(timer.Cancel());

            bool called = false;
            timer.Start(Yogi.InfiniteTimeSpan, (res) => {
                Assert.IsType<Yogi.Failure>(res);
                Assert.Equal(Yogi.ErrorCode.Canceled, res.ErrorCode);
                called = true;
            });

            GC.Collect();
            Assert.True(timer.Cancel());
            GC.Collect();

            while (!called)
            {
                context.RunOne();
            }

            Assert.True(called);

            GC.KeepAlive(timer);
        }
    }
}
