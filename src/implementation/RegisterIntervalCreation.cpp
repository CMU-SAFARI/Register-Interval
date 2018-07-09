/*
 * \file    RegisterIntervalCreation.cpp
 *
 * \authors      Ali Hajiabadi <hajiabadi@ce.sharif.edu>
 *               Mohammad Sadrosadati
 *               Amirhossein Mirhosseini
 *
 * \brief   Implementation of register-interval creation algorithm
 * 
 * Note: Register-interval creation algorithm is used in paper "LTRF: Enabling High-Capacity Register Files for GPUs via Hardware/Software Cooperative Register Prefetching" accepted in ASPLOS’18.
 */

#include <vector>
#include <set>
#include <queue>

#include "../interface/RegisterIntervalCreation.h"

using namespace std;

vector<basicBlock*> basic_blocks; //set of all basic blocks of application
vector<registerInterval*> RegisterIntervals; //the set of register-intervals that contains the final register-intervals
queue<basicBlock*> WS; //The Working Set of register-interval creation algorithm (Pass 1)

vector<bool> mark; //The bit vector that is used to mark predecessors of basic blocks
vector<bool> registerInterval_mark; //The bit vector that is used to mark predecessors of register-intervals

vector<registerInterval*> RegisterIntervalCreationPass::registerIntervalCreationPassOne(vector<basicBlock*> BB_vector, int N){ //N is the maximum number of registers allowed in a register-interval 
        basic_blocks = BB_vector;
    
        /* Initialization */
	for (auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
		(*b)->clear_input_list();
		(*b)->set_registerInterval(nullptr);
	}

	RegisterIntervals.clear();
	for(auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
		auto Predecessors = (*b)->get_predecessors();
		if(Predecessors.size() == 0){ // each basic block with no predecessors is assumed as an entry basicBlock
			auto temp = new registerInterval();
			RegisterIntervals.push_back(temp);
			(*b)->set_registerInterval(temp);
			WS.push(*b);// The entry basic block is added to the working set of registerInterval creation algorithm
		}
	}

	while (!WS.empty()){
		auto BB = WS.front(); //Get one basic blocks from working set
		WS.pop();
		auto i = BB->RegisterInterval(); //Get the register-interval that this basic block belongs to
		traverse(BB, N); //Traversing the basic block and splitting it if it has more than the allowed number of registers
		if (BB->numberOfRegisters() < N){
                        //In this loop we will add all eligible basic blocks to the current registerInterval (i)
			for (auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
				set<string> Union;//The set that will contain the union of register sets of all basic blocks of i
                                Union.clear();
				bool C = true;//This boolean determines that the basicBlock is reachable only from the current register-interval (in other words, its all predecessors should belong to the current register-interval)

				visit_all_predecessors(*b);//marks all basic blocks that can reach this basic block

				Union = (*b)->get_output_list();
                                //This loop will add all registers of marked basic blocks that belong to the current register-interval (i)
				for(auto bb = basic_blocks.begin(); bb != basic_blocks.end(); bb++){
					if(mark[(*bb)->getID()]){
						if((*bb)->RegisterInterval() == i){
							auto s = (*bb)->get_output_list();
							for (auto it = s.begin(); it != s.end(); it++){
								Union.insert(*it);
							}
						}
					}
				}
		
				//This loop determines the boolean value of C
				auto P = (*b)->get_predecessors();
                                for(auto p = P.begin(); p != P.end(); p++){
                                        if((*p)->RegisterInterval() != i)
                                                C = false;
                                }

				//If this basic block is eligible, it will be added to the current register-interval
				if ((*b)->RegisterInterval() == nullptr //the basic block should not belong to another register-interval
                                        && C  //All predecessors of the basic block should belong to the current register-interval
                                        && Union.size() < N //The union of all registers of the basic block and current register-interval should not be more than the allowed number
                                        )
                                {
					(*b)->set_registerInterval(i);
					(*b)->set_input_list(Union);
					traverse((*b), N);
				}
			}
		}
                //At this point no more basic blocks can be added to the current register-interval

                set<basicBlock*> Successors; //Successors will contain all successors of the current register-interval (i)
		Successors.clear();
                
                //This loop will add all successors of the current registerInterval to Successors set
		for (auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
			if ((*b)->RegisterInterval() == i){
				auto S = (*b)->get_successors();
				for (auto s = S.begin(); s != S.end(); s++){
					if ((*s)->RegisterInterval() != i)
						Successors.insert(*s);
				}
			}
		}
                
                //This loop will assign new register-intervals to the successors of the current register-interval
		set<basicBlock*>::iterator it2;
		for (auto s = Successors.begin(); s != Successors.end(); s++){
			if ((*s)->RegisterInterval() == nullptr){
				auto temp = new registerInterval();
				RegisterIntervals.push_back(temp);
				(*s)->set_registerInterval(temp);
				(*s)->clear_input_list();
				WS.push(*s);
			}
		}
	}
        
        /*At this point all register-interval are created
         *      and the rest of the function just sets the register list of register-intervals
         *      and forms the new CFG that its nodes are register-intervals*/

        //This loop will set the register list of all register-intervals
	for (auto i = RegisterIntervals.begin(); i != RegisterIntervals.end(); i++){
                set<string> regList;//the register list of register-interval
		regList.clear();
		for (auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
			if ((*b)->RegisterInterval() == *i){
				auto s = (*b)->get_output_list();
				for (auto it = s.begin(); it != s.end(); it++){
					regList.insert(*it);
				}
			}
		}
		(*i)->set_register_list(regList);
	}

	//This loop sets the series of instructions of all register-intervals
	string code;
	for (auto i = RegisterIntervals.begin(); i != RegisterIntervals.end(); i++){
		code = "";
		for (auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
			if ((*b)->RegisterInterval() == *i){
				code = code + (*b)->get_code();
			}
		}
		(*i)->set_code(code);
	}

	//This loop sets the predecessors and successors of all register-intervals
	for (auto i = RegisterIntervals.begin(); i != RegisterIntervals.end(); i++){
		for (auto b = basic_blocks.begin(); b != basic_blocks.end(); b++){
			if ((*b)->RegisterInterval() == *i){
				auto P = (*b)->get_predecessors();
				for (auto p = P.begin(); p != P.end(); p++){
					if ((*p)->RegisterInterval() != *i)
						(*i)->add_predecessor((*p)->RegisterInterval());
				}
                                
                                auto S = (*b)->get_successors();
				for (auto s = S.begin(); s != S.end(); s++){
					if ((*s)->RegisterInterval() != *i)
						(*i)->add_successors((*s)->RegisterInterval());
				}
			}
		}
	}
        
        return RegisterIntervals;
}

