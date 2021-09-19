from pysys.constants import *
from apama.basetest import ApamaBaseTest
from apama.correlator import CorrelatorHelper


class PySysTest(ApamaBaseTest):
	def execute(self):
		correlator = CorrelatorHelper(self, name='testcorrelator')
		correlator.start(logfile='testcorrelator.log')
		correlator.injectEPL(filenames=['BigNumPlugin.mon'], filedir=PROJECT.APAMA_WORK+'/monitors')
		correlator.injectEPL(filenames=['test.mon'])
		correlator.flush() 

	def validate(self):
		self.assertGrep('testcorrelator.log', expr='ERROR', contains=False)
