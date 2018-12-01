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
        self.ns_table = self.table_base_name + "nscs"
        self.ne_table = self.table_base_name + "nemo"
        self.eng = create_engine(connection_dsn)


        ### Set up table:

    def __gen_core_query(self,core_id,tbl):
        qry = f"SELECT srcneuon, destcore,destaxon,timestamp\"" \
            f"FROM {tbl} WHERE srccore = {core_id}"

        return qry

        # qry = f"SELECT  {self.ns_table}.srcNeuron,  {self.ns_table}.destCore,{self.ns_table}.destAxon,{self.ns_table}.timestamp\
        #        {self.ne_table}.srcNeuron,  {self.ne_table}.destCore, {self.ne_table}.destAxon, {self.ne_table}.timestamp \n WHERE srcCore =  {core_id} \n"

    def get_core(self,core_id):
        with self.eng.connect() as con:
            with self.eng.connect() as con2:
                ns_qry = self.__gen_core_query(core_id,self.ns_table)
                ne_qry = self.__gen_core_query(core_id,self.ne_table)
                if self.return_dask:
                    nscs_data = df.from_pandas(pd.read_sql(ns_qry,con),chunksize=128)
                    nemo_data = df.from_pandas(pd.read_sql(ne_qry,con2),chunksize=128)
                elif self.return_dask:
                    nscs_data = pd.read_sql(ns_qry,con)
                    nemo_data = pd.read_sql(ne_qry,con2)
                else:

                    nscs_data = con.execute(text(ns_qry))
                    nemo_data = con.execute(text(ne_qry))

                return (nscs_data,nemo_data)

    def create_table(self):
        with self.eng.connect() as con:
            assert(isinstance(self.nemo_data,pd.DataFrame))
            self.nemo_data.to_sql(self.ne_table,if_exists='replace')
            self.nscs_data.to_sql(self.ns_table,if_exists='replace')


    def exec_arb_q(self,query):
        with self.eng.connect() as con:
            return pd.read_sql(query,con)

