import click
import numpy as np
import os
from scripts.read_TN_json import createTNNeMoConfig,readSpikeFile
from scripts.read_TN_spikes import *


APP_NAME = "NeMo TN System"

nemoBin = os.path.join(click.get_app_dir(APP_NAME), 'bin')


@click.command()
@click.option('--modelf', help="The TN core config JSON file to read in",
			  type=click.File('r'))
@click.option('--spikef',  help="The spike JSON file", type=click.File('r'))
@click.option('--np', help="The maximum number of processes to run")
@click.option('--mpi', help="Location of MPIRUN for NeMo (If not present uses system mpirun)", default='')
@click.option('--end', help="Simulate for this many ticks.", default=1000)
def run_tn_model(modelf,spikef, np, mpi,end):
	df = createTNNeMoConfig(modelf)

	tempfile = "tn_model_tmp.csv"
	loadfile = "tn_model.csv"
	spikec = "tn_spikes.csv"
	extramem = 6553500


	df.save_csv(tempfile)
	with open(tempfile, 'r') as f:
		with open(loadfile, 'w') as o:
			fdat = f.readlines()
			cores = fdat[0]
			o.writelines(fdat[2:])

	#todo: remove temp file
	spikes = readSpikeFile(spikef, 'json')
	csv = [x.to_csv() for x in spikes]

	with open(spikec, 'w') as f:
		for l in csv:
			f.write(l)


if __name__ == '__main__':
		run_tn_model()
