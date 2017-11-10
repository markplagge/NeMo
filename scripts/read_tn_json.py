import click
import os
from tn_json_readers import createNeMoCFGFromJson, readAndSaveSpikeFile
import shutil


class nOpts:
    def __init__(self, csvFN):
        self.csvFN = csvFN


@click.command()
@click.option('--modelname', default='nemo_', help="The file prefix used for this model/spike file parse run")
# @click.option('--cores', default=1024, help="The number of NS cores to simulate NOT IMPLEMENTED")
@click.option('--jmodel', help="The TN JSON file to read")
@click.option('--jspike', help="The TN JSON spike file to read")
def cli(modelname, jmodel, jspike):
    ctx = nOpts(modelname)

    ctx.jspike = jspike
    ctx.jmodel = jmodel
    readJSON(ctx)


def readJSON(ctx):
    outPrefix = ctx.csvFN
    modelFile = ctx.jmodel
    spikeFile = ctx.jspike

    click.echo("Starting JSON read - will take some time.")
    df = createNeMoCFGFromJson(modelFile, outPrefix + "_model")
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

    readAndSaveSpikeFile(filename=spikeFile, saveFile=outPrefix + "_spike.csv")


if __name__ == '__main__':
    cli()
