import yogi
import unittest

class TestLicenses(unittest.TestCase):
    def test_license(self):
        lic = yogi.get_license()
        self.assertIsInstance(lic, str)
        self.assertGreater(len(lic), 100)

    def test_3rd_party_licenses(self):
        lic = yogi.get_3rd_party_licenses()
        self.assertIsInstance(lic, str)
        self.assertGreater(len(lic), 100)
        self.assertNotEqual(yogi.get_license(), lic)

if __name__ == '__main__':
    unittest.main()