vector<registerInterval*> RegisterIntervalCreationPass::registerIntervalCreationPassTwo(vector<registerInterval*> RegisterInterval_in,int N){
	vector<registerInterval*> RegisterInterval_out;//The registerInterval set that contains the reduced register-intervals

        /*Initialization*/
	for (auto i = RegisterInterval_in.begin(); i != RegisterInterval_in.end(); i++){
		(*i)->set_next_level_registerInterval(nullptr);
	}

	queue<registerInterval*> WorkingSet; //The working set of registerInterval creation algorithm (pass 2)

        
	for(auto i = RegisterInterval_in.begin(); i != RegisterInterval_in.end(); i++){
		auto Predecessors = (*i)->get_predecessors();
		if(Predecessors.size() == 0){// each register-interval with no predecessors is assumed as an entry registerInterval
			auto temp = new registerInterval();
			RegisterInterval_out.push_back(temp);
                        (*i)->set_next_level_registerInterval(temp);
			WorkingSet.push(*i);//The register-interval is added to working set of the algorithm
		}
	}

	while (!WorkingSet.empty()){
		auto i = WorkingSet.front();//Get a register-interval form the working set
		WorkingSet.pop();
		auto ii = i->nextLevelInterval();//ii is the current next-level register-interval that i belongs to 
		if (i->get_register_list().size() < N){//Just register-intervals will be processed that contain less registers than the allowed number of registers
			ii->set_register_list(i->get_register_list());//At the beginning the register list of the current next-level register registerInterval is equal to i
                        
                        /*This loop looks at all register-interval as a candidate for adding to the current register-interval
                                                            and checks the required conditions*/
			for (auto Interval = RegisterInterval_in.begin(); Interval != RegisterInterval_in.end(); Interval++){
				auto Predecessors = (*Interval)->get_predecessors();
				bool C = true;//This boolean value will determine that all predecessors of the candidate register-interval belong to the current next-level register-interval
                                set<string> Union;//This set will contain the union of register sets of all register-intervals that belong to the current next-level register-interval
                                Union.clear();

				visit_registerInterval_all_predecessors(*Interval);//All predecessors of the candidate register-interval in CFG will be marked

                                /*This loop will determine that all predecessors of the current register-interval belong to the current next-level register-interval*/
				for(auto p = Predecessors.begin(); p != Predecessors.end(); p++){
					if((*p)->nextLevelInterval() != ii)
						if(*p != *Interval)
							C = false;
				}

                                /*This loop will set the union set of registers of all register-intervals that the the candidate register-interval can be reached from
                                                                        and also belong to the current next-level register registerInterval*/
				for(auto iterator = RegisterInterval_in.begin(); iterator != RegisterInterval_in.end(); iterator++){
					auto s = (*iterator)->get_register_list();
					for (auto it = s.begin(); it != s.end(); it++){
						Union.insert(*it);
					}
				}

                                /*At this point we add the candidate register-interval to the current register-interval if all required conditions are satisfied*/
				if ((*Interval)->nextLevelInterval() == nullptr //the register-interval should belong to no other next-level register-intervals
                                        && C //the register-interval can be reached from the current next-level register-interval
                                        && Union.size() < N //the size of the union set should not be more than the allowed number of registers in each register-interval
                                        )
                                {
					(*Interval)->set_next_level_registerInterval(ii);
					auto s = (*Interval)->get_register_list();
					Union = ii->get_register_list();
					for (auto it = s.begin(); it != s.end(); ++it){
						Union.insert(*it);
					}
					ii->set_register_list(Union);
				}
			}
		}
                /*At this point no more register-intervals can be added to the current next-level register-interval*/
                
		set<registerInterval*> Successors;//This set will contain the successors the current next-level register-interval
		Successors.clear();
                /*This loop determines the members of Successors set*/
		for (auto Interval = RegisterInterval_in.begin(); Interval != RegisterInterval_in.end(); Interval++){
			if ((*Interval)->nextLevelInterval() == ii){
				auto S = (*Interval)->get_successors();
				for (auto s = S.begin(); s != S.end(); s++){
					Successors.insert(*s);
				}
			}
		}
                
                /*This loop will assign  new register-intervals to all successors of the current next-level registerInterval (as their next-level register registerIntervals)*/
		for (auto s = Successors.begin(); s != Successors.end(); s++){
			if ((*s)->nextLevelInterval() == nullptr){
				auto temp = new registerInterval();
				RegisterInterval_out.push_back(temp);
				(*s)->set_next_level_registerInterval(temp);
				WorkingSet.push(*s);
			}
		}
	}
        /*At this point all register-intervals of the next level are created
                    and rest of the function sets the register lists and the forms the new CFG*/

	/*this loop sets the register list of all output register-intervals*/
	for (auto Interval = RegisterInterval_out.begin(); Interval != RegisterInterval_out.end(); Interval++){
		set<string> regList;
            	regList.clear();
		for (auto iterator = RegisterInterval_in.begin(); iterator != RegisterInterval_in.end(); iterator++){
			if ((*iterator)->nextLevelInterval() == *Interval){
				auto R = (*iterator)->get_register_list();
				for (auto it = R.begin(); it != R.end(); it++){
					regList.insert(*it);
				}
			}
		}
		(*Interval)->set_register_list(regList);
	}

	/*this loop sets the series of instructions in all output register-intervals*/
	string code;
	for (auto Interval = RegisterInterval_out.begin(); Interval != RegisterInterval_out.end(); Interval++){
		code = "";
		for (auto iterator = RegisterInterval_in.begin(); iterator != RegisterInterval_in.end(); iterator++){
			if ((*iterator)->nextLevelInterval() == *Interval){
				code = code + (*iterator)->get_code();
			}
		}
		(*Interval)->set_code(code);
	}

	/*this loop sets the predecessors and successors of all output register-intervals*/
	for (auto Interval = RegisterInterval_out.begin(); Interval != RegisterInterval_out.end(); Interval++){
		for (auto iterator = RegisterInterval_in.begin(); iterator != RegisterInterval_in.end(); iterator++){
			if ((*iterator)->nextLevelInterval() == *Interval){
				auto Predecessors = (*iterator)->get_predecessors();
                                for (auto p = Predecessors.begin(); p != Predecessors.end(); p++){
					if ((*p)->nextLevelInterval() != *Interval)
						(*Interval)->add_successors((*p)->nextLevelInterval());
				}
                                
				auto Successors = (*iterator)->get_successors();
				for (auto s = Successors.begin(); s != Successors.end(); s++){
					if ((*s)->nextLevelInterval() != *Interval){
						(*Interval)->add_predecessor((*s)->nextLevelInterval());
					}
				}
			}
		}
	}

	return RegisterInterval_out;
}

