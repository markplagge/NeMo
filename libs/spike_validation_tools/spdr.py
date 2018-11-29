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

rf = "/shared/share/superneuro/NeMo/libs"
nscs_filename = "./test_data/cifar_model/th_corelet_net_spikes0.json"
nemo_folder = "./test_data/cifar_model/"
nemo_pattern = "fire_record_rank_*.csv.gz"
scheduler_file = "/shared/share/superneuro/dask_work/scheduler.json"


c = Client(scheduler_file=dask_sched_file)
