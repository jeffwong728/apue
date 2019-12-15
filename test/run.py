import os
import sys
import unittest
import HtmlTestRunner
import extradata

extradata.init()
tests = unittest.TestLoader().discover(start_dir='.', pattern='test_*Diameter*.py')
tmplPath = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'report_template.html')

teml_args = {
    'perf' : extradata.perfData,
    'obj_path' : extradata.objPath
}

runner = HtmlTestRunner.HTMLTestRunner(combine_reports=True, report_name="mvlab", add_timestamp=False, template=tmplPath, template_args=teml_args)
runner.run(tests)

