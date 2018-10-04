import platform
import os

numruns = 1000
resultdir = "results_long"
rio="RIO0"
command = "latency.exe"

if os.name == 'posix':
    rio = "RIO1"
    command = "build/src/latency"

if not os.path.isdir(resultdir):
    os.mkdir(resultdir)
if not os.path.isdir(resultdir):
    raise RuntimeError(f"Could not create directory {resultdir}")

print(str(platform.uname()), file=open(resultdir+"/_metadata.txt", "w"))

os.system(f"{command} -d {resultdir} -r {rio} -p 1 --numruns {numruns}")
os.system(f"{command} -d {resultdir} -r {rio} -p 2 --numruns {numruns}")
os.system(f"{command} -d {resultdir} -r {rio} -p 4 --numruns {numruns}")
os.system(f"{command} -d {resultdir} -r {rio} -p 8 --numruns {numruns}")
os.system(f"{command} -d {resultdir} -r {rio} -p 16 --numruns {numruns}")
