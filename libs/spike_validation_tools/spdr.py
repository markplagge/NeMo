# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""
script_folder = "/shared/share/superneuro/NeMo/libs/spike_validation_tools/"
dask_sched_file = "/shared/share/superneuro/dask_work/scheduler.json"
import os
os.chdir("/shared/share/superneuro/NeMo/libs/spike_validation_tools/")
from dask.distributed import Client,progress
import dask.dataframe as df
import spike_comps
from spike_validation.comparisons import MissingSpikes,SpikeQuery,SpikeQuery_SCN_DCN,SpikeQuery_SCN_DCN_Time


egg_loc = "/shared/share/superneuro/NeMo/libs/spike_validation_tools.zip"
rf = "/shared/share/superneuro/NeMo/libs"
nscs_filename = "./test_data/cifar_model/th_corelet_net_spikes0.json"
nemo_folder = "./test_data/cifar_model/"
nemo_pattern = "fire_record_rank_*.csv"
scheduler_file = "/shared/share/superneuro/dask_work/scheduler.json"
ip = "128.213.17.109"
diag = 9999
mem = "12GB"
#c = Client(scheduler_file=dask_sched_file)
client = Client(n_workers=2,diagnostics_port=diag,ip=ip)


nscs_data,nemo_data = spike_comps.init_data(nscs_filename,nemo_folder,nemo_pattern,"")


qobj = MissingSpikes(nscs_data,nemo_data)

qobj.add_query_object(SpikeQuery_SCN_DCN())
qobj.execute_queries()
