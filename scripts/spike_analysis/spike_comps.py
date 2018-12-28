
# from .file_load import spike_file_reader
## configure dask client
import pathlib


import click_spinner
from click_spinner import spinner
from spike_accuracy.spike_validation import read_nscs_spikes, read_nemo_spike_files, STAT
import concurrent
from dask.distributed import Client, progress
import dask
import dask.dataframe as df
from concurrent.futures import ThreadPoolExecutor as ThreadPool
tpool = ThreadPool(max_workers=2)
import click
import os
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper
import yaml

## Global values:
# config = {
# "dask_cluster_settings": {"address": None,
#                                      "processes": True,
#                                      "threads_per_worker": 5,
#                                      "n_workers": 5,
#                                      "memory_limit": None},
# 'spike_match_file':'./output.csv'}
config_dumper = None
config_loader = None
file_config = {}
modes =['thread', 'proc', 'cluster']

## configuration object

class Config(object):
    def __init__(self, database="postgres://plaggm@localhost", spike_match_file='./output.csv', dask_cluster_settings=None,
                 mode='thread',default_dask_cluster=True,context=None):


        if dask_cluster_settings is None:
            dask_cluster_settings = {"address": None,
                                     "processes": True,
                                     "threads_per_worker": 5,
                                     "n_workers": 5,
                                     "memory_limit": None}
        if context is None:
            context = {"base_ctx_item":"base_ctx_value"}
        self.context = context
        self.database = database
        self.spike_match_file = spike_match_file
        self.mode = mode
        self.dask_cluster_settings = dask_cluster_settings
        self.default_dask_cluster =default_dask_cluster

    def __repr__(self):
        return f"{self.__class__.__name__}(database={self.database}, " \
            f"spike_match_file={self.spike_match_file}, " \
            f"dask_cluster_settings={self.dask_cluster_settings},mode={self.mode}, default_dask_cluster={self.default_dask_cluster},contex={self.context})"


def init_client(pp_cfg):
    client = Client(**pp_cfg)
    Client()
    print(client)
    return client


def init_client_ovr(tpw=2 ,nw=10 ,mem='6gb' ,p=True):
    client = Client(processes=p, threads_per_worker = tpw, n_workers = nw, memory_limit = mem)
    client.restart()
    print(client)
    return client


def init_dask_cluster(dask_config):
    return init_client(dask_config)



def load_config_file(cfg_filename):
    global config
    p = pathlib.Path(cfg_filename)
    if p.exists() and p.is_file():
        click.secho("loading config file", fg="green")
        with open(cfg_filename, 'r') as cfg_file:
            config = yaml.load(cfg_file)
        return True

    else:
        click.secho("cfg file not found. Will create.",fg="yellow")
        config = Config()
        return False


def save_config_file(cfg_filename):
    global config
    with open(cfg_filename, 'w') as cfg_file:
        yaml.dump(config,cfg_file,default_flow_style=False)

def interactive_analysis(dsn):
    iface = spike_accuracy.SpikeDataInterface(dsn, create_new=False)
    q_holder = spike_accuracy.MissingSpikeSQL(iface)
    print("Query holder ready")
    print("adding spike-core query")
    spike_core_query =spike_accuracy.SpikeQuery_FULL_SRC_COMP()
    q_holder.add_query_object(spike_core_query)
    click.secho("Adding sql spike-core query")
    spike_core_query = spike_accuracy.SpikeQuery_SCN_DCN_SQL_GRP()
    q_holder.add_query_object(spike_core_query)
    return q_holder



@click.group(chain=True)
@click.option('-d', '--database', default="CC", help="Database DSN for sqlalchemy")
@click.option('--cfgfile', default='./sa_settings.yaml', help="Config file for dask")
@click.option('-m' ,'--mode' ,default="thread" ,type=click.Choice(modes) ,help="What DASK mode should we run in?\n"
                                                                           "thread: threaded mode (GIL LOCK)\n"
                                                                           "proc: multiprocessing mode\n"
                                                                           "cluster: initialize cluster.\n"
                                                                           "For cluster mode, settings will be read"
                                                                           "if file exists from cluster.yaml in current folder. (TODO)")
