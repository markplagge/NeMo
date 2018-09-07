import json
import multiprocessing as mp
from multiprocessing import Pool
import tqdm
#from click._unicodefun import click
import click
import networkx as nx


#import traffic_analysis.analysis_utils
from analysis_utils import MultiFileWorker,MultiFileDumpi

## SEE ELEMENTS BELOW -- CLICK COMMANDS

class FileLoader(MultiFileWorker):
    num_spikes_core_got = 0
    num_spikes_core_sent = 0
    core_of_interest = 0
    def __init__(self,network_data,datas,core_of_interest=0,*args,**kwargs):
        self.core_of_interest = core_of_interest
        self.csv_fn = './core_' + str(core_of_interest) + '_traffic.csv'
        self.json_fn = self.csv_fn.replace('csv','json')
        self.edge_fn = self.csv_fn.replace('csv','json').replace('core','edge_list')
        #m = mp.Manager()
        self.data_q =datas # m.Queue()
        self.network_q = network_data
        #self.m = m

        #self.data_q = mp.Queue()

        super().__init__(args,kwargs)

        self.split_size = 50 * 1024 * 1024
        self.start_pos = 1
        self.nworkers = 20



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
        #click.secho("Subclass process file worker starting...",fg='green',underline=True)
        if start == 0 and stop == 0:
            with open(filename,'r') as f:
                results = self.line_parser(f.readlines())
                #counts,connections = readCSVChunk(f.readlines())

        else:
            with open(filename, 'r') as fh:
                fh.seek(start)
                lines = fh.readlines(stop - start)
                results = self.line_parser(lines,bar_pos=default_pos)
        #results = (counts,connections)
        return results

    def line_parser(self,lines,bar_pos=5):
        #click.secho('starting line parsing...', fg='green')
        dr = []
        #for line in lines:

        for line in tqdm.tqdm(lines,position=bar_pos + 1,desc='subclass parsing file data...',dynamic_ncols=True,leave=False):
            if 'spike' in line:
                l = json.loads(line)
                if l['spike']['srcCore'] == self.core_of_interest:
                    #self.num_spikes_core_sent += 1
                    dr.append( l['spike'] )
                    self.data_q.put(l['spike'])
                elif l['spike']['destCore'] == self.core_of_interest:
                    #self.num_spikes_core_got += 1
                    dr.append(l['spike'])
                    self.data_q.put(l['spike'])


    def result_aggregate(self,worker_results):
        results = []
        for proc in tqdm.tqdm(worker_results,desc="aggregation of tables..."):
            pfile_result = proc.get()
            #results = results + pfile_result
        self.result = results
        return 0


    def graph_worker(self):
        G = nx.MultiDiGraph()
        running = True
        while running:
            val = self.network_q.get()
            if 'done' in val:
                running = False
            else:

                G.add_edge()


    def save_worker_process(self):
        click.secho('Saving spikes to ' + self.csv_fn + ' and ' + self.json_fn, fg='green')
        num_gos = 0
        def w_csv(fh,dct):
            ot = f"{dct['srcCore']},{dct['srcNeuron']}" \
                 f",{dct['destCore']},{dct['destAxon']}\n"
            fh.write(ot)

        def w_json(jout,dct):
            v = jout.write(json.dumps(dct))
            v = jout.write('\n')


        running = True
        with open(self.csv_fn,'w') as csvf:
            csvf.write('srcCore,srcNeuron,destCore,destNeuron\n')
            with open(self.json_fn,'w') as jsnf:
                with tqdm.tqdm(position=2) as pbar:
                    while running:
                        #try:
                            val = self.data_q.get()
                            num_gos += 1
                            if 'done' in val:
                                running = False

                            elif isinstance(val,dict):
                                pbar.update(1)
                                l = val
                                #click.secho(str(l),fg='red',bold=True)

                                if l['srcCore'] == self.core_of_interest:
                                    self.num_spikes_core_sent += 1
                                else:
                                    self.num_spikes_core_got += 1

                                otcsv = f"{l['srcCore']},{l['srcNeuron']}" \
                                     f",{l['destCore']},{l['destAxon']}\n"
                                csvf.write(otcsv)
                                jsnf.write(json.dumps(l) + '\n')



                                #w_csv(csv,l)
                                #w_json(jsn,l)


                            else:
                                click.secho("not done but no dict got.",fg='red',underline=True,bold=True)
                                click.secho("exiting...",fg='red',bold=True)
                                exit(1)

        #write metadata file:
        stn = "Core " + str(self.core_of_interest) + " "
        click.secho("Ran " + str(num_gos) + ' times...',fg='blue')
        with open('core_0_metadata.txt', 'w') as meta:
            meta.write(stn + " received " + str(self.num_spikes_core_got) + ' spikes. \n')
            meta.write(stn + ' send ' + str(self.num_spikes_core_sent) + ' spikes. \n')




    def parse_files(self,test=False):
        if test:
            self.nworkers = 1
        px = mp.Process(target=self.save_worker_process)
        px.start()
        click.pause('Started file writer process. Press any key to continue..."')
        super().parse_files()
        self.data_q.put('done')
        click.secho('waiting to complete...')
        px.join()


        # workers = []
        # results = []
        # with mpool.Pool(self.nworkers) as pool:
        #     for filename in self.file_list:
        #         filesize = os.path.getsize(filename)
        #         # if filesize > self.split_size:
        #         print("Using MP to parse files.")
        #
        #         cursor = 0
        #         with open(filename,'r') as fh:
        #             for chunk in range(filesize // self.split_size):
        #                 if cursor + self.split_size > filesize:
        #                     end = filesize
        #                 else:
        #                     end = cursor + self.split_size
        #
        #                 fh.seek(end)
        #                 fh.readline()
        #                 end = fh.tell()
        #                 proc = pool.apply_async(self.process_file_worker, args=[filename,cursor,end])
        #                 workers.append(proc)
        #                 cursor = end
        #     # else:
        #         #     print("File too small - no MP")
        #         #     results.append(self.process_file_worker(filename))
        #         #
        #
        #     results = self.result_aggregate(workers)
        # return results


