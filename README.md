#### Overview
Register-interval creation algorithm is a part of paper _**"LTRF: Enabling High-Capacity Register Files for GPUs via Hardware/Software Cooperative Register Prefetching"**_. This algorithm divides the control flow graph of the application into some register-intervals which have two main characteristics:
* Register-intevals have only one entry-point in CFG.
* They also have a limted number of registers.

In LTRF paper, a PREFETCH instruction is inserted at the beginning of each register-interval that brings its all registers to the register file cache.

#### Compiling
Register-interval creation codes are complied by _make_ command in terminal. The Makefile is in the main directory.

* Note: For compiling register-interval codes, c++11 is needed. So make sure that your g++ version supports c++11.

#### Running
After compiling, there is an executable file in the main directory named **"registerIntevalCreation"**. Two input arguments are needed to run this program:
1. The allowed number of registers in each register-interval.
2. The control flow graph of the application.

The format used for control flow graph file is Graphviz Dot that you can get this file from **_nvdisasm_** which is one of the CUDA binary utilities. More information in [this link](http://docs.nvidia.com/cuda/cuda-binary-utilities/index.html).

* Note: _nvdisasm_ only generates CFG for cuda binary files with Compute Capabilty 3.0 or later.

Here is an example of running register-interval creation alogorithm:
```{r, engine='bash'}
./registerIntervalCreation 16 bfs.dot 
```
After running, results and the output logs will be in _/output_ directory. File **"basicBlocks_{inputFileName}.txt"** will contain the details of the input control flow graph and its nodes are basic blocks. File **"registerInterval_{inputFileName}.txt"** will contain the information of created register-intervals and the resuced control flow graph that its nodes are register-intervals.
