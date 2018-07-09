/*
 * \file    registerIntervalCreation-Interface.h
 * \authors Ali Hajiabadi <hajiabadi@ce.sharif.edu>
 *          Mohammad Sadrosadati
 *          Amirhossein Mirhosseini
 *
 * \note: Register-interval creation algorithm is used in paper "LTRF: Enabling High-Capacity Register Files for GPUs via Hardware/Software Cooperative Register Prefetching" accepted in ASPLOS’18.
 *
 * \brief   Interface for register-interval creation algorithm
 *
 * \algorithm	register-interval creation
 * \input	
 *		1. The CFG of application (the nodes are basic blocks)
 *		2. The allowed number of registers in register-intervals
 * \output
 *		1. The reduced CFG of application that its nodes are register-intervals
 *
 * \purpose	The register-interval creation algorithm partitions the control flow graph of an application inot some register-intervals.
 *		Register-intervals have two main characteristic:
 *			1. They have only one entry point.
 *			2. They have a limited number of registers.
 *		These features of register-intervals are used in LTRF paper and we insert a PREFETECH instruction at the beginning of each register-interval.
 *		So during the execution of a register-interval all registers in its working set will be in the register file cache.
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <set>

#include "../implementation/ControlFlowGraph.cpp"

/* \brief This class implements the register-interval creation algorithm.*/
class RegisterIntervalCreationPass{
public:
    /* \brief the first pass of registerInterval creation algorithm
        -Input: Basic blocks of control flow graph, the maximum number of registers that each register-interval can contain
        -Output: Register-registerIntervals*/
    static std::vector<registerInterval*> registerIntervalCreationPassOne(std::vector<basicBlock*> inputBlocks, int registerNumber);
   
    /* \brief the second pass of register-interval creation algorithm
        -Input: Register-registerIntervals, the maximum number of registers that each register-interval can contain
        -Output: Reduced register-register-intervals*/
    static std::vector<registerInterval*> registerIntervalCreationPassTwo(std::vector<registerInterval*> inputIntervls, int registerNumber);
    
    /* \brief traverses a basic block instructions and splits it into two blocks if its number of registers
              is more than the maximum number.*/
    static void traverse(basicBlock*,int);
    
    /* \brief four functions that traverse the control flow graph (that the nodes are basic blocks or register-intervals)
              and marks the predecessors of a specified node.*/
    static void visit_all_predecessors(basicBlock*);
    static void visit_registerInterval_all_predecessors(registerInterval*);
    static void visit_predecessors(basicBlock*);
    static void registerInterval_visit_predecessors(registerInterval*);
};

#endif /* INTERVALCREATIONPASS_H */