void RegisterIntervalCreationPass::traverse(basicBlock* BB, int N){
	set<string> temp_list;
	vector<basicBlock*> V;
	auto register_list = BB->get_input_list();

	basicBlock* BB1;
	registerInterval* temp;
        string new_name, old_name;
        
        //set<string> register_list;//register_list is a list that will be updated after processing each instruction and the new registers will be added to it
        //register_list.clear();
        set<string> temporary_register_list;//temporary_register_list contains is like register_list but it can have more than N registers and it is checked at each iteration
        temporary_register_list.clear();
        
        string code = BB->get_code();//code is the string of instruction sequence
        
	int found1 = 0; //points to the beginning of the instruction string
	int found2 = code.find(";", found1, 1); //points to the end of instructions string
	while (found2 != string::npos){//This loop continues until all instructions of the basic block are processed
		
                /* this loop updates register_list for the current instruction (in other words it finds the register operands of the instruction)*/
		for (int i = found1; i < found2; i++){
			if (code[i] == 'R'){
				string str = "";
				if (code[i + 1]>47 && code[i + 1] < 58){
					str.push_back('R');
					str.push_back(code[i + 1]);
					if (code[i + 2]>47 && code[i + 2] < 58){
						str.push_back(code[i + 2]);
					}
					temporary_register_list.insert(str);
				}
			}
		}

		/*after cheching the size of temporary_register_list, we decide to split the basic block or not */
		if (temporary_register_list.size() > N){
                        /*At this point the basic block will be splited and the current instruction will reside in the second baisc basicBlock*/
			string code1, code2;//code1 and code2 contain the instructions of two basic blocks after splitting
                        code1.append(code, 0, found1); 
			code2.append(code, found1 + 1, code.size() - found1);


			//introducing a new basic block BB1:
			BB1 = new basicBlock();
			basic_blocks.push_back(BB1);
			
			old_name = BB->get_name();
			string new_name = old_name + std::to_string(BB->getID() + 1);
			BB1->set_name(new_name);//sets the name of the new basic blcok
			
                        int found = 0;
			found = code2.find(old_name, found+1);
			while(found != string::npos){//in thils loop finds points of code that uses the old name and replaces with the new name
				code2.replace(found, old_name.size(), new_name);
				
				found = code2.find(old_name, found+1);
			}
			BB1->set_code(code2);

			/*At this point the register set of the new basic block will be set*/
			found = 0;
			found = code2.find("R", found + 1, 1);
			while (found != string::npos){
				string str = "R";
				if (code2[found + 1] > 47 && code2[found + 1] < 58){
					for (int j = found + 1; code2[j] >47 && code2[j] < 58; j++){
						str = str + code2[j];
					}
					BB1->add_register(str);
				}

				found = code2.find("R", found + 1, 1);
			}
                        
                        //The BB will be splitted and its code and register list are updated
			BB->set_code(code1);
			BB->set_output_list(register_list);

			/*setting successors of old and new basic blocks*/
			BB1->set_successors(BB->get_successors()); //the successors of the new basic block is just like the original basic block
                        vector<basicBlock*> S;
                        S.clear();
                        S.push_back(BB1);
                        BB->set_successors(S); //the only successor of the old basic block is the new one

			/*setting predecessors of new and old basic blocks*/
			BB1->add_predecessor(BB); //the only predecessor of the new basic block is the old one (also the predecessors of the old basic block won't change)
			

			auto temp = new registerInterval();
			RegisterIntervals.push_back(temp);
			BB1->set_registerInterval(temp);
			BB1->clear_input_list();
			WS.push(BB1);//the new basic block has been assigned to a new register-interval

			return;
		}
		else{
			register_list = temporary_register_list;
			BB->set_output_list(register_list);
		}

		register_list = temporary_register_list;
		found1 = found2 + 1;
		found2 = code.find(";", found1, 1);
	}

	return;
}

