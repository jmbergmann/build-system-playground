using System;
using System.Threading;
using Xunit;

namespace test
{
    public class ContextTest : TestCase
    {
        Yogi.Context context = new Yogi.Context();

        public new void Dispose()
        {
            context.Stop();
            context.WaitForStopped();
            context.Poll();
            base.Dispose();
        }

        [Fact]
        public void Poll()
        {
            Assert.Equal(0, context.Poll());
            context.Post(() => {});
            context.Post(() => {});
            Assert.Equal(2, context.Poll());
        }

        [Fact]
        public void PollOne()
        {
            Assert.Equal(0, context.PollOne());
            context.Post(() => {});
            context.Post(() => {});
            Assert.Equal(1, context.PollOne());
        }

        [Fact]
        public void Run()
        {
            Assert.Equal(0, context.Run(TimeSpan.FromMilliseconds(1)));
            context.Post(() => {});
            context.Post(() => {});
            Assert.Equal(2, context.Run(TimeSpan.FromMilliseconds(1)));
        }

        [Fact]
        public void RunOne()
        {
            Assert.Equal(0, context.RunOne(TimeSpan.FromMilliseconds(1)));
            context.Post(() => {});
            context.Post(() => {});
            Assert.Equal(1, context.RunOne(TimeSpan.FromMilliseconds(1)));
        }

        [Fact]
        public void RunInBackground()
        {
            var ev = new AutoResetEvent(false);
            context.RunInBackground();
            context.Post(() => {
                ev.Set();
            });
            ev.WaitOne();
        }

        [Fact]
        public void Stop()
        {
            var th = new Thread(() => {
                context.WaitForRunning();
                context.Stop();
            });
            th.Start();
            context.Run();
            th.Join();
        }

        [Fact]
        public void WaitForRunningAndStopped()
        {
            Assert.True(context.WaitForStopped());
            Assert.True(context.WaitForStopped(TimeSpan.FromMilliseconds(1)));
            Assert.False(context.WaitForRunning(TimeSpan.FromMilliseconds(1)));

            context.RunInBackground();

            Assert.True(context.WaitForRunning());
            Assert.True(context.WaitForRunning(TimeSpan.FromMilliseconds(1)));
            Assert.False(context.WaitForStopped(TimeSpan.FromMilliseconds(1)));

            context.Stop();

            Assert.True(context.WaitForStopped());
        }
    }
}
