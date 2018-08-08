import collections
import click




def extract_core_data(line):
    s_core_start = line.find('coreID')
    s_core_end = line.find(',localID')
    s_core_txt = line[s_core_start:s_core_end]
    s_core = s_core_txt.split('=')[1]

    d_core_start = line.find('destCore')
    d_core_end = line.find(',destLocal')
    d_core_txt = line[d_core_start:d_core_end]
    d_core = d_core_txt.split('=')[1]

    return s_core,d_core ## STILL STRINGS



def get_source_dest_cores_from_lines(lines):
    connections = collections.defaultdict(list)
    for line in lines:





@click.command()
@click.option("nfg_file", type=click.File('r'))
def loadN(nfg_file):
    nf = nfg_file
    line = nf.readline()
    while ("coreID" not in line):
           line = nf.readline()
    click.secho("Starting core connectivity data gen from NFG file...",fg="green")

    

if __name__ == "__main__":
    print("Starting data gen.")
    loadN()
