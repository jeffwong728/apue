import os
import sys
import unittest
import HtmlTestRunner
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-d', '--Debug', action='store_true', help='Enable debug mode')
parser.add_argument('-r', '--Release', action='store_true', help='Enable release mode')
args = parser.parse_args()
if args.Debug:
    os.add_dll_directory(os.path.join(os.environ["VCPKG_ROOT_DIR"], 'installed', 'x64-windows', 'debug', 'bin'))
else:
    os.add_dll_directory(os.path.join(os.environ["VCPKG_ROOT_DIR"], 'installed', 'x64-windows', 'bin'))
import extradata

extradata.init()
testLoader = unittest.TestLoader()
cases = []
cases.append('morphology.test_Dilation')
cases.append('morphology.test_Erosion')
cases.append('morphology.test_Opening')
cases.append('morphology.test_Closing')
tests = testLoader.loadTestsFromNames(cases)
#tests = testLoader.discover(start_dir='.', pattern='test_*.py')
tmplPath = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'report_template.html')

teml_args = {
    'perf' : extradata.perfData,
    'obj_path' : extradata.objPath,
    'obj_paths' : extradata.objPaths
}

runner = HtmlTestRunner.HTMLTestRunner(combine_reports=True, report_name="mvlab", add_timestamp=False, template=tmplPath, template_args=teml_args)
runner.run(tests)

os.startfile(os.path.join(os.environ["SPAM_ROOT_DIR"], 'reports', 'mvlab.html'))
