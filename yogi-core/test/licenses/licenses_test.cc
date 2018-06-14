#include "../common.h"

#include <vector>
#include <string.h>

class LicensesTest : public Test {
 protected:
  std::vector<std::string> GetIndividual3rdPartyLicenses() {
    std::string all = YOGI_Get3rdPartyLicenses();

    std::vector<std::string> licenses;
    std::string::size_type start = 0;
    while (start < all.size()) {
      auto end = all.find("\n\n\n", start);
      licenses.push_back(all.substr(start, end - start));
      start = end + 3;
    }

    return licenses;
  }

  void Check3rdPartyLicense(std::vector<std::string> strings_to_find) {
    auto licenses = GetIndividual3rdPartyLicenses();
    for (auto& license : licenses) {
      std::size_t strings_found = 0;
      for (auto& str : strings_to_find) {
        if (license.find(str) != std::string::npos) {
          ++strings_found;
        }
      }

      if (strings_found == strings_to_find.size()) {
        SUCCEED();
        return;
      }
    }

    FAIL() << "Required strings have not been found in the 3rd party licenses";
  }
};

TEST_F(LicensesTest, YogiLicense) {
  auto license = YOGI_GetLicense();
  EXPECT_NE(strstr(license, "GNU GENERAL PUBLIC LICENSE"), nullptr);
  EXPECT_NE(strstr(license, "Version 3, 29 June 2007"), nullptr);
}

TEST_F(LicensesTest, JsonLicense) {
  Check3rdPartyLicense({
    "JSON for Modern C++",
    "https://github.com/nlohmann/json",
    "Licensed under the MIT License"
  });
}
