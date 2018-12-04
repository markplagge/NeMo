import sqlalchemy
from sqlalchemy import create_engine
from sqlalchemy.sql import text

import dask.dataframe as df

import dask

import pandas as pd


class SpikeDataInterface():
    table_base_name = 'spike_'
    return_pandas = False
    return_dask = False
    def __init__(self,connection_dsn='sqlite:///:./spike_data.sqlite',create_new=True,nscs_data=None,nemo_data=None):
        ## I want this class to handle both pandas and dask dataframes if we are creating a new table:
        if create_new:
            #assert(nscs_data != None)
            #assert(nemo_data != None)
            if isinstance(nemo_data, df.DataFrame):
                self.nemo_data = nemo_data.compute()
                del(nemo_data)
            #elif isinstance(nemo_data_)
            else:
                self.nemo_data = nemo_data
            if isinstance(nscs_data, df.DataFrame):
                self.nscs_data = nscs_data.compute()
                del(nscs_data)
            else:
                self.nscs_data = nscs_data

        self.ns_table = self.table_base_name + "nscs"
        self.ne_table = self.table_base_name + "nemo"
        self.eng = create_engine(connection_dsn)


        ### Set up table:

    def __gen_core_query(self,core_id,tbl):
        qry = f'SELECT "srcneuon", "destcore","destaxon","timestamp" FROM {tbl} WHERE srccore = {core_id}'
        return qry

        # qry = f"SELECT  {self.ns_table}.srcNeuron,  {self.ns_table}.destCore,{self.ns_table}.destAxon,{self.ns_table}.timestamp\
        #        {self.ne_table}.srcNeuron,  {self.ne_table}.destCore, {self.ne_table}.destAxon, {self.ne_table}.timestamp \n WHERE srcCore =  {core_id} \n"


    def __gen_group_query_core(self, tbl,core):
        qry = self.__gen_group_query(tbl)
        #qry = qry.replace("GROUP BY",'WHERE "srcCore" = ' + str(core) + ' \n GROUP BY')
        qry = qry + 'WHERE t."srcCore" = ' + str(core) + ";"
        return qry

    def __gen_group_query(self,tbl):
        ## Added this to use the nice view setup:
        tbl = tbl + "_grps"


        return f'SELECT t."srcCore",t."srcNeuron",t."destCore",t."destAxon",t."num_spikes" as t \n ' \
            f' FROM {tbl} t \n '
#            f'\n  GROUP BY t."srcCore",t."srcNeuron",t."destCore",t."destAxon" '

    def test_gq(self,tbl,core):
        print(self.__gen_group_query_core(tbl,core))
        print(self.__gen_group_query(tbl))


    def run_dual_queries(self,q1,q2):
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

    def get_core(self,core_id):

        ns_qry = self.__gen_core_query(core_id,self.ns_table)
        ne_qry = self.__gen_core_query(core_id,self.ne_table)
        return self.run_dual_queries(ns_qry,ne_qry)


    def mview_query(self,stbl):
        qry = f'create MATERIALIZED VIEW {stbl}_grps AS \n ' \
            f'select "srcCore", "srcNeuron","destCore","destAxon", COUNT("timestamp") ' \
            f'as num_spikes from {stbl} \n' \
            f'group by "srcCore", "srcNeuron","destCore","destAxon" \n' \
            f'order by "srcCore" ASC;'

        return qry

    def create_table(self):
        with self.eng.connect() as con:
            assert(isinstance(self.nemo_data,pd.DataFrame))
            print("saving NeMo table")
            self.nemo_data.to_sql(self.ne_table,con,if_exists='replace',chunksize=4096)
            print("saving NSCS table")
            self.nscs_data.to_sql(self.ns_table,con,if_exists='replace',chunksize=4096)
            self.ns_table_g = self.ns_table + "_grps"
            self.ne_table_g = self.ne_table + "_grps"

            print("creating view")
            view_1_q = self.mview_query(self.ns_table)
            view_2_q = self.mview_query(self.ne_table)
            con.execute(text(view_1_q))
            con.execute(text(view_2_q))



    def exec_arb_q(self,query):
        with self.eng.connect() as con:
            return pd.read_sql(query,con)

    def get_spike_counts_by_core(self,core_id):
            ns_q = self.__gen_group_query_core(self.ns_table,core_id)
            ne_q = self.__gen_group_query_core(self.ne_table,core_id)
            #print(ns_q)
            #print(ne_q)

            return self.run_dual_queries(ns_q,ne_q)

    def get_spike_counts_all(self):
        ns_q = self.__gen_group_query(self.ns_table)
        ne_q = self.__gen_group_query(self.ne_table)
        return self.run_dual_queries(ns_q,ne_q)

    def get_max_src_core(self):
        maxq1 = f'SELECT max(t."srcCore") as "big" from {self.ne_table} t'
        maxq2 = f'SELECT max(t."srcCore") as "big" from {self.ns_table} t'
        with self.eng.connect() as con:
            m1 = con.execute(text(maxq1))
            m2 = con.execute(text(maxq2))
            return max(m1,m2)
