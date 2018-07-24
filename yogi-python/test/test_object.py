import yogi

from .common import TestCase


class TestContext(TestCase):
    def test_str(self):
        obj = yogi.Context()
        self.assertRegex(str(obj), "Context \\[[1-9a-f][0-9a-f]+\\]")


if __name__ == '__main__':
    unittest.main()
