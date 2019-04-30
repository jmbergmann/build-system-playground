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
using Xunit;

namespace test
{
    public class OperationIdTest : TestCase
    {
        [Fact]
        public void DefaultValue()
        {
            var id = new Yogi.OperationId();
            Assert.Equal(id, new Yogi.OperationId(0));
        }

        [Fact]
        public void Constructor()
        {
            var id = new Yogi.OperationId(5);
            Assert.Equal(5, id.Value);
            Assert.True(id.IsValid);

            id = new Yogi.OperationId(-12);
            Assert.Equal(-12, id.Value);
            Assert.False(id.IsValid);

            id = new Yogi.OperationId(0);
            Assert.Equal(0, id.Value);
            Assert.False(id.IsValid);
        }

        [Fact]
        public void Comparison()
        {
            var id1 = new Yogi.OperationId(11);
            var id2 = new Yogi.OperationId(11);
            var id3 = new Yogi.OperationId(333);

            Assert.True(id1 == id2);
            Assert.False(id1 == id3);
            Assert.False(id1 == null);

            Assert.False(id1 != id2);
            Assert.True(id1 != id3);
            Assert.True(id1 != null);

            Assert.False(id1.Equals(new Exception()));
            Assert.False(id1.Equals(null));
            Assert.True(id1.Equals(id2));
            Assert.False(id1.Equals(id3));
        }
    }
}
