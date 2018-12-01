import multiprocessing as mp
import multiprocessing.pool as mpool
import os
from collections import defaultdict

import tqdm


class MultiFileWorker():

    def __init__(self,file_list,split_size = 50*1024*1024,nworkers=mp.cpu_count()//2):
        print('Setting up file(s):')
        print(file_list)
        self.start_pos = 3
        if isinstance(file_list,tuple):
            fl = []
            for f in file_list:
                fl.append(f)
            self.file_list = fl

        elif isinstance(file_list, list):
            self.file_list = file_list
        elif isinstance(file_list,str):
            self.file_list = [file_list]
        else:
            print("File list must be a list of files, or a single file name.")

        self.split_size = split_size
        self.nworkers = nworkers

        self.mp_results = []
        self.agg_results = 0



    def line_parser(self,lines):
        counts = defaultdict(int)
        connections = {}
        for l in lines:
            if not 'timestamp' in l:
                sr = l.split(',')
                if len(sr) == 4:
                    #alternate NeMo Spike Record Format
                    src = f"{sr[1]},{sr[2]}"
                    gid = int(sr[3])
                    # core is the global ID divided by the nubmer of LPs per core - which is 256 (axons) + 1 (synapse) + 256 (neurons).
                    d_core = gid // (256 + 1 + 256)
                    #neuron id is gid mod the same value.
                    d_neuron = gid % (256 + 1 + 256)
                    dst = f"{d_core},{d_neuron}"
                else:
                    src = f"{sr[1]},{sr[2]}"
                    dst = f"{sr[4]},{sr[5]}"
                connections[src] = dst
                counts[src] += 1

        return (counts,connections)


    def result_aggregate(self,worker_results):
        counts = defaultdict(int)
        connections = {}

        def updateCounts(count_d, newcount_d):
            for key in newcount_d.keys():
                count_d[key] += newcount_d[key]
            return count_d

        for proc in tqdm.tqdm(worker_results,desc="aggregation of tables..."):
            pfile_result = proc.get()
            counts = updateCounts(counts,pfile_result[0])
            connections.update(pfile_result[1])

        result = (counts,connections)
        self.result = result
        return result





    def process_file_worker(self,filename,start=0, stop=0,default_pos=5):
        """
        Takes a filename, a function, and the start and stop bytes. Reads in lines between start and stop,
        passing the lines to a worker function. The worker function must take multiple lines.
        :param filename:
        :param multi_line_reader_func:
        :param start:
        :param stop:
        :return:
        """
        if start == 0 and stop == 0:
            with open(filename,'r') as f:
                results = self.line_parser(f.readlines())
                #counts,connections = readCSVChunk(f.readlines())

        else:
            with open(filename, 'r') as fh:
                fh.seek(start)
                lines = fh.readlines(stop - start)
                results = self.line_parser(lines)
        #results = (counts,connections)
        return results


    def parse_files(self):
        workers = []
        results = []
        print("using " + str(self.nworkers) + " to read data.")
        with mpool.Pool(self.nworkers) as pool:
            max_pos = self.nworkers + 1
            for filename in self.file_list:
                print("filename: " + filename)
                filesize = os.path.getsize(filename)
                # if filesize > self.split_size:
                print("Using MP to parse files.")

                cursor = 0
                with open(filename,'r') as fh:
                    for chunk in range(filesize // self.split_size):
                        if cursor + self.split_size > filesize:
                            end = filesize
                        else:
                            end = cursor + self.split_size

                        fh.seek(end)
                        fh.readline()
                        end = fh.tell()
                        proc = pool.apply_async(self.process_file_worker, args=[filename,cursor,end,self.start_pos,])
                        self.start_pos += 1
                        self.start_pos = self.start_pos % max_pos

                        workers.append(proc)
                        cursor = end
            # else:
                #     print("File too small - no MP")
                #     results.append(self.process_file_worker(filename))
                #

            results = self.result_aggregate(workers)
        return results


