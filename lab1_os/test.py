#!/usr/bin/env python3

import os
import pathlib
import re
import stat
import subprocess

import unittest

PROG = os.getenv('PROG')

def execute(args):
    p = subprocess.run(
        [PROG, *args],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return p.stdout.decode(), p.stderr.decode(), p.returncode


def match_parent_pids(lines):
    return re.findall(r'\[PARENT\] getpid\(\)=(\d+), getppid\(\)=(\d+)', lines)

def match_child_pids(lines):
    return re.findall(r'\[CHILD\] getpid\(\)=(\d+), getppid\(\)=(\d+)', lines)


class TestLab1(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(PROG, "Run with PROG=./a.out ./test.py")

    def test_help_flag(self):
        stdout, stderr, rc = execute(['--help'])
        self.assertEqual(rc, 0)
        self.assertEqual(stderr, '')
        self.assertEqual(stdout, 'Usage: {} filename\n'.format(PROG))

    def test_no_args(self):
        stdout, stderr, rc = execute([])
        self.assertEqual(rc, 1)
        self.assertEqual(stdout, '')
        self.assertEqual(stderr, 'Usage: {} filename\n'.format(PROG))

    def test_many_args(self):
        stdout, stderr, rc = execute(['a', 'b'])
        self.assertEqual(rc, 1)
        self.assertEqual(stdout, '')
        self.assertEqual(stderr, 'Usage: {} filename\n'.format(PROG))

    def test_simple(self):
        try:
            os.remove('output.txt')
        except FileNotFoundError:
            pass

        stdout, stderr, rc = execute(['output.txt'])
        self.assertEqual(rc, 0)
        self.assertEqual(stdout, '')
        self.assertEqual(stderr, '')
        self.assertEqual(stat.filemode(os.stat('output.txt').st_mode), "-rw-r--r--", "Incorrect permissions for output.txt")

        with open('output.txt') as fin:
            contents = fin.read()

        os.unlink('output.txt')

        try:
            [(ppid, _)] = match_parent_pids(contents)
        except Exception:
            self.fail("Could not parse parent PIDs line in output.txt")

        try:
            [(_, cppid)] = match_child_pids(contents)
        except Exception:
            self.fail("Could not parse child PIDs line in output.txt")

        self.assertEqual(cppid, ppid, "Parent PID does not match")

    def test_file_exists(self):
        with open('output.txt', 'w') as fout:
            fout.write('original contents')

        stdout, stderr, rc = execute(['output.txt'])
        self.assertEqual(rc, 1)
        self.assertEqual(stdout, '')
        self.assertEqual(stderr, 'Error: output.txt already exists\n')

        os.unlink('output.txt')




if __name__ == "__main__":
    unittest.main()
