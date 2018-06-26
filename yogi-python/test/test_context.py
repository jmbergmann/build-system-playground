import yogi
import unittest
import datetime
import threading

from .common import TestCase


class TestContext(TestCase):
    def setUp(self):
        self.context = yogi.Context()

    def test_poll(self):
        self.assertEqual(self.context.poll(), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.poll(), 2)

    def test_poll_one(self):
        self.assertEqual(self.context.poll_one(), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.poll_one(), 1)

    def test_run(self):
        self.assertEqual(self.context.run(1e-3), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.run(1e-3), 2)

    def test_run_one(self):
        self.assertEqual(self.context.run_one(1e-3), 0)
        self.context.post(lambda: None)
        self.context.post(lambda: None)
        self.assertEqual(self.context.run_one(1e-3), 1)

    def test_run_in_background(self):
        called = False

        def fn():
            nonlocal called
            called = True

        self.context.run_in_background()
        self.context.post(fn)
        while not called:
            pass

    def test_stop(self):
        def thread_fn():
            self.context.wait_for_running()
            self.context.stop()

        thread = threading.Thread(target=thread_fn)
        thread.start()
        self.context.run()
        thread.join()

    def test_wait_for_running_and_stopped(self):
        self.assertTrue(self.context.wait_for_stopped())
        self.assertTrue(self.context.wait_for_stopped(1e-3))
        self.assertFalse(self.context.wait_for_running(1e-3))

        self.context.run_in_background()

        self.assertTrue(self.context.wait_for_running())
        self.assertTrue(self.context.wait_for_running(1e-3))
        self.assertFalse(self.context.wait_for_stopped(1e-3))

        self.context.stop()

        self.assertTrue(self.context.wait_for_stopped())


if __name__ == '__main__':
    unittest.main()
