import click
import numpy as np
import os
from tn_api.read_TN import createNeMoCFGFromJson,readAndSaveSpikeFile, readSpikeFile
from tn_api.tn_nemo_api import ConfigFile

mcrs = 128

APP_NAME = "NeMo TN System"

nemoBin = os.path.join(click.get_app_dir(APP_NAME), 'bin')

class nOpts:
	def __init__(self, csvFileName):
		self.csvFileName = csvFileName


@click.group(chain=True)
@click.option('--fread', default=False, is_flag=True, help="Re-Read JSON")
@click.option('--csvname', default='tnmodel', help="The CSV file name generated via parsing.")
@click.option('--nemo', help='The location of the NeMo executable', default='../bin/NeMo')
@click.pass_context
def cli(ctx,fread,csvname,nemo):
	"""
	This script runs NeMo  with a set of TrueNorth json files, or the CSV files chosen.
	:param ctx: 
	:return: 
	"""
	ctx.obj = nOpts(csvname)
	ctx.obj.fread = fread
	ctx.obj.nemo = nemo
	ctx.obj.cores = 128





@click.command()
@click.option('--modelf', help="The TN core config JSON file to read in")
@click.option('--spikef',  help="The spike JSON file")
# @click.option('--output_name', help="Output CSV file name prefix", default='model')
@click.pass_obj
def read(params, modelf, spikef):

	output_name = params.csvFileName
	tempfile = "tn_model_tmp.csv"
	loadfile = output_name + ".csv"
	spikec = output_name + "_spike.csv"

	if (params.fread) or not os.path.exists(loadfile):
		# with click.progressbar(length=100, label="Processing JSON") as bar:
		click.echo("Starting JSON load (will take some time)")

		df = createNeMoCFGFromJson(modelf)

		df.save_csv(tempfile)
		with open(tempfile, 'r') as f:
			fdat = f.readlines()
			cores = fdat[0]
			with open(loadfile, 'w') as o:

				o.writelines(fdat[2:])


		#todo: remove temp file
		spikes = readAndSaveSpikeFile(filename=spikef, saveFile=spikec)
		params.cores = cores


@click.command()
#@click.option('--csv', help='input csv_name', default= 'model')
@click.option('--np', help="The maximum number of processes to run", default=2)
@click.option('--npc', help="The NP command in MPI", default="-np ")
@click.option('--mpi', help="Location of MPIRUN for NeMo", default='mpirun')
@click.option('--end', help="Simulate for this many ticks. (curr. not read from TN JSON)", default=1000)
@click.option('--synch', help='synch mode', default=3)
@click.argument('flags', default="")
@click.pass_obj
def run(params,np,npc, mpi,end,synch,flags):
	nemoFile = params.nemo
	csvModel = params.csvFileName + ".csv"
	spikeFile = params.csvFileName + "_spike.csv"

	nin = '--netin'
	nfn = f'--nfn={csvModel}'
	sfn = f'--sfn={spikeFile}'
	tm = '--tm=0'
	cm = f'--cores={params.cores}'


	cmd = f"{mpi} {npc}{np} {nemoFile} {nfn} {nin} {sfn} {tm} {end} {synch} {cm} {flags} "


	
	click.echo(f"Running nemo with command: {cmd}  ")

	click.pause()
	click.clear()


cli.add_command(read)
cli.add_command(run)
if __name__ == '__main__':
		cli()
