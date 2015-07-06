__author__ = 'Mark'
import shlex, subprocess
import threading
from multiprocessing.pool import ThreadPool


returnVals = []

class ThreadWorker():
    '''
    The basic idea is given a function create an object.
    The object can then run the function in a thread.
    It provides a wrapper to start it,check its status,and get data out the function.
    '''
    def __init__(self,func):
        self.thread = None
        self.data = None
        self.func = self.save_data(func)

    def save_data(self,func):
        '''modify function to save its returned data'''
        def new_func(*args, **kwargs):
            self.data=func(*args, **kwargs)

        return new_func

    def start(self,params):
        self.data = None
        if self.thread is not None:
            if self.thread.isAlive():
                return 'running' #could raise exception here

        #unless thread exists and is alive start or restart it
        self.thread = threading.Thread(target=self.func,args=params)
        self.thread.start()
        return 'started'

    def status(self):
        if self.thread is None:
            return 0#'not_started'
        else:
            if self.thread.isAlive():
                return 1#'running'
            else:
                return 2#'finished'

    def get_results(self):
        if self.thread is None:
            return 'not_started' #could return exception
        else:
            if self.thread.isAlive():
                return 'running'
            else:
                return self.data




def runSim(cmd, args):
        col = subprocess.Popen([cmd + args], shell=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE).communicate()
        return (args, col[1],col[0])

rRange = [0, 1,  2, 3]

rMod = [1,2,3,"0.1",".01"]

littleTickRange = ["0.00001", "0.0001", "0.001", "0.01", "0.1"]

simEnd = "1000"

clockRates = [1, 10, 100, 1000]


cmd = "../../builds/models/tnt_benchmark/tnt_benchmark --neurons=256 "

processes = []
results = []
vals = []
running = 0
current = 0
total = 0
for rmode in rRange:
    for littleTick in littleTickRange:
        for clock in clockRates:
            for modd in rMod:
                args = " --clock-rate=%s --end=%s --lt=%s --rm=%s --extramem=20000000 --rv=%s" % (clock, simEnd, littleTick, rmode,modd)
                if threading.active_count() < 40:
                    current = current + 1
                    total = total + 1
                    th = (ThreadWorker(runSim))
                    th.start((cmd,args),)
                    processes.append(th)
                    print("Running sim %s" % total)

                else:

                    while threading.active_count() >= 32:
                        pass
                    th = (ThreadWorker(runSim))
                    th.start((cmd,args),)
                    processes.append(th)
                    total = total + 1



ct = 0
for proc in processes:
    while(proc.status() ==1):
        pass
    if("0" in proc.get_results()[1]):
        print("Proc ran with no colls")
        print (proc.get_results()[0])
    if("increase" in proc.get_results()[2]):
        print ("Memory error -- ")
        print (proc.get_results()[0])

print (total)
