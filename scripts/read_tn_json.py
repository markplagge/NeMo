import shutil

import click
from tn_json_readers import createNeMoCFGFromJson, readAndSaveSpikeFile


class nOpts:
    def __init__(self, csvFN):
        self.csvFN = csvFN


@click.command()
@click.option('--modelname', default='nemo_', help="The file prefix used for this model/spike file parse run")
# @click.option('--cores', default=1024, help="The number of NS cores to simulate NOT IMPLEMENTED")
@click.option('--jmodel', help="The TN JSON file to read")
@click.option('--jspike', help="The TN JSON spike file to read")
@click.option('--nospike', help='Do not read spikes', default=False, is_flag=True)
@click.option('--debug', help='activate debug mode', default=False, is_flag=True)
def cli(modelname, jmodel, jspike, nospike, debug):
    ctx = nOpts(modelname)

    ctx.jspike = jspike
    ctx.jmodel = jmodel
    ctx.nospike = nospike
    ctx.debug = debug
    readJSON(ctx)


def readJSON(ctx):
    outPrefix = ctx.csvFN
    modelFile = ctx.jmodel
    spikeFile = ctx.jspike
    debug = ctx.debug
    v = ctx.nospike

    if v:
        click.secho("Spike db generation disabled...", fg='red')

    if not v:
        click.secho("Starting SPIKE parsing. Will take some time.", fg='blue')
        readAndSaveSpikeFile(filename=spikeFile, saveFile=outPrefix + "_spike.csv")
    else:
        click.secho('Specified no spike file reading. Continuing.', fg='blue')
    click.secho("Starting JSON read - will take some time.", fg='green')
    df = createNeMoCFGFromJson(modelFile, outPrefix + "_model", debug)
    df.closeFile()

    # remove the first two lines per new spec:
    with open(df.destination, 'r') as source_file:
        source_file.readline()
        source_file.readline()
        with open('trimmed_' + df.destination, 'w') as target_file:
            shutil.copyfileobj(source_file, target_file)


    #with open(df.destination, 'r') as f:
    #    fdat = f.readline()
    #fdat = f.readlines()
    #with open("trimed_" + df.destination, 'w') as f:
    #    f.writelines(fdat[2:])



if __name__ == '__main__':
    cli()
