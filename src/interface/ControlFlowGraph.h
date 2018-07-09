/*
 * \file    ControlFlowGraph.h
 * 
 * \authors Ali Hajiabadi <hajiabadi@ce.sharif.edu>
 *          Mohammad Sadrosadati
 *          Amirhossein Mirhosseini
 *
 * \brief   Interface for Control Flow Graph of application
 *          Note: The nodes of CFG can be basic blocks or register-intervals
 * 
 * Note: Register-interval creation algorithm is used in paper "LTRF: Enabling High-Capacity Register Files for GPUs via Hardware/Software Cooperative Register Prefetching" accepted in ASPLOS’18.
 */

#include <string>
#include <set>
#include <vector>

/* \brief registerInterval contains some basicBlocks and it has only one entry point
     and limited number of registers*/
class registerInterval
{
public:
	registerInterval();
	~registerInterval();

        /*
         * Setters of registerInterval class
         */
public:
        /* Set the series of instructions in registerInterval*/
	void set_code(std::string);
        /* Set the register list of registerInterval*/
	void set_register_list(std::set<std::string>);
        /* Add a predecessor for registerInterval*/
	void add_predecessor(registerInterval*);
        /* Add a predecessor for registerInterval*/
	void add_successors(registerInterval*);
        /* Set the next-level registerInterval that this registerInterval belongs to
            note: next-level registerInterval is used to reduce the number of registerIntervals and construct larger ones.*/
	void set_next_level_registerInterval(registerInterval*);


	/*
         * Getters of registerInterval class
         */
public:
        /* Get the id of registerInterval*/
	int getID();
        /* Get the number of instructions in registerInterval*/
	int getNumberOfInstructions();
        /* Get the register list of registerInterval*/
	std::set<std::string> get_register_list();
        /* Get the series of instructions in registerInterval*/
	std::string get_code();
        /* Get the register vector that specifies which registers are used in this registerInterval*/
	std::vector<bool> get_register_vector();
        /* Get the predecessors of registerInterval*/
	std::vector<registerInterval*> get_predecessors();
        /* Get the successors of registerInterval*/
	std::vector<registerInterval*> get_successors();
        /* Get the next-level registerInterval that this registerInterval belongs to*/
	registerInterval* nextLevelInterval();

        /*    Data    */
private:
	static int IDgenerator;
	int ID;
	std::vector<bool> register_vector;
	registerInterval* next_level_registerInterval;
	std::string code;
	int registerInterval_numberOfInstructions;
	std::set<std::string> register_list;
	std::vector<registerInterval*> predecessors;
	std::vector<registerInterval*> successors;
};


/*\brief A basic block contains a series of instructions
 *       terminated by control flow 
 */
class basicBlock
{
public:
        /*Constructors of basicBlock*/
	basicBlock();
	basicBlock(std::string);
	basicBlock(int);
	~basicBlock();

	/*
         * Setters of basicBlock class
         */
public:
        /* Get the name of basicBlock*/
	void set_name(std::string);
        /* Get the series of instructions of basicBlock*/
	void set_code(std::string);
        /* Add a register to the register set of basicBlock*/
	void add_register(std::string);
        /* Add a predecessor for basicBlock*/
	void add_predecessor(basicBlock*);
        /* Add a successor for basicBlock*/
	void add_successor(basicBlock*);
        /* Set the successors of basicBlock*/
        void set_successors(std::vector<basicBlock*>);
        /* Set the predecessors of basicBlock*/
        void set_predecessors(std::vector<basicBlock*>);
        /* Delete one of the successors of basicBlock*/
	void delete_successor(basicBlock*);
        /* Set the output register list of basicBlock*/
	void set_output_list(std::set<std::string>);
        /* Set the input register list of basicBlock*/
	void set_input_list(std::set<std::string>);
        /* Clear the output register list basicBlock*/
	void clear_output_list();
        /* Clear the input register list of basicBlock*/
	void clear_input_list();
        /* Set the registerInterval that this basicBlock belongs to*/
	void set_registerInterval(registerInterval*);
        /* Set the abstract basicBlock that this basicBlock belongs to*/
	void set_ABB(int);
        /* Set if this basicBlock is a controling basicBlock*/
	void set_controling();
        /* Set the controling basicBlock of this basicBlock*/
	void set_control_basicBlock(basicBlock*);
        /* Set if this basicBlock is the exit basicBlock*/
	void set_exit_state();


	/*
         * Getters of basicBlock class
         */
public:
        /* Get the id of basicBlock*/
	int getID();
        /* Get the name of basicBlock*/
	std::string get_name();
        /* Get the instructions of basicBlock*/
	std::string get_code();
	/* Get the number of instructions in basic block*/
	int numberOfInstructions();
        /* Get the number of registers of basicBlock*/
	int numberOfRegisters();
        /* Get the output register list of basicBlock*/
	std::set<std::string> get_output_list();
        /* Get the input register list of basicBlock*/
	std::set<std::string> get_input_list();
        /* Get the predecessors of basicBlock*/
	std::vector<basicBlock*> get_predecessors();
        /* Get the successors of basicBlock*/
	std::vector<basicBlock*> get_successors();
        /* Get the registerInterval that this basicBlock belongs to*/
	registerInterval* RegisterInterval();
        /* Get the Abstract basicBlock that this basicBlock belongs to*/
	int get_ABB();
        /* Is this basicBlock controling*/
	bool is_controling();
        /* Get the controling basicBlock of this basicBlock*/
	basicBlock* get_controling_basicBlock();
        /* Is this basicBlock the exit basicBlock*/
	bool is_exit_state();
        

        /*    Data    */
private:
	static int IDgenerator;
	static int IDgenerator2;
	int ID;
	std::string name;
	std::string code;
	int NumberOfInstructions;
	int ABB;  //Abstract Basic Block
	bool controlBB;
	bool exit_state;
	basicBlock* controling_basicBlock;
	std::vector<basicBlock*> predecessors;
	std::vector<basicBlock*> successors;
	std::set<std::string> input_list;
	std::set<std::string> output_list;
	registerInterval* BB_registerInterval;
};
