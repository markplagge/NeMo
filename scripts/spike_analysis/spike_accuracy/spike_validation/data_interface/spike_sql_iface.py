import concurrent
import multiprocessing
import os
import tempfile
import threading
from concurrent.futures import as_completed

import sqlalchemy
from sqlalchemy import create_engine
from sqlalchemy.sql import text
from sqlalchemy import exists
import dask.dataframe as df
import dask.array as da
from tqdm import tqdm
from functools import partial

# from analysis_utils import MultiFileWorker
from spike_accuracy.spike_validation.data_interface.spike_file_reader import read_nscs_spikes_chunks, json_callback, \
    popen_method
from concurrent.futures.process import SimpleQueue

df_runner_q = SimpleQueue()
try:
    from psycopg2cffi import compat

    compat.register()
except:
    pass

import dask
import numpy as np

import pandas as pd
# from enum import Flag, auto
from flags import Flags


class STAT(Flags):
    NEMO_TABLE_OK = ()
    NSCS_TABLE_OK = ()
    NEMO_VIEW_OK = ()
    NSCS_VIEW_OK = ()

    NEMO_TABLE_ERR = ()
    NSCS_TABLE_ERR = ()
    NEMO_VIEW_ERR = ()
    NSCS_VIEW_ERR = ()

    FILE_LOADED = ()
    NULL_INPUT_DATA = ()
    QUERY_ERROR = ()
    DATABASE_ERROR = ()
    DATABASE_OKAY = ()
    NOT_INIT = ()
    START_INIT = ()


js_values = multiprocessing.SimpleQueue()


# class TableMFCreate(MultiFileWorker):
#     def __init__(self, file_list,engine,table, split_size=50 * 1024 * 1024, nworkers=10):
#         self.engine = engine
#         self.table = table
#         super.__init__(file_list,split_size,nworkers)
#
#     def line_parser(self,lines):
#         """basically just doing the stuff in the other file but now input is handled in chunks"""
#         js_list = []
#         for l in lines:
#             if '"Spike"' in l:
#                 js_list.append(json_callback(l))
#         js_values.put(js_list)
#
#     def parse_files(self):
#         pass


def nemo_table_runner(p_dataframe, dsn, table):
    eng = create_engine(dsn)
    with eng.connect() as conn:
        # for p_dataframe in data_list:
        # p_dataframe = pd.DataFrame(p_dataframe)
        p_dataframe.to_sql(table, conn, if_exists='append')


