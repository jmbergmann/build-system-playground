using System;
using Xunit;

namespace test
{
    public class ConstantsTest : TestCase
    {
        [Fact]
        public void VersionNumber()
        {
            Assert.IsType<string>(Yogi.Constants.VersionNumber);
            Assert.Equal(Yogi.Constants.VersionNumber, GetCoreMacroString("YOGI_HDR_VERSION"));
        }

        [Fact]
        public void VersionMajor()
        {
            Assert.IsType<int>(Yogi.Constants.VersionMajor);
            Assert.Equal(Yogi.Constants.VersionMajor, GetCoreMacroInt("YOGI_HDR_VERSION_MAJOR"));
        }

        [Fact]
        public void VersionMinor()
        {
            Assert.IsType<int>(Yogi.Constants.VersionMinor);
            Assert.Equal(Yogi.Constants.VersionMinor, GetCoreMacroInt("YOGI_HDR_VERSION_MINOR"));
        }

        [Fact]
        public void VersionPatch()
        {
            Assert.IsType<int>(Yogi.Constants.VersionPatch);
            Assert.Equal(Yogi.Constants.VersionPatch, GetCoreMacroInt("YOGI_HDR_VERSION_PATCH"));
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

        [Fact]
        public void DefaultTimeFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultTimeFormat);
            Assert.Contains(".%3", Yogi.Constants.DefaultTimeFormat);
        }

        [Fact]
        public void DefaultInfiniteDurationString()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultInfiniteDurationString);
            Assert.Contains("inf", Yogi.Constants.DefaultInfiniteDurationString);
        }

        [Fact]
        public void DefaultDurationFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultDurationFormat);
            Assert.Contains(".%3", Yogi.Constants.DefaultDurationFormat);
        }

        [Fact]
        public void DefaultInvalidHandleString()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultInvalidHandleString);
            Assert.Contains("INVALID", Yogi.Constants.DefaultInvalidHandleString);
        }

        [Fact]
        public void DefaultObjectFormat()
        {
            Assert.IsType<string>(Yogi.Constants.DefaultObjectFormat);
            Assert.Contains("$T", Yogi.Constants.DefaultObjectFormat);
        }
    }
}