@click.option('-r' ,'--use_cfg', default=False, is_flag=True, help="use config file - flags override set parameters")
@click.option('-s', '--save_cfg', default=False, is_flag=True, help='save cfg file')
@click.pass_context
def cli(ctx ,database ,cfgfile ,mode,use_cfg,save_cfg):
    print(ctx.obj)
    print(os.getcwd())
    global config
    p = pathlib.Path(cfgfile)
    ldl = False
    if p.exists() and p.is_file():
        ldl = load_config_file(cfgfile)
    if not ldl:
        config=Config(database=database,mode=mode,context=ctx.obj)

    if use_cfg:
        if database == "CC":
            database = config.database
        if mode == 'thread':
            mode = config.mode

    ctx.obj['database'] = database
    ctx.obj['cfgfile'] = cfgfile
    ctx.obj['mode'] = mode
    if mode == 'proc':
        dask.config.set(scheduler="processes")
    elif mode == 'cluster':
        c = Client(processes=True)
        print("using client / cluster mode:")
        print(c)
    ctx.obj['save_cfg'] = save_cfg
    ctx.obj['use_cfg'] = use_cfg

@cli.command()
@click.pass_context
def save_cfg(ctx):
    global config
    config.context = ctx.obj
    save_config_file(ctx.obj['cfgfile'])


@cli.command()
@click.option('--out_data_fn' ,default="./spike_counts.csv", help="Spike (mis)match CSV file will be saved here.")
@click.option('-C', default=False, help="Generate CORE->CORE communication pattern statistics",is_flag=True)
@click.option('--no_nscs', default=False, is_flag=True, help="Do not do NSCS analysis.")
@click.pass_context
def data_analysis(ctx ,out_data_fn,c,no_nscs):
    click.secho("Connecting to " + ctx.obj['database'] + " for data analysis" ,fg="green")
    iface = ctx.obj.get('iface')

    if iface is None:
        iface = spike_accuracy.SpikeDataInterface(ctx.obj['database'],create_new=False)

    iface.return_dask = True
    iface.return_pandas = True

    click.secho("Missing/Extra Spike Query Generation", fg="green")
    sqobj = spike_accuracy.MissingSpikeSQL(iface)

    qo = spike_accuracy.SpikeQuery_SCN_DCN()
    qo.run_full_q = True

    sqobj.add_query_object(qo)
    if c:
        pass


    sqobj.add_query_object(qo)
    qo = spike_accuracy.SpikeQuery_SCN_DCN_SQL_GRP()
    sqobj.add_query_object(qo)
    sqobj.execute_queries()
    results = sqobj.get_query_results()
    ii = 0
    import pandas as pd
    for r in results:
        if isinstance(r,dask.dataframe.DataFrame): #or isinstance(r,pd.DataFrame):
            fn = f"{out_data_fn.replace('.csv','')}_q{ii}.csv"
            click.secho("saving result to " + fn, fg="green")
            r = r.compute()

        r.to_csv(fn,sep=',')
        ii += 1

    i = 0



    """
    
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

    if mode == 'cluster':
        c.close()

    """


@cli.command()
@click.option('--nscs',default=None, help="Path to NSCS file")
@click.option('--load_nscs', default=True, is_flag=True, help="Load NSCS Spikes")
@click.option('-n' ,'--nemo_folder' ,default = None, help="Path to nemo files")
@click.option('-np' ,'--nemo_pattern' ,default="fire_record_rank_*.csv")
@click.option('-r', '--refresh', default=False ,help="Refresh cached data? If specified, will reload data into the DB"
              ,is_flag=True)
