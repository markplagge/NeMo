"""
Creates a small json with a specified number of cores from an input tn json file.
Use to test the NeMo
"""
import click
import tqdm
import re

def parse_file_split(open_file):
    strd = open_file.read()
    data = strd.split('"core":{')
    return data


def fix_split_core(core_data_str):
    s =  '"core":{'  + core_data_str
    return s


def fix_split_cores(core_data_list,n):
    """
fixes the cores after doing a split. First, the split eats the string "core":{ so we re-add it using
fix_split_core. Next, we see if there is a missing end }\n}, and add it using regex to the last core.

    :param core_data_list:
    :param n:
    :return:
    """
    import concurrent.futures

    #fixed_cores = [fix_split_core(c) for c in core_data_list[1:n]]
    fixed_cores = []
    with concurrent.futures.ProcessPoolExecutor(max_workers=2) as executor:
        results = [executor.submit(fix_split_core,c_str) for c_str in core_data_list]
        for future in concurrent.futures.as_completed(results):
            fixed_cores.append(future.result())


    regex = r"},..\/\*.Core parameters \*\/"
    regex_last = r".+(},.+/*.\/)$"
    regex_full_last = r"(},..\/\*.Core parameters \*\/)(?!\n\n)"
    last_line = "}\n}\n"


    #p = re.compile(regex,re.MULTILINE | re.DOTALL)
    p = re.compile(regex_full_last, re.MULTILINE|re.DOTALL)
    test_str = p.sub(last_line,fixed_cores[-1])

    fixed_cores[-1] = p.sub(last_line,fixed_cores[-1])
    all_cores = ''.join(fixed_cores)

    return all_cores




@click.command()
@click.option('-n', help="num cores to extract", default=16,type=click.INT)
@click.option('--split/--no-split', help="use split instead of generator", default=False)
@click.argument('inputf', type=click.File('r') )
@click.argument('outputf', type=click.File('w'))
def generate_small(n,split,inputf,outputf):

    if(split):
        data = parse_file_split(inputf)
        pre_core = data[0]
        cores = data[1:n + 2]
        outputf.write(pre_core)
        output_data = fix_split_cores(cores,n)
        outputf.write(output_data)
    else:
        pass


if __name__ == '__main__':
    generate_small()