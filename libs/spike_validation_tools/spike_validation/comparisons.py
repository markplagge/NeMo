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


class SpikeQuery(object):
    """Default SpikeQuery object.
    Runs no query by default."""
    def __init__(self,fields,name):
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


    def run_query(self):
        super().run_query()

        self.result = {}
        click.secho("Running both_query...",fg="blue")
        with click_spinner.spinner():
            result = self.in_both.compute()
            self.result['in_both'] = result
        click.secho("Running in_nemo filter", fg="blue")
        with click_spinner.spinner():
            result = self.in_nemo.compute()
            self.result['in_nemo'] = result
        click.secho("Running in_nscs filter", fg="blue")
        with click_spinner.spinner():
            result = self.in_nscs.compute()
            self.result['in_nscs'] = result

        click.secho("Completed running.")


    def build_query(self):
        """
        builds the query, and stores it as dask values
        :return:
        """
        nscs_df = self.parent.nscs_df#self.parent.nscs_df
        nemo_df = self.parent.nemo_df#self.parent.nemo_df


        outer_join = df.merge(nscs_df,nemo_df,how="outer",on=self.fields,indicator=True)#.sort_values("timestamp_x")
        in_nscs = outer_join[outer_join ['_merge'] == 'left_only']
        in_nemo = outer_join[outer_join ['_merge'] == 'right_only']
        in_both = outer_join[outer_join ['_merge'] == 'both']

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


