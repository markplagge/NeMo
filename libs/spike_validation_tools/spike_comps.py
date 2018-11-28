

#from .file_load import spike_file_reader
## configure dask client
import pathlib
from dask.distributed import Client, progress


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

@click.command()
@click.argument('nscs_file',type=click.Path(exists=True))
@click.option('--nemo_folder')
@click.option('-m','--mode',default="mp")
def compare_nscs_nemo(nscs_file,nemo_folder,mode):
    p = pathlib.Path(nemo_folder)
    if not p.exists():
        click.secho("Missing path to NeMo data: \n" + nemo_folder, fg='red')
        return 1






if __name__ == '__main__':
    compare_nscs_nemo()
