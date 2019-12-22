import os
import sys
import unittest
import HtmlTestRunner
import extradata
import argparse

extradata.init()
tests = unittest.TestLoader().discover(start_dir='.', pattern='test_Contour*.py')
tmplPath = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'report_template.html')

teml_args = {
    'perf' : extradata.perfData,
    'obj_path' : extradata.objPath
}

parser = argparse.ArgumentParser()
parser.add_argument('-d', '--debug', action='store_true', help='Enable debug mode')
args = parser.parse_args()
if args.debug:
    os.system('pause')

runner = HtmlTestRunner.HTMLTestRunner(combine_reports=True, report_name="mvlab", add_timestamp=False, template=tmplPath, template_args=teml_args)
runner.run(tests)

