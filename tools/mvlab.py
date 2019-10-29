import sys
import importlib
del sys.modules['mvlab']
sys.modules['mvlab'] = importlib.import_module('cv2.mvlab')