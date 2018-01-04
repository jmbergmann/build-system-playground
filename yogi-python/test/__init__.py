import os
import platform

if platform.system() == 'Windows':
    filename = 'yogi-core.dll'
else:
    filename = 'libyogi-core.so'
    
os.environ['YOGI_CORE_LIBRARY'] = os.path.realpath(os.path.dirname(__file__) + '../../../build/yogi-core/' + filename)

