import unittest
import os
import xmlrunner

case_path = os.path.join(os.getcwd())
report_path = os.path.join(os.getcwd(), "report")


def all_case():
   discover = unittest.defaultTestLoader.discover(case_path, pattern="test*.py", top_level_dir=None)
   print(discover)
   return discover

if __name__ == "__main__":
   runner = xmlrunner.XMLTestRunner(output=report_path)
   test_result = runner.run(all_case())
   """
   runner = unittest.TextTestRunner()
   test_result = runner.run(all_case())
   """
   