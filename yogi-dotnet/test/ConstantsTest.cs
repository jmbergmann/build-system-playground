using System;
using System.Reflection;
using Xunit;

namespace test
{
    public class ConstantsTest
    {
        [Fact]
        public void VersionNumber()
        {
            Assert.IsType<string>(Yogi.Constants.VersionNumber);
            Assert.Equal(Yogi.Constants.VersionNumber, Yogi.GetVersion());
        }

        [Fact]
        public void VersionMajor()
        {
            Assert.IsType<int>(Yogi.Constants.VersionMajor);
            Assert.True(Yogi.Constants.VersionMajor >= 0);
        }

        [Fact]
        public void VersionMinor()
        {
            Assert.IsType<int>(Yogi.Constants.VersionMinor);
            Assert.True(Yogi.Constants.VersionMinor >= 0);
        }

        [Fact]
        public void VersionPatch()
        {
            Assert.IsType<int>(Yogi.Constants.VersionPatch);
            Assert.True(Yogi.Constants.VersionPatch >= 0);
        }

        [Fact]
        public void DefaultAdvAddress()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultAdvAddress);
            Assert.True(Yogi.Constants.DefaultAdvAddress.Length > 1);
        }

        [Fact]
        public void DefaultAdvPort()
        {
            Assert.IsType<int>(Yogi.Constants.DefaultAdvPort);
            Assert.True(Yogi.Constants.DefaultAdvPort >= 1024);
        }

        [Fact]
        public void DefaultAdvInterval()
        {
            Assert.IsType<TimeSpan>(Yogi.Constants.DefaultAdvInterval);
            Assert.True(Yogi.Constants.DefaultAdvInterval.TotalSeconds > 0.5);
        }

        [Fact]
        public void DefaultConnectionTimeout()
        {
            Assert.IsType<TimeSpan>(Yogi.Constants.DefaultConnectionTimeout);
            Assert.True(Yogi.Constants.DefaultConnectionTimeout.TotalSeconds > 0.5);
        }

        [Fact]
        public void DefaultLoggerVerbosity()
        {
            Assert.IsType<Yogi.Verbosity>(Yogi.Constants.DefaultLoggerVerbosity);
        }

        [Fact]
        public void DefaultLogTimeFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultLogTimeFormat);
            Assert.True(Yogi.Constants.DefaultLogTimeFormat.Length > 1);
        }

        [Fact]
        public void DefaultLogFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultLogFormat);
            Assert.True(Yogi.Constants.DefaultLogFormat.Length > 1);
        }

        [Fact]
        public void MaxMessageSize()
        {
            Assert.IsType<int>(Yogi.Constants.MaxMessageSize);
            Assert.True(Yogi.Constants.MaxMessageSize >= 1000);
        }
    }
}
