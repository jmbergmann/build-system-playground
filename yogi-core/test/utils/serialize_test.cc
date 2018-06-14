

#include "../common.h"
#include <boost/uuid/uuid_generators.hpp>

#include "../../src/utils/serialize.h"

class SerializeTest : public Test {
 protected:
  template <typename T>
  void Check(T val) {
    utils::ByteVector buffer;
    utils::Serialize(&buffer, val);

    T des_val;
    auto it = buffer.cbegin();
    EXPECT_TRUE(utils::Deserialize(&des_val, buffer, &it));
    EXPECT_EQ(des_val, val);
    EXPECT_EQ(it, buffer.end());

    EXPECT_FALSE(utils::Deserialize(&des_val, buffer, &it));
  }
};

TEST_F(SerializeTest, unsigned_short) {
  Check<unsigned short>(12345);
}

TEST_F(SerializeTest, int) {
  Check<int>(-1234567890);
}

TEST_F(SerializeTest, size_t) {
  Check<size_t>(1234567890);
}

TEST_F(SerializeTest, nanoseconds) {
  Check(std::chrono::nanoseconds(1234567890));
}

TEST_F(SerializeTest, Timestamp) {
  Check(utils::Timestamp::Now());
}

TEST_F(SerializeTest, string) {
  Check<std::string>("Hello");
}

TEST_F(SerializeTest, uuid) {
  Check(boost::uuids::random_generator()());
}
