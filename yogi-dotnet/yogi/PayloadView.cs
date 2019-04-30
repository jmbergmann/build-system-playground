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
using System.Text;
using System.Linq;
using MessagePack;
using System.Diagnostics;

public static partial class Yogi
{
    /// <summary>
    /// Possible data/payload encoding types.
    /// </summary>
    public enum EncodingType
    {
        /// <summary>Data is encoded as JSON.</summary>
        Json = 0,

        /// <summary>Data is encoded as MessagePack.</summary>
        Msgpack = 1
    }

    /// <summary>
    /// Helper class for passing and converting different types of user payload.
    /// </summary>
    public class PayloadView
    {
        /// <summary>
        /// Constructs a view from a buffer.
        /// </summary>
        /// <param name="data">Buffer to use.</param>
        public PayloadView(byte[] data, EncodingType enc)
        {
            Data = data;
            Encoding = enc;

            Debug.Assert(enc != EncodingType.Json || data[data.Length - 1] == 0);
        }

        /// <summary>
        /// Constructs a view from JSON data.
        /// </summary>
        /// <param name="json">The JSON data.</param>
        public PayloadView(JsonView json)
        : this(json.Data, EncodingType.Json)
        {
        }

        public static implicit operator PayloadView(JsonView json)
        {
            return new PayloadView(json);
        }

        /// <summary>
        /// Constructs a view from MessagePack data.
        /// </summary>
        /// <param name="msgpack">The MessagePack data.</param>
        public PayloadView(MsgpackView msgpack)
        : this(msgpack.Data, EncodingType.Msgpack)
        {
        }

        public static implicit operator PayloadView(MsgpackView msgpack)
        {
            return new PayloadView(msgpack);
        }

        public static bool operator ==(PayloadView lhs, PayloadView rhs)
        {
            if (object.ReferenceEquals(lhs, null))
            {
                return object.ReferenceEquals(rhs, null);
            }

            return lhs.Equals(rhs);
        }

        public static bool operator !=(PayloadView lhs, PayloadView rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is PayloadView))
            {
                return false;
            }

            var other = (PayloadView)obj;
            return Encoding == other.Encoding && Data.SequenceEqual(other.Data);
        }

        public override int GetHashCode()
        {
            return Data.GetHashCode() * 17 + Encoding.GetHashCode();
        }

        /// <summary>
        /// Buffer holding the payload data.
        /// </summary>
        public byte[] Data { get; }

        /// <summary>
        /// Size of the payload data in bytes.
        /// </summary>
        public int Size
        {
            get { return Data == null ? 0 : Data.Length; }
        }

        /// <summary>
        /// Encoding of the payload data.
        /// </summary>
        public EncodingType Encoding { get; }
    }
}