#
#
# def save_0_csv(out_file_name='core_' + str(core_of_interest)+".csv"):
#     with open(save_0_csv,'w') as out:
#         out.write('srcCore,srcNeuron,destCore,destNeuron\n')
#
# def save_0_json(out_file_name='core_' + str(core_of_interest)+  ".json"):
#     pass
#
#
# def get_from_0(file):
#     global num_spikes_core_got
#     global num_spikes_core_sent
#     with open(file,'r') as f:
#         for line in f:
#                 if 'spike' in line:
#                         l = json.loads(line)
#                         if l['spike']['srcCore'] == core_of_interest:
#                             num_spikes_core_sent += 1
#                             yield l['spike']
#                         elif l['spike']['destCore'] == :
#                             num_spikes_core_got += 1
#                             yield l['spike']
#
# def save_to_0(open_file, dct):
#     #out = f"{dct['srcCore']},dct['srcNeuron'],dct['destCore'],dct["destAxon"]\n"
#     out = str(dct['srcCore']) + str(dct['srcNeuron']) + str(dct['destCore']) + str(dct['destAxon']) + '\n'
#     v = open_file.write(out)
#
num_spikes_core_got_e = 0
num_spikes_core_sent_e = 0
core_of_interest_e = 0
fn = 'mnist_tee0.json'
mode = 'nscs'
mf_data = mp.Queue()


@click.command()
@click.option('--spike_file', default='',help='spike data file')
@click.option('-N', default=False, is_flag=True, help='process NSCS spike file (default is NEMO spike json')
@click.option('--core',default=0, help='What core are we counting?')
def cli(spike_file,n,core):
    if not n:
        click.secho("parsing "  + spike_file, fg='green')
        tasks = mp.Manager().Queue()
        t2 = mp.Manager().Queue()
        worker = FileLoader(t2,tasks,core,spike_file)


        click.secho("working!", fg='yellow')

        worker.parse_files()
        click.secho("CORE #" + str(core), fg='blue',bold=True,underline=True)
        click.secho("Number of spikes the core got: " + str(worker.num_spikes_core_got))
        click.secho("Number of spikes the core sent: " )
    else:
        click.secho("NeMo Core Spike Count not implemented!", fg='red')




if __name__ == '__main__':
    cli()



# zero_core = []
# with open('zero_cores.csv','w') as out:
#     with open('zero_cores.json','w') as jout:
#         for zc in get_from_0(fn):
#             save_to_0(out,zc)
#             v = jout.write(json.dumps(zc))
#             v = jout.write('\n')
#
# print("number of sends to core 0")
# print(num_spikes_core_got)
# print("number of sends from core 0")
# print(num_spikes_core_sent)
# with open('core_0_metadata.txt','w') as meta:
#         meta.write('Sends to core 0: ' + str(num_spikes_core_got) + '\n')
#         meta.write('Core 0 recvs: ' + str(num_spikes_core_sent) + '\n')