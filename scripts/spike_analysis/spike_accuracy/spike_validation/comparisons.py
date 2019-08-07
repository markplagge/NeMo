import click
import dask
import dask.dataframe as df
import numpy as np
import pandas as pd
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor as procE
from concurrent.futures import ThreadPoolExecutor  as threadE
import concurrent.futures
import weakref
import click_spinner

from .data_interface.spike_sql_iface import SpikeDataInterface




class MissingSpikes(object):
    """
    Missing spikes - object that manages spike comparisons with NeMo and NSCS spikes, or NEMO and NEMO spikes.
    For dataframes, the LEFT part of the queries are defiend as NSCS_xx, and the RIGHT sides are NEMO_xx.
    Add one of the query classes to generate queries!
    """
    query_objects = []
    def __init__(self,nscs_df, nemo_df,parallel_q=False,parallel_method='T'):
        self.nscs_df = nscs_df
        self.nemo_df = nemo_df
        self.parallel_q = parallel_q
        self.parallel_method = parallel_method


    def add_query_object(self, qobject):
        assert(isinstance(qobject, SpikeQuery))
        qobject.parent = self
        #qobject.nscs_df = weakref.ref(self.nscs_df)
        #qobject.nemo_df = weakref.ref(self.nemo_df)
        click.secho("Adding query object " + qobject.name + " and assembling queries", fg="yellow")

        qobject.build_query()
        self.query_objects.append(qobject)



    def __execute_queries_parallel(self):
        if self.parallel_method == 'T':
            ex = threadE
        else:
            ex = procE
        running_queries = []
        finished_queries = []
        with ex() as executor:
            for qobj in self.query_objects:
                running_queries.append(executor.submit(qobj.run_query))
            for f in tqdm(concurrent.futures.as_completed(running_queries),total=len(running_queries),desc="Running Parallel Queries"):
                pass

    def get_query_results(self,name=None):
        if name:
            for qo in self.query_objects:
                if qo.name == name:
                    return qo.result
        else:
            return [qo.result for qo in self.query_objects]

    def execute_queries(self):
        if self.parallel_q:
            self.__execute_queries_parallel()
        else:
            #for qo in self.query_objects:
            for qo in tqdm(self.query_objects, desc="running queries"):
                qo.run_query()

class MissingSpikeSQL(MissingSpikes):
    query_objects = []
    def __init__(self,data_iface,parallel_q=False):
        self.data_iface = data_iface
        self.parallel_q = parallel_q
        assert(isinstance(self.data_iface,SpikeDataInterface))

    def add_query_object(self,qobject):
        assert(isinstance(qobject,SpikeQuery))
        qobject.parent = self
        self.query_objects.append(qobject)


class SpikeQuery(object):
    """Default SpikeQuery object.
    Runs no query by default."""
    def __init__(self, fields=None, name="Base"):
        if fields is None:
            fields = ["A", "B"]
        assert(isinstance(fields,list))
        self.fields = fields
        self.name = name
        self.result = None
        self.parent = None

    def run_query(self):
        self.result =[]

    def get_result(self):

        return self.result

    def build_query(self):
        pass

class SpikeQuery_SCN_DCN_Time(SpikeQuery):

    def __init__(self,name="time_delta"):
        fdl = ['srcCore', 'srcNeuron', 'destCore', 'destAxon']
        super().__init__(fdl,name)

class SpikeQuery_FULL_SRC_COMP(SpikeQuery):

    def run_query(self):
        """By default executes the dask query, since build_query sets up the Q into
        pending dask dataframes"""
        print("running full results")
        self.results = {}
        self.results["full"] = self.full.compute()
        self.results["nemo"] = self.ne_only.compute()
        self.results["nscs"] = self.ns_only.compute()
        return self.results

        # ns_gp, ne_gp = self.parent.data_iface.get_spike_counts_all()
        # r = ns_gp.merge(ne_gp, how='outer', indicator=True)
        # # df = df.mask(df == 'PASS', '0').mask(df == 'FAIL', '1')
        # r = r.mask(r == 'left_only', 'nscs_only').mask(r == 'right_only', 'nemo_only')
        # self.result = r


        return r

    def build_query(self):
        ns_gp, ne_gp = self.parent.data_iface.get_spike_counts_all()
        r = ns_gp.merge(ne_gp, how='outer', indicator=True)
        # df = df.mask(df == 'PASS', '0').mask(df == 'FAIL', '1')
        def repl(col):
            if "left_only" in col:
                return "nscs_only"
            if "right_only" in col:
                return "nemo_only"

        #r = r.mask(r == 'left_only', 'nscs_only').mask(r == 'right_only', 'nemo_only')
        r = r['_merge'] = r['_merge'].map(repl)
        #in_nscs = outer_join[outer_join['_merge'] == 'left_only']
        self.full = r
        self.ns_only = r[r['_merge'] == 'nscs_only']
        self.ne_only = r[r['_merge'] == 'nemo_only']
        print("Built Query")



class SpikeQuery_SCN_DCN_SQL_GRP(SpikeQuery):

    def run_query(self):
        super().run_query()
        #self.result = {}
        query = """
 select spike_nemo_grps."srcCore" as nemo_src_core,
        spike_nemo_grps."srcNeuron" as nemo_src_neuron,
        spike_nemo_grps."destCore" as nemo_dest_core,
        spike_nemo_grps."destAxon" as nemo_dest_axon,
        spike_nemo_grps."num_spikes" as nemo_num_spikes,
        spike_nscs_grps."srcCore" as tn_src_core,
        spike_nscs_grps."srcNeuron" as tn_src_neuron,
        spike_nscs_grps."destCore" as tn_dest_core,
        spike_nscs_grps."destAxon" as tn_dest_axon,
        spike_nscs_grps."num_spikes" as tn_num_spikes
 from spike_nemo_grps
full outer join  spike_nscs_grps
on spike_nemo_grps."srcNeuron" = spike_nscs_grps."srcNeuron" and
   spike_nemo_grps."srcCore" = spike_nscs_grps."srcCore" and
   spike_nemo_grps."destAxon" = spike_nscs_grps."destAxon" and
   spike_nemo_grps."destCore" = spike_nscs_grps."destCore"
"""
        self.result = self.parent.data_iface.exec_arb_q(query)
        #working_df = df.from_pandas(self.result,chunksize=1024)

        return self.result


class SpikeQuery_SCN_DCN(SpikeQuery):
    """Query for Soruce neurons, cores & dest neurons, cores
    Runs a full outer join - and generates spikes missing in NeMo,
    and spikes missing in NSCS, as well as the full outer join data.
    Stores these as dask dataframes until computation is needed."""


    def __init__(self,name="full_missing"):
        fdl = ['srcCore','srcNeuron','destCore','destAxon']
        super().__init__(fdl,name)

        self.in_both = None
        self.in_nscs = None
        self.in_nemo = None
        self.merge_filename='in_both.csv'
        self.do_chunks = False
        self.run_full_q = False


    def run_query(self):
        super().run_query()

        self.result = {}
        click.secho("Executing outer join query.")
        click.secho("TODO: override sql iface to return dask dataframes.")


        if isinstance(self.parent, MissingSpikeSQL):

            max_qup = 512
            c_core = 0
            nf = 0
            pending = []
            results = []
            ## SQL GRP Q
            #end_core = self.parent.data_iface.get_max_src_core()
            end_core = 4096





            if self.run_full_q:
                ## Don't always do dask here...
                if self.parent.parallel_q:
                    return -1
                else:
                    self.parent.data_iface.return_dask = True
                    print("running large query group result")
                    ns_gp, ne_gp = self.parent.data_iface.get_spike_counts_all()
                    print("Sql got")

                    #assert(isinstance(ns_gp,pd.DataFrame))
                    #assert(isinstance(ne_gp,dask.dataframe.DataFrame))
                    r = ns_gp.merge(ne_gp,how='outer',indicator=True)
                    self.result = r
                    return r

            elif self.parent.parallel_q:
                return -1
            else:
                ### Always do DASK here...
                self.parent.data_iface.return_dask = True
                #with open(self.merge_filename, 'w') as out_file:
                for i in tqdm(range(0,end_core + 1),position=2,desc="running sql group queries"):



                    ns_gp,ne_gp = self.parent.data_iface.get_spike_counts_by_core(i)

                    #do a merge:
                    pending.append(df.merge(ns_gp,ne_gp,how="outer",on=self.fields,indicator=True))

                    c_core += 1
                    if c_core > max_qup and self.do_chunks:
                        vx_result = df.concat(df.concat(pending).compute())
                        #results.append(vx_result)
                        with open(self.merge_filename,'a') as out_file:
                            vx_result = pd.DataFrame(vx_result)
                            if nf == 0:
                                hdr = True
                                nf = 1
                            else:
                                hdr = False
                            vx_result.to_csv(out_file,header=hdr)
                        c_core = 0
                        pending = []



                #compute results:
                if self.do_chunks:
                    result =pd.DataFrame( df.concat(pending).compute())
                    result.to_csv(self.merge_filename)
                    ## and maybe not this if we are memory bound...
                    return result

        nscs_data_full = self.parent.nscs_df
        nemo_data_full = self.parent.nemo_data
        ## try a core-wise query setup first:
        max_core = max(nscs_data_full['destCore'].max().compute(),nemo_data_full.max().compute())
        group_results = []
        with click_spinner.spinner():
            for core in tqdm(range(0,max_core+1),desc="Core filter technique", position=1):
                nemo_cores = nemo_data_full[ nemo_data_full['srcCore'] == core]
                nscs_cores = nscs_data_full[nscs_data_full['srcCore'] == core]
                ## and generate the merge - save to a csv file to unload:
                #nemo_both = df.merge(nscs_cores, nemo_cores,how="outer", on=self.fields,indicato=True)
                ## also a groupby might work too:
                nscs_groups = nscs_cores.groupby(['srcCore','srcNeuron','destCoe','destNeuron']).count()
                ## right, do this.
                group_results.append(nscs_groups.compute())
        group_df = pd.concat(group_results)
        group_df.to_csv(self.merge_filename,delimiter=',')
        return group_df
        #    result = self.in_both.compute()
        #    self.result['in_both'] = result
        """
        click.secho("Running in_nemo filter", fg="blue")
        with click_spinner.spinner():
            result = self.in_nemo.compute()
            self.result['in_nemo'] = result
        click.secho("Running in_nscs filter", fg="blue")
        with click_spinner.spinner():
            result = self.in_nscs.compute()
            self.result['in_nscs'] = result
"""
        click.secho("Completed running.")


    def build_query(self):
        """
        builds the query, and stores it as dask values
        :return:
        """
        if isinstance(self.parent,MissingSpikeSQL):
            nscs_df = self.parent.nscs_df#self.parent.nscs_df
            nemo_df = self.parent.nemo_df#self.parent.nemo_df

            outer_join = df.merge(nscs_df,nemo_df,how="outer",on=self.fields,indicator=True)#.sort_values("timestamp_x")


            in_nscs = outer_join[outer_join ['_merge'] == 'left_only']
            in_nemo = outer_join[outer_join ['_merge'] == 'right_only']
            in_both = outer_join[outer_join ['_merge'] == 'both']
            #df = df.mask(df == 'PASS', '0').mask(df == 'FAIL', '1')


            self.in_nemo = in_nemo
            self.in_nscs = in_nscs
            self.in_both = in_both




if __name__ == '__main__':
    ## test objects:
    test_data_1 = """timestamp,srcCore,srcNeuron,destCore,destAxon
    3.031378,1,0,3,5
    3.031378,1,1,3,6
    3.031378,1,2,3,7
    3.031378,2,0,4,9
    3.031378,2,1,4,10
    3.031378,2,2,4,11
    3.031378,2,3,4,12
    6,0,0,99,99"""

    test_data_2 = """timestamp,srcCore,srcNeuron,destCore,destAxon
    3.202086,1,0,3,5
    3.944109,1,1,3,6
    3.737143,1,2,3,7
    3.586790,2,0,4,9
    3.361805,2,1,4,10
    3.338743,2,2,4,11
    3.955040,2,3,4,12
    6.04315288019573,32,32,99,99
    1.43732959253849,0,0,88,88"""
    from io import StringIO

    test_data_1 = StringIO(test_data_1)
    test_data_2 = StringIO(test_data_2)
    d1 = pd.read_csv(test_data_1, header=0, sep=',')
    d2 = pd.read_csv(test_data_2, header=0, sep=',')
    d1 = df.from_pandas(d1,npartitions=2)
    d2 = df.from_pandas(d2,npartitions=2)

    spike_results = MissingSpikes(d1,d2)
    spike_results.add_query_object(SpikeQuery_SCN_DCN("full"))

    spike_results.execute_queries()
    results = spike_results.get_query_results('full')
    for k,v in results.items():
        click.secho(f"{k}:", fg="blue")
        print(v)

    print("done")


