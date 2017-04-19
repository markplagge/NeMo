import re
import jsoncomment.package.comments as comments
import copy
import json

import api_def
from api_def import TN


class tnModel():
	def init(self, filename):
		self.filename = filename



	def loadFile(self):