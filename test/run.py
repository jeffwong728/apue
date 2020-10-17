import os
import sys
import unittest
import HtmlTestRunner
import argparse
import platform
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('-D', '--Debug', action='store_true', help='Enable debug mode')
parser.add_argument('-r', '--Release', action='store_true', help='Enable release mode')
parser.add_argument('-c', '--Case', help='Run test from name')
parser.add_argument('-f', '--File', help='Run test list from file')
parser.add_argument('-d', '--Directory', help='Run test directory')
args = parser.parse_args()
if platform.system() == "Windows":
    if args.Debug:
        os.add_dll_directory(os.path.join(os.environ["VCPKG_ROOT_DIR"], 'installed', 'x64-windows', 'debug', 'bin'))
    else:
        os.add_dll_directory(os.path.join(os.environ["VCPKG_ROOT_DIR"], 'installed', 'x64-windows', 'bin'))
import extradata

extradata.init()
testLoader = unittest.TestLoader()

tests = None
if args.Case:
    cases = []
    cases.append(args.Case)
    tests = testLoader.loadTestsFromNames(cases)
elif args.Directory:
    tests = testLoader.discover(start_dir=args.Directory, pattern='test_*.py')
else:
    pass

tmplPath = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'report_template.html')
teml_args = {
    'perf' : extradata.perfData,
    'obj_path' : extradata.objPath,
    'obj_paths' : extradata.objPaths
}

if tests:
    outDir = os.path.join(os.environ["SPAM_ROOT_DIR"], 'reports')
    runner = HtmlTestRunner.HTMLTestRunner(combine_reports=True, output=outDir, report_name="mvlab", add_timestamp=False, template=tmplPath, template_args=teml_args)
    runner.run(tests)

if platform.system() == "Windows":
    os.startfile(os.path.join(os.environ["SPAM_ROOT_DIR"], 'reports', 'mvlab.html'))
else:
    opener ="open" if sys.platform == "darwin" else "xdg-open"
    subprocess.call([opener, os.path.join(os.environ["SPAM_ROOT_DIR"], 'reports', 'mvlab.html')])
