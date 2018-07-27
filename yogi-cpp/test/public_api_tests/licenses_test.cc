#include "../common.h"

TEST(LicensesTest, GetLicense) {
  EXPECT_GT(yogi::GetLicense().size(), 100);
}

TEST(LicensesTest, Get3rdPartyLicenses) {
  EXPECT_GT(yogi::Get3rdPartyLicenses().size(), 100);
  EXPECT_NE(yogi::Get3rdPartyLicenses(), yogi::GetLicense());
}