void RegisterIntervalCreationPass::visit_all_predecessors(basicBlock* BB){
	for(int i=0; i < basic_blocks.size(); i++)
		mark.push_back(false);

	for(int i=0;i<basic_blocks.size(); i++){
		mark[i] = false;
	}

	visit_predecessors(BB);
}

void RegisterIntervalCreationPass::visit_predecessors(basicBlock* BB){
	mark[BB->getID()] = true;
	auto P = BB->get_predecessors();
	for(int i=0;i<P.size();i++){
		if(!mark[P[i]->getID()])
			visit_predecessors(P[i]);
	}
}

void RegisterIntervalCreationPass::visit_registerInterval_all_predecessors(registerInterval* in){
	for(int j=0; j < RegisterIntervals[0]->getID() + RegisterIntervals.size(); j++)
		registerInterval_mark.push_back(false);

	for(int i=0; i < RegisterIntervals[0]->getID() + RegisterIntervals.size(); i++){
		registerInterval_mark[i] = false;
	}

	registerInterval_visit_predecessors(in);
}

void RegisterIntervalCreationPass::registerInterval_visit_predecessors(registerInterval* in){
	registerInterval_mark[in->getID()] = true;
	auto P = in->get_predecessors();
	for(int i=0;i<P.size();i++){
		if(!registerInterval_mark[P[i]->getID()])
			registerInterval_visit_predecessors(P[i]);
	}
}
