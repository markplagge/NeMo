

#from .file_load import spike_file_reader
## configure dask client
import pathlib


import click_spinner
from click_spinner import spinner

from spike_validation import read_nscs_spikes, read_nemo_spike_files

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

modes=['mp','def','custom']
@click.command()
@click.argument('nscs_file',type=click.Path(exists=True))
@click.option('-n','--nemo_folder',help="Path to nemo files")
@click.option('-np','--nemo_pattern',default="fire_record_rank_*.csv")
@click.option('-m','--mode',default="def",type=click.Choice(modes))
@click.option('-d', '--data_folder', default="./")
@click.option('-r', '--refresh', default=False,help="Refresh cached data?")
@click.option('-a', default=True,help="Run analysis?")
@click.option('--one_shot', default=False, help="One Shot mode? Ignores caching options and runs as a complete"
                                                "dask graph")
def compare_nscs_nemo(nscs_file,nemo_folder,nemo_pattern,mode,data_folder,refresh,a,one_shot):
    nscs_data_file = 'nscs_data.dat'
    nemo_data_file = 'nemo_data.dat'

    #if data folder exists, and temp data files are there, and refresh is false,
    #reuse existing data.
    if(one_shot):
        one_shot(nscs_file,nemo_folder,nemo_pattern,mode)
        return 0
    p = pathlib.Path(data_folder)
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


if __name__ == '__main__':
    compare_nscs_nemo()
