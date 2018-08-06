using System;
using Xunit;

namespace test
{
    public class ErrorsTest : TestCase
    {
        [Fact]
        public void ErrorCodeEnum()
        {
            foreach (var elem in GetEnumElements<Yogi.ErrorCode>())
            {
                var prefix = (elem == Yogi.ErrorCode.Ok) ? "YOGI_" : "YOGI_ERR_";
                AssertEnumElementMatches(prefix, elem);
            }
        }

        [Fact]
        public void Result()
        {
            Assert.False(new Yogi.Result(-1));
            Assert.True(new Yogi.Result(0));
            Assert.True(new Yogi.Result(1));
            Assert.Equal(10, new Yogi.Result(10).Value);
            Assert.Equal(new Yogi.Result(3), new Yogi.Result(3));
            Assert.NotEqual(new Yogi.Result(2), new Yogi.Result(3));
            Assert.True(new Yogi.Result(-3).ToString().Length > 5);
            Assert.NotEqual(new Yogi.Result(1).GetHashCode(), new Yogi.Result(2).GetHashCode());
            Assert.IsType<Yogi.ErrorCode>(new Yogi.Result(-1).ErrorCode);
            Assert.Equal(new Yogi.Result(0).ErrorCode, new Yogi.Result(1).ErrorCode);
        }

        [Fact]
        public void Failure()
        {
            Assert.False(new Yogi.Failure(Yogi.ErrorCode.Busy));
            Assert.Equal(new Yogi.Failure(Yogi.ErrorCode.Busy).ToString(), new Yogi.Result(Yogi.ErrorCode.Busy).ToString());
            Assert.Equal(new Yogi.Failure(Yogi.ErrorCode.Busy), new Yogi.Result(Yogi.ErrorCode.Busy));
            Assert.True(typeof(Yogi.Failure).IsSubclassOf(typeof(Yogi.Result)));
        }

        [Fact]
        public void DescriptiveFailure()
        {
            Assert.Equal("ab", new Yogi.DescriptiveFailure(Yogi.ErrorCode.Busy, "ab").Description);
            Assert.True(typeof(Yogi.DescriptiveFailure).IsSubclassOf(typeof(Yogi.Failure)));
        }

        [Fact]
        public void Exception()
        {
            Assert.True(typeof(Yogi.Exception).IsSubclassOf(typeof(System.Exception)));
        }

        [Fact]
        public void FailureException()
        {
            Assert.True(typeof(Yogi.FailureException).IsSubclassOf(typeof(Yogi.Exception)));
            Assert.Equal(new Yogi.FailureException(Yogi.ErrorCode.Busy).Message,
                new Yogi.Failure(Yogi.ErrorCode.Busy).ToString());
        }

        [Fact]
        public void DescriptiveFailureException()
        {
            Assert.True(typeof(Yogi.DescriptiveFailureException)
                .IsSubclassOf(typeof(Yogi.FailureException)));
            Assert.Equal(new Yogi.DescriptiveFailureException(Yogi.ErrorCode.Busy, "ab").Message,
                new Yogi.DescriptiveFailure(Yogi.ErrorCode.Busy, "ab").ToString());
        }

        [Fact]
        public void Success()
        {
            Assert.True(new Yogi.Success(0));
            Assert.True(new Yogi.Success(1));
            Assert.Equal(new Yogi.Success(1).ToString(), new Yogi.Result(1).ToString());
            Assert.Equal(new Yogi.Success(1), new Yogi.Result(1));
            Assert.True(typeof(Yogi.Success).IsSubclassOf(typeof(Yogi.Result)));
        }
    }
}
