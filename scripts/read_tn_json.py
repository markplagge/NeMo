import click
import os
from tn_json_readers import createNeMoCFGFromJson, readAndSaveSpikeFile


class nOpts:
	def __init__(self, csvFN):
		self.csvFN = csvFN


@click.group(chain=True)
@click.option('--modelname', default='nemo_', help="The file prefix used for this model/spike file parse run")
@click.option('--cores', default=1024, help="The number of NS cores to simulate NOT IMPLEMENTED")
@click.option('--jmodel', help="The TN JSON file to read")
@click.option('--jspike', help="The TN JSON spike file to read")
@click.pass_context
def cli(ctx, modelname,cores,jmodel,jspike):
	ctx.obj = nOpts(modelname)
	ctx.cores = cores
	ctx.jspike = jspike
	ctx.jmodel = jmodel
	readJSON(ctx)



def readJSON(ctx):
	outPrefix = ctx.csvFN
	modelFile = ctx.jmodel
	spikeFile = ctx.jspike

	click.echo("Starting JSON read - will take some time.")
	df = createNeMoCFGFromJson(modelFile,outPrefix+"_model")
	df.closeFile()
	with open(df.destination, 'r') as f:
		fdat = f.readline()

	with open(df.destination, 'w') as f:
		f.writelines(fdat[2:])

	readAndSaveSpikeFile(filename=spikeFile, saveFile=outPrefix+"_spike.csv")






