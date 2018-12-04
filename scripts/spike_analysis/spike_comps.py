

#from .file_load import spike_file_reader
## configure dask client
import pathlib


import click_spinner
from click_spinner import spinner

from spike_accuracy.spike_validation import read_nscs_spikes, read_nemo_spike_files

import concurrent
from dask.distributed import Client, progress
import dask
import dask.dataframe as df

from concurrent.futures import ThreadPoolExecutor as ThreadPool
tpool = ThreadPool(max_workers=2)

import click




def init_client_default():
    client = Client(processes=True)
    client.restart()
    print(client)
    return client


def init_client_ovr(tpw=2,nw=10,mem='6gb',p=True):
    client = Client(processes=p, threads_per_worker = tpw, n_workers = nw, memory_limit = mem)
    client.restart()
    print(client)
    return client


def init_dask_mp(nw=10):


    return True


def init_data(nscs_file,nemo_folder,nemo_pattern,mode,**kwargs):


    click.secho("Loading NSCS and NeMo Data into dataframes.")
    with click_spinner.spinner():
        ## NSCS load:
        nscs_data = read_nscs_spikes(nscs_file)
        nemo_data = read_nemo_spike_files(nemo_folder,nemo_pattern)

    return nscs_data,nemo_data



def load_cached_data(nscs_df,nemo_df):
    nscs_data = df.read_csv(nscs_df)
    nemo_data = df.read_csv(nemo_df)
    return nscs_data, nemo_data



def create_cached_data(data_folder,nemo_cache_name,nscs_cache_name,nscs_data,nemo_data):
    click.secho("Saving NeMo data to " + data_folder + nemo_cache_name)

    nemo_data.compute().to_csv(data_folder + nemo_cache_name)
    click.secho("Saving NSCS data to " + data_folder + nscs_cache_name)
    nscs_data.compute().to_csv(data_folder + nscs_cache_name)

    return True




def data_analysis(nscs_data,nemo_data,mode,cached=False):
    pass


def one_shot(nscs_file,nemo_folder,nemo_pattern,mode,**kwargs):
    pass

def gen_sql(nscs_data,nemo_data):
    pass


import sqlalchemy
from sqlalchemy import create_engine
modes=['mp','def','custom']
@click.command()
@click.argument('nscs_file',type=click.Path(exists=True))
@click.option('-n','--nemo_folder',help="Path to nemo files")
@click.option('-np','--nemo_pattern',default="fire_record_rank_*.csv")
@click.option('-m','--mode',default="def",type=click.Choice(modes))
@click.option('-d', '--database', default="postgres://plaggm@localhost")
@click.option('-r', '--refresh', default=False,help="Refresh cached data?",is_flag=True)
@click.option('-a', default=True,help="Run analysis?")
@click.option('--one_shot', default=False, help="One Shot mode? Ignores caching options and runs as a complete"
                                                "dask graph")
@click.option('--out_data_fn',default="./spike_counts.csv")
#@click.option('--db_dsn',default="")
def compare_nscs_nemo(nscs_file,nemo_folder,nemo_pattern,mode,database,refresh,a,one_shot,out_data_fn):
    nscs_data_file = 'nscs_data.dat'
    nemo_data_file = 'nemo_data.dat'

    #if data folder exists, and temp data files are there, and refresh is false,
    #reuse existing data.
    if(one_shot):
        one_shot(nscs_file,nemo_folder,nemo_pattern,mode)
        return 0


    ## test tables:
    #eng = sqlalchemy.create_engine(database)
    #c = eng.connect()
    if refresh:
        click.echo("Creating table.")
        nscs_data,nemo_data = init_data(nscs_file,nemo_folder,nemo_pattern,mode)
        iface = spike_accuracy.SpikeDataInterface(database,refresh,nscs_data,nemo_data)
        iface.create_table()
    else:
        iface = spike_accuracy.SpikeDataInterface(database,refresh)
    click.echo("Database connected. Generating spike report")
    sqobj = spike_accuracy.MissingSpikeSQL(iface)
    click.echo("will generate report with spike counts and missing values")
    qo = spike_accuracy.SpikeQuery_SCN_DCN()
    qo.run_full_q = True
    sqobj.add_query_object(qo)
    sqobj.execute_queries()
    click.echo("results generated. saving to " + out_data_fn)
    results = sqobj.get_query_results()
    i = 0
    for r in results:
        r.to_csv(str(i) + out_data_fn)
        i += 1
    click.echo("done!")
    return


    if(p.exists()):
        if mode == 'custom':
            client = init_client_ovr()
        elif mode == 'def':
            client = init_client_default()
        else:
            client = init_dask_mp()

        ## client is set up. Load data.
        with pathlib.Path(data_folder + nscs_data_file) as nscs_df:
            with pathlib.Path(data_folder + nemo_data_file) as nemo_df:
                if nscs_df.exists() and nemo_df.exists and not refresh:
                        click.secho("Using existing cached data", fg='green')
                        nscs_data,nemo_data = load_cached_data(nscs_df.nemo_df)
                        cached=True
                else:
                    click.secho("Generating cache data")
                    nscs_data, nemo_data = init_data(nscs_file,nemo_folder,nemo_pattern,mode)
                    cached=False

        if a:
            click.secho("Running analysis.", fg="green")
            data_analysis(nscs_data,nemo_data,mode,cached)

    else:
        click.secho("invalid data folder chosen.")
        return 2

import spike_accuracy
if __name__ == '__main__':
    ## DEMO MODE:

    dask.config.set(scheduler='processes')
    # qf = spike_accuracy.SpikeDataInterface("postgres://plaggm@localhost", create_new=False)
    # qf.ne_table = "nemo_spike"
    # qf.ns_table = "nscs_spike"
    # qf.test_gq(qf.ne_table, 500)
    # msq = spike_accuracy.MissingSpikeSQL(qf)
    # qo = spike_accuracy.SpikeQuery_SCN_DCN()
    # qo.run_full_q=True
    # msq.add_query_object(qo)
    # msq.execute_queries()
    # results = msq.query_objects[0].result
    # r = results.compute()
    # print("found  " + str(len(results))+ " records.")
    # r.to_csv("spike_demo.csv")





    compare_nscs_nemo()