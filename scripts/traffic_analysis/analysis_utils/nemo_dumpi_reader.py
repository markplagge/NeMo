from collections import defaultdict

import numpy as np

from analysis_utils import MultiFileWorker


def dumpi_parser(file):
    pass

def read_dumpi_files(file_list):
    pass

class MultiFileDumpi(MultiFileWorker):

    def line_parser(self,lines):

        counts = defaultdict(int)
        connections = {}

        for l in lines:
            _,line = l.split(',')
            sl  = line.split(' ')
            type = sl[0]
            source = sl[1]
            dest = sl[2]
            if "recv" in type:
                x = source
                source = dest
                dest = x

            src = f"{source}"
            dst = f"{dest}"
            connections[src] = dst
            counts[src] += 1
        return (counts, connections)







