using System;
using Xunit;

namespace test
{
    public class ErrorsTest
    {
        [Fact]
        public void ErrorCodeEnum()
        {
            foreach (int ec in Enum.GetValues(typeof(Yogi.ErrorCode)))
            {
                var prefix = "YOGI_";
                if (ec != (int)Yogi.ErrorCode.Ok) prefix += "ERR_";
                Helpers.AssertEnumElementMatches<Yogi.ErrorCode>(prefix, ec);
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
            Assert.False(new Yogi.Failure(-1));
            Assert.Equal(new Yogi.Failure(-1).ToString(), new Yogi.Result(-1).ToString());
            Assert.Equal(new Yogi.Failure(-1), new Yogi.Result(-1));
            Assert.True(typeof(Yogi.Failure).IsSubclassOf(typeof(Yogi.Result)));
        }

        [Fact]
        public void DescriptiveFailure()
        {
            Assert.Equal("ab", new Yogi.DescriptiveFailure(-4, "ab").Description);
            Assert.True(typeof(Yogi.DescriptiveFailure).IsSubclassOf(typeof(Yogi.Failure)));
        }

        [Fact]
        public void Exception()
        {
            Assert.True(typeof(Yogi.Exception).IsSubclassOf(typeof(System.Exception)));

            var failure = new Yogi.DescriptiveFailure(-4, "ab");
            var exception = new Yogi.Exception(failure);
            Assert.Equal(failure, exception.Failure);
            Assert.Equal(exception.Message, failure.ToString());
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