@click.option('--nscs_cl', default=False, is_flag=True, help="Load the NSCS file in small chunks into the postgres database.")
@click.pass_context
def load_data(ctx, nscs, load_nscs, nemo_folder, nemo_pattern, refresh, nscs_cl):
    global config
    #main cli entrypoint loads this
    database = ctx.obj['database']
    ctx.obj['load_nscs'] = load_nscs
    #are we loading the config file? if so, check for defaults and override
    if ctx.obj['use_cfg']:
        try:
            if nscs is None:
                nscs = config.context['nscs']
            if nemo_folder is None:
                nemo_folder = config.context['nemo_folder']
            if nemo_pattern == 'fire_record_rank_*.csv':
                nemo_pattern = config.context['nemo_pattern']
            if nscs_cl is False:
                nscs_cl = config.context['nscs_cl']
            if refresh is False:
                refresh = config.context['refresh']
        except KeyError as e:
            click.secho("could not find " + str(e), fg="yellow")
            exit(1)

    ## save these files in the context so that they can be saved to the config file.
    ctx.obj['nemo_folder'] = nemo_folder
    ctx.obj['nemo_pattern'] = nemo_pattern
    ctx.obj['nscs'] = nscs
    ctx.obj['nscs_cl'] = nscs_cl
    ctx.obj['refresh'] = refresh

    if nemo_folder is not None:
        if not nemo_folder.endswith('/'):
            nemo_folder = nemo_folder + "/"

    if ctx.obj['save_cfg']:
        save_config_file(ctx.obj['cfgfile'])

    #finished config/option stuff
    iface = spike_accuracy.SpikeDataInterface(database ,check_only=True)
    status = iface.status

    full_ok = status & ( STAT.NEMO_VIEW_OK & STAT.NSCS_TABLE_OK & STAT.NSCS_VIEW_OK & STAT.NEMO_TABLE_OK)
    if bool(full_ok):
        exist = True
    else:
        exist = False

    print(status)
    if not refresh:
        y = "Yes\n"
        n = "No\n"
        msg = "Table Exist Status: \n NeMo Spike: "
        if status.NEMO_TABLE_ERR:
            msg += n
        else:
            msg += y
        msg += "NSCS Table: "
        if status.NSCS_TABLE_ERR:
            msg += n
        else:
            msg += y
        msg += "\nM.View Exist: \n NeMo: "
        if status.NEMO_VIEW_ERR:
            msg += n
        else:
            msg += y
        msg += "\nNSCS: "
        if status.NSCS_VIEW_ERR:
            msg += n
        else:
            msg += y

        click.secho(msg, fg="blue")
        click.secho("Ensure that missing table data is specified.", fg="yellow")
    ### All table refresh logic was moved to the spike_sql_iface interface class.
    ### So we don't need any extra logic here - except for file loading options
    nscs_data = None
    nemo_data = None
    ## See if we can load files:
    if status.NEMO_TABLE_ERR or refresh:
        try:
            nemo_data = spike_accuracy.read_nemo_spike_files(nemo_folder ,nemo_pattern)
        except:
            click.secho("Could not load nemo data from " + nemo_folder ,fg="red")

    if (status.NSCS_TABLE_ERR or refresh) and  load_nscs:
        if nscs_cl:
            nscs_data = nscs
        else:
            try:
                nscs_data = read_nscs_spikes(nscs)
            except:
                click.secho("Could not load NSCS JSON file: " + nscs, fg="red")

    ## Files are loaded. Create the SQL data interface
    iface = spike_accuracy.SpikeDataInterface(database ,refresh ,nscs_data ,nemo_data ,nscs_cl)
    ctx.obj['iface'] = iface
    click.secho("Database connected and generated." ,fg="green")
    click.secho("Can now run analysis mode.")






import spike_accuracy

if __name__ == '__main__':
    ## DEMO MODE:

    # dask.config.set(scheduler='processes')
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
    cli(obj={})
