
import argparse

def read_chunks(open_file):
        while True:
            data = open_file.readline()
            if not data:
                break
        yield data


def prsr(l):
    l = l.split(',')[1]
    l = l.replace(' ', ',')
    return l

def interleave(out_file_name, rcvr_fn, sndr_fn):
    with open(out_file_name, 'w') as out_file:
        out_file.write("<mpiType>,<src>,<dst>,<wallStart>,<wallStop>,<cpuStart>,<cpuStop>,<count>,<dataType>,<comm>,<tag>\n")
        with open(rcvr_fn, 'r') as rcvr_file:
            with open(sndr_fn, 'r') as sndr_file:
                output = []
                for l in rcvr_file:
                    out_file.write(prsr(l))
                for l in sndr_file:
                    out_file.write(prsr(l))

def interleavelist(out_file_name, inlist):
    with open(out_file_name, 'w') as out_file:
        out_file.write(
            "<mpiType>,<src>,<dst>,<wallStart>,<wallStop>,<cpuStart>,<cpuStop>,<count>,<dataType>,<comm>,<tag>\n")
        for filename in inlist:
            with open(filename, 'r') as in_file:

                for l in in_file:
                    out_file.write(prsr(l))

def apparse(out_file, in_file):
    for l in in_file:
        out_file.write(prsr(l))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='interleave two NeMo dumpi files')
    parser.add_argument('--out', nargs=1, help="output filename", default="interleaved.txt")
    parser.add_argument("files", nargs="*", help="process these files", type=argparse.FileType('r'))

    arg = parser.parse_args()

    #interleave(arg.out,arg.in1,arg.in2)
    with open(arg.out, 'w') as out_file:
        out_file.write(
            "<mpiType>,<src>,<dst>,<wallStart>,<wallStop>,<cpuStart>,<cpuStop>,<count>,<dataType>,<comm>,<tag>\n")
        for file in arg.files:
            apparse(out_file,file)