class SpikeDataInterface():
    table_base_name = 'spike_'
    return_pandas = False
    return_dask = False
    pgfutter_pth = "/home/plaggm/util/pgfutter"

    def __set_status(self, new):
        self.status = self.status | new

    def __init__(self, connection_dsn='sqlite:///:./spike_data.sqlite', create_new=True, nscs_data=None, nemo_data=None,
                 ns_file=True, ne_file=False, check_only=False):
        """

        :param connection_dsn:
        :param create_new:
        :param nscs_data:
        :param nemo_data:
        :param ns_file:
        :param ne_file:
        :param use_grobber: do we use the posgresql grobbing system to load the data?
        """
        self.dbname = connection_dsn.split("//")[1].split("@")[0]
        self.schema_name = 'public'

        self.ns_table = self.table_base_name + "nscs"
        self.ne_table = self.table_base_name + "nemo"
        self.ns_table_g = self.ns_table + "_grps"
        self.ne_table_g = self.ne_table + "_grps"
        self.eng = create_engine(connection_dsn)
        self.dsn = connection_dsn
        self.status = STAT.NOT_INIT
        self.table_result = 0
        self.view_result = 0
        self.create_new = create_new
        self.ns_file = ns_file
        self.ne_file = ne_file

        ## I want this class to handle both pandas and dask dataframes if we are creating a new table:
        self.status = STAT.START_INIT
        result = self.get_table_data()
        if check_only:
            return

        if ne_file == True:
            print("Not Implemented")
        if isinstance(nemo_data, df.DataFrame):
            self.nemo_data = nemo_data.compute()
            del (nemo_data)
        else:
            self.nemo_data = nemo_data

        if ns_file == True and (create_new or self.status.NSCS_TABLE_ERR):
            assert (isinstance(nscs_data, str))
            self.nscs_data = nscs_data
        elif isinstance(nscs_data, df.DataFrame):
            self.nscs_data = nscs_data.compute()
            del (nscs_data)
        else:
            self.nscs_data = nscs_data

        self.__create_table()

        if self.status.NSCS_VIEW_ERR:
            print("Creating NSCS spike query view")
            self.create_view(self.ns_table)
        if self.status.NEMO_VIEW_ERR:
            print("Creating NeMo spike query view")
            self.create_view(self.ne_table)

    def get_table_data(self):
        m = "loaded"
        if self.__do_tables_exist():
            tbl = "tables ok"
        else:
            tbl = "tbls not okay"
        if self.__do_views_exist():
            view = "views ok"
        else:
            view = "views not okay"

        return [tbl, view]

    def __do_views_exist(self):
        views = sqlalchemy.inspect(self.eng).get_view_names()
        nview_exist = False
        nsview_exist = False
        for v in views:
            if self.ns_table_g in v:
                nsview_exist = True
            if self.ne_table_g in v:
                nview_exist = True

        # nview_exist = self.eng.dialect.has_view(self.eng,self.ne_table_g)

        # nsview_exist = self.eng.dialect.has_view(self.eng,self.ns_table_g)
        if nsview_exist:
            self.__set_status(STAT.NSCS_VIEW_OK)
        else:
            self.__set_status(STAT.NSCS_VIEW_ERR)

        if nview_exist:
            self.__set_status(STAT.NEMO_VIEW_OK)
        else:
            self.__set_status(STAT.NEMO_VIEW_ERR)

        return nview_exist and nsview_exist

    def __do_tables_exist(self):
        nemo_exist = self.eng.dialect.has_table(self.eng, self.ne_table)
        if nemo_exist:
            self.__set_status(STAT.NEMO_TABLE_OK)
        else:
            self.__set_status(STAT.NEMO_TABLE_ERR)

        nscs_exist = self.eng.dialect.has_table(self.eng, self.ns_table)
        if nscs_exist:
            self.__set_status(STAT.NSCS_TABLE_OK)
        else:
            self.__set_status(STAT.NSCS_TABLE_ERR)

        return nemo_exist and nscs_exist

        # with self.eng.connect() as con:

    def __gen_core_query(self, core_id, tbl):
        qry = f'SELECT "srcneuon", "destcore","destaxon","timestamp" FROM {tbl} WHERE srccore = {core_id}'
        return qry

        # qry = f"SELECT  {self.ns_table}.srcNeuron,  {self.ns_table}.destCore,{self.ns_table}.destAxon,{self.ns_table}.timestamp\
        #        {self.ne_table}.srcNeuron,  {self.ne_table}.destCore, {self.ne_table}.destAxon, {self.ne_table}.timestamp \n WHERE srcCore =  {core_id} \n"

    def __gen_group_query_core(self, tbl, core):
        qry = self.__gen_group_query(tbl)
        # qry = qry.replace("GROUP BY",'WHERE "srcCore" = ' + str(core) + ' \n GROUP BY')
        qry = qry + 'WHERE t."srcCore" = ' + str(core) + ";"
        return qry

    def __gen_group_query(self, tbl):
        ## Added this to use the nice view setup:
        tbl = tbl + "_grps"

        return f'SELECT t."srcCore",t."srcNeuron",t."destCore",t."destAxon",t."num_spikes" as t \n ' \
            f' FROM {tbl} t \n '

    #            f'\n  GROUP BY t."srcCore",t."srcNeuron",t."destCore",t."destAxon" '

    def test_gq(self, tbl, core):
        print(self.__gen_group_query_core(tbl, core))
        print(self.__gen_group_query(tbl))

    def run_dual_queries(self, q1, q2):
        with self.eng.connect() as con:
            with self.eng.connect() as con2:
                if self.return_dask:
                    nscs_data = df.from_pandas(pd.read_sql(q1, con), chunksize=128)
                    nemo_data = df.from_pandas(pd.read_sql(q2, con2), chunksize=128)
                elif self.return_pandas:
                    nscs_data = pd.read_sql(q1, con)
                    nemo_data = pd.read_sql(q2, con2)
                else:

                    nscs_data = con.execute(text(q1))
                    nemo_data = con.execute(text(q2))

                return (nscs_data, nemo_data)

    def get_core(self, core_id):

        ns_qry = self.__gen_core_query(core_id, self.ns_table)
        ne_qry = self.__gen_core_query(core_id, self.ne_table)
        return self.run_dual_queries(ns_qry, ne_qry)

    def mview_query(self, stbl):

        qry = f'create MATERIALIZED VIEW {stbl}_grps AS \n ' \
            f'select "srcCore", "srcNeuron","destCore","destAxon", COUNT("timestamp") ' \
            f'as num_spikes from {stbl} \n' \
            f'group by "srcCore", "srcNeuron","destCore","destAxon" \n' \
            f'order by "srcCore" ASC;'

        return qry

    def df_runner(self):
        eng = create_engine(self.dsn)
        con = eng.connect()
        print("SQL Runner started.")
        tbl, all_dicts = df_runner_q.get()
        while "end" not in tbl:
            keys = {k for d in all_dicts for k in d}
            merged_dict = {k: [d.get(k, np.nan) for d in all_dicts] for k in keys}

            ld = pd.DataFrame.from_dict(merged_dict)
            if len(ld) == 0:
                print("ERROR LD WAS 0")
                print(ld)
                exit(-1)
            ld.to_sql(tbl, con, if_exists="append")
            tbl, all_dicts = df_runner_q.get()

    # def df_dask_runner(self,dsk_frame):
    #     con = self.eng.connect()

    def query_runner(self):
        eng = create_engine(self.dsn)
        con = eng.connect()
        print("SQL Runner started.")

        query = df_runner_q.get()
        while 'end' not in query:
            con.execute(text(query))

    def create_nscs_tbl_grob(self, filename):

        cc = 0
        max_chunk = 4096
        all_dicts = []
        # vals = ['timestamp','srcCore','srcNeuron','destGID','destCore','destAxon','isOutput?']
        ## try threaded mode
        runners = []
        done = []
        num_thds = 2
        for i in range(0, num_thds):
            runner = multiprocessing.Process(target=self.query_runner())
            runner.start()
            runners.append(runner)

        filesize = os.path.getsize(filename)
        # with tqdm(total=filesize, position=0, desc="loading files.") as bar:
        # so this time we are going to use the pgfutter utility
        cmd = self.pgfutter_pth + " "

    def create_nscs_tbl_chunk(self, filename):
        cc = 0
        max_chunk = 4096
        all_dicts = []
        # vals = ['timestamp','srcCore','srcNeuron','destGID','destCore','destAxon','isOutput?']
        ## try threaded mode
        runners = []
        done = []
        num_thds = 30
        for i in range(0, num_thds):
            runner = multiprocessing.Process(target=self.df_runner)
            runner.start()
            runners.append(runner)
        filesize = os.path.getsize(filename)
        with tqdm(total=filesize, position=0, desc="loading files.") as bar:
            for line in read_nscs_spikes_chunks(filename):
                all_dicts.append(line)
                cc += 1
                if cc >= max_chunk:
                    df_runner_q.put((self.ns_table, all_dicts))
                    all_dicts = []
                    cc = 0
                    bar.update(max_chunk * 100)
            for i in range(0, num_thds * 2):
                df_runner_q.put(["end", "end"])
        running = True

        for thd in tqdm(runners, desc="waiting for workers..."):
            if thd.is_alive():
                thd.join()
        return
        ##non-threaded:
        for line in read_nscs_spikes_chunks(filename):
            all_dicts.append(line)
            cc += 1
            if cc >= max_chunk:
                keys = {k for d in all_dicts for k in d}
                merged_dict = {k: [d.get(k, np.nan) for d in all_dicts] for k in keys}

                ld = pd.DataFrame.from_dict(merged_dict)
                ld.to_sql(self.ns_table, self.eng.connect(), if_exists="append")
                all_dicts = []
                cc = 0

    def create_nemo_tbl_chunk(self, filename):
        pass

    def mview_query_grob(self):
        qry = f''

    def splitDataFrameIntoSmaller(self, df, chunkSize=10000):
        listOfDf = list()
        numberChunks = len(df) // chunkSize + 1
        for i in tqdm(range(numberChunks)):
            mdf = df[i * chunkSize:(i + 1) * chunkSize]
            listOfDf.append(mdf)
        return listOfDf

    def do_ncv(self):
        self.nemo_data.to_csv('nemo_concat.csv',sep=",")
    def create_nemo_table(self, con):
        #working_data = multiprocessing.SimpleQueue()
        #rt = threading.Thread(target=self.do_ncv)
        #rt.start()
        #with open('nemo_concat.csv', 'w') as tmp:
        #    print("saving NeMo CSV Aggregate File")
        #    assert (isinstance(self.nemo_data, pd.DataFrame))
        #    self.nemo_data.to_csv(tmp, sep=',',)
        # popen_method()

        workers = []

        chunks = self.splitDataFrameIntoSmaller(self.nemo_data, 50000)
        print("spawning processes")
        with concurrent.futures.ThreadPoolExecutor(max_workers=40) as pool:
            # results = pool.map(nemo_table_runner,chunks)
            results = [pool.submit(nemo_table_runner, dfrm, self.dsn, self.ne_table) for dfrm in chunks]
            #dln = len(self.nemo_data)
            #results = pool.map(nemo_table_runner,self.nemo_data, [self.dsn] * dln, [self.ne_table] * dln)
            s_size = sum([len(res) for res in chunks])
            print("Sum res: " + str(s_size) + " \n OP Size: " + str(len(self.nemo_data)))
            for _ in tqdm(as_completed(results), desc="running sql add",total=len(chunks)):
                pass

        #print("Waiting for NeMo CSV File to close...")
        #rt.join()
        # self.nemo_data.to_sql(self.ne_table, con, if_exists='replace', chunksize=4096)

    def create_nscs_table(self, con):
        self.nscs_data.to_sql(self.ns_table, con, if_exists='replace', chunksize=4096)

    def __create_table(self):
        """
        Creates tables and needed views. Behavior is determined by the class parameters:
        if create_new is set, then recreate all tables. If create_new is not set,
        this will check to see if the tables and views exist.

        If create_new is not set:
            If we are missing a table, then if this object has the data needed for that table
            it will populate it.

            if a view is missing, then it will be created.

        :return:
        """
        print("Table/Data init")
        with self.eng.connect() as con:
            if self.create_new or self.status.NEMO_TABLE_ERR or self.status.NEMO_TABLE_OK != True:
                print("saving NeMo table")
                self.create_nemo_table(con)
            if self.create_new or self.status.NSCS_TABLE_ERR or self.status.NSCS_TABLE_OK != True:
                print("saving NSCS Tables")
                if self.ns_file:
                    self.create_nscs_tbl_chunk(self.nscs_data)
                else:
                    self.create_nscs_table(con)

            self.ns_table_g = self.ns_table + "_grps"
            self.ne_table_g = self.ne_table + "_grps"

    def create_view(self, op_table):
        print("Create view in " + op_table)

        view_q = self.mview_query(op_table)
        with self.eng.connect() as con:
            con.execute(text(view_q))

    def exec_arb_q(self, query):
        with self.eng.connect() as con:
            return pd.read_sql(query, con)

    def get_spike_counts_by_core(self, core_id):
        ns_q = self.__gen_group_query_core(self.ns_table, core_id)
        ne_q = self.__gen_group_query_core(self.ne_table, core_id)
        # print(ns_q)
        # print(ne_q)

        return self.run_dual_queries(ns_q, ne_q)

    def get_spike_counts_all(self):
        ns_q = self.__gen_group_query(self.ns_table)
        ne_q = self.__gen_group_query(self.ne_table)
        return self.run_dual_queries(ns_q, ne_q)

    def get_max_src_core(self):
        maxq1 = f'SELECT max(t."srcCore") as "big" from {self.ne_table} t'
        maxq2 = f'SELECT max(t."srcCore") as "big" from {self.ns_table} t'
        with self.eng.connect() as con:
            m1 = con.execute(text(maxq1))
            m2 = con.execute(text(maxq2))
            return max(m1, m2)
