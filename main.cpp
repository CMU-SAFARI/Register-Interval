/*
 * \file    main.cpp
 * \authors Ali Hajiabadi <hajiabadi@ce.sharif.edu>
 *          Mohammad Sadrosadati
 *          Amirhossein Mirhosseini
 *
 * \brief   The main function of register-interval creation algorithm
 * 
 * Note: Interval creation algorithm is used in paper "LTRF: Enabling High-Capacity Register Files for GPUs via Hardware/Software Cooperative Register Prefetching" accepted in ASPLOS’18.
 */

//Standard include
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>

//File includes
#include "src/implementation/RegisterIntervalCreation.cpp"

using namespace std;

/* \brief This function gets the input file that contains the Graphviz Dot code of CFG and generates the basic blocks of application*/
vector<basicBlock*> create_control_flow_graph(char* file_name);

/* \brief Splits abstract basic blocks to regular basic blocks*/
vector<basicBlock*> split_abstract_basic_blocks(vector<basicBlock*>);

/* \brief generates a log that contains all information about the basic blocks and CFG (file: "basicBlocks.txt") */
void basic_blocks_log(vector<basicBlock*> BBs, char* file_name);

/* \brief generates a log that contains all information about the register-intervals and register-interval creation algorithm (file: "register-intervals_#{iteration}.txt") */
void registerInterval_log(vector<registerInterval*> intervals, int iteration, char* file_name);

int main(int argc, char **argv)
{
	int REGNUM=atoi(argv[1]); //REGNUM is the allowed number of registers in register-intervals

        //At first the graphviz dot code (the output of nvidisasm tool) will be parsed and the CFG will be generated.
	vector<basicBlock*> basic_blocks = create_control_flow_graph(argv[2]);
        
        basic_blocks_log(basic_blocks, argv[2]);//generates a log file

        //Register-interval creation algorithm (pass one) will construct the initial register-intervals from basic blocks and CFG
	auto intervals = RegisterIntervalCreationPass::registerIntervalCreationPassOne(basic_blocks, REGNUM);

        //In the following loop, pass two of register-interval creation algorithm will be repeated until no changes occur
	int Size, i = 0;
	do
	{
		registerInterval_log(intervals, i++, argv[2]);

		Size = intervals.size();
		intervals = RegisterIntervalCreationPass::registerIntervalCreationPassTwo(intervals, REGNUM);
	} while (intervals.size() < Size); //Pass 2 of register-interval creation algorithm will be done until the CFG can not be reduced anymore

	return 0;
}

vector<basicBlock*> create_control_flow_graph(char* file_name){
    /* The control flow analysis of nvdisasm tool generates a CFG that its nodes are abstract basic blocks. So
     we first extract the abstract basic blocks from graphviz dot code (the output of nvdisasm tool) and then  we will create basic blocks and CFG.*/
    
    vector<basicBlock*> abstract_basic_blocks;
    
    //**********************getting input file and finding abstract basic blocks*******************************
        fstream file (file_name, fstream::in);
        
	char ch;
	string cfgString;
	string str = "";
	bool start = false;
	bool ready = false;
	bool OK = false;
	while (file >> noskipws >> ch)
	{
		cfgString = cfgString + ch;
	}

	for (int i = 0; i < cfgString.size(); i++){
		if (start){
			str = str + cfgString[i];
		}

		if (cfgString[i] == '"'){
			if (!start){
				start = true;
			}
			else{
				ready = true;
				start = false;
			}
		}

		if (ready){
			if (cfgString[i + 1] == '\n' || cfgString[i + 1] == 13){
				OK = true;
				ready = false;
			}
			else{
				ready = false;
				str = "";
			}
		}

		if (OK){
			str.erase(str.size() - 1, 1);
			auto temp = new basicBlock(str);
			abstract_basic_blocks.push_back(temp);
			OK = false;
			str = "";
		}
	}
	//******************************************************************************************

	//***************************setting code of abstract basic blocks*************************************
	int found1 = 0;
	int found2 = 0;
	string code;
	for (int i = 0; i < abstract_basic_blocks.size(); i++){
		code = "";
		found1 = cfgString.find("<entry>", found1 + 1, 7);
		found2 = cfgString.find("}\"]", found2 + 1, 3);
		for (int j = found1; j < found2 - 2; j++){
			code = code + cfgString[j];
		}
		abstract_basic_blocks[i]->set_code(code);
	}
	//******************************************************************************************

	//********************************Setting successors of abstract basic blocks******************************
	size_t found = 0;
	string tmp;
	int count = 0;
	for (int i = 0; i < abstract_basic_blocks.size(); i++){
		str = "\"";
		str = str + abstract_basic_blocks[i]->get_name();
		str = str + "\"";
		str = str + ":exit";
		found = 0;
		found = cfgString.find(str, found + 1);
		while (found != string::npos){
			count = 0;
			tmp = "";
			for (int j = found + 1; count < 3; j++){
				if (count == 2){
					tmp = tmp + cfgString[j];
				}

				if (cfgString[j] == '"')
					count++;
			}
			tmp.erase(tmp.end() - 1);
			for (int j = 0; j < abstract_basic_blocks.size(); j++){
				if (abstract_basic_blocks[j]->get_name() == tmp){
					abstract_basic_blocks[i]->add_successor(abstract_basic_blocks[j]);
					break;
				}
			}

			found = cfgString.find(str, found + 1);
		}
	}
	
	count = 0;
	for (int i = 0; i < abstract_basic_blocks.size(); i++){
		str = "\"";
		str = str + abstract_basic_blocks[i]->get_name();
		str = str + "\"";
		str = str + ":entry:s";
		found = 0;
		found = cfgString.find(str, found + 1);
		while (found != string::npos){
			count = 0;
			tmp = "";
			for (int j = found + 1; count < 3; j++){
				if (count == 2){
					tmp = tmp + cfgString[j];
				}

				if (cfgString[j] == '"')
					count++;
			}
			tmp.erase(tmp.end() - 1);
			for (int j = 0; j < abstract_basic_blocks.size(); j++){
				if (abstract_basic_blocks[j]->get_name() == tmp){
					abstract_basic_blocks[i]->add_successor(abstract_basic_blocks[j]);
					break;
				}
			}

			found = cfgString.find(str, found + 1);
		}
	}
	//************************************************************************************************

	//********************************Setting predecessors of abstract basic blocks**********************************
	vector<basicBlock*> x;
	for (int i = 0; i < abstract_basic_blocks.size(); i++){
		for (int j = 0; j < abstract_basic_blocks.size(); j++){
			x = abstract_basic_blocks[j]->get_successors();
			for (int k = 0; k < x.size(); k++){
				if (x[k]->get_name() == abstract_basic_blocks[i]->get_name()){
					abstract_basic_blocks[i]->add_predecessor(abstract_basic_blocks[j]);
				}
			}
		}
	}
	//************************************************************************************************
	
	//**************************Splitting abstract basic blocks to regular basic blocks***********************************
	auto basic_blocks = split_abstract_basic_blocks(abstract_basic_blocks);
	
	//***********************Extracting Registers of each basicBlock***************************************
	found = 0;
	for (int i = 0; i < basic_blocks.size(); i++){
		found = 0;
		code = basic_blocks[i]->get_code();
		found = code.find("R", found + 1, 1);
		while (found != string::npos){
			str = "R";
			if (code[found + 1] > 47 && code[found + 1] < 58){
				for (int j = found + 1; code[j] >47 && code[j] < 58; j++){
					str = str + code[j];
				}
				basic_blocks[i]->add_register(str);
			}

			found = code.find("R", found + 1, 1);
		}
	}
	//************************************************************************************************
        
        return basic_blocks;
}

vector<basicBlock*> split_abstract_basic_blocks(vector<basicBlock*> ABBs){
	basicBlock *temp, *temp2;
	vector<basicBlock*> controling_basicBlocks;
	string q = "Controling_basicBlock";
	int p = 0;
        
        vector<basicBlock*> basic_blocks;

	//**********************Splitting Codes and Constructing new BBs***********************
	string str, str2;
	int found1 = 0;
	int found2 = 0;
	int foundtemp = 0;
	int foundExit = 0;
	for(int i = 0; i < ABBs.size(); i++){
		str = ABBs[i]->get_code();
		char tt = 'A';
		found1 = 0;
		found2 = str.find("<exit", found1);
		
		while(found2 != string::npos){
			str2.clear();
			for(int j = found1; j < found2; j++){
				str2 = str2 + str[j];
			}

			foundExit = str2.find("EXIT", 0);
			foundtemp = str2.find(";", 0);

			if(foundExit == string::npos){
				if(str2.find(";", foundtemp + 1) == string::npos && str2.find("@P0", 0) != string::npos){
					temp2 = new basicBlock();
					temp2->set_code(str2);
					temp2->set_name(q + std::to_string(p));
					p++;
					temp2->set_controling();
					controling_basicBlocks.push_back(temp2);

					temp->set_control_basicBlock(temp2);
				}
				else{
					temp = new basicBlock(ABBs[i]->getID());
					temp->set_code(str2);
					if(found1 == 0)
						temp->set_name(ABBs[i]->get_name());
					else{
						temp->set_name(ABBs[i]->get_name()+tt);
						tt++;
					}
					basic_blocks.push_back(temp);
				}
			}

			found1 = found2+1;
			found2 = str.find("<exit", found1);
		}

		str2.clear();
		for(int j = found1; j < str.size(); j++){
			str2 = str2 + str[j];
		}

		foundExit = str2.find("EXIT", 0);
		foundtemp = str2.find(";", 0);
		if(foundExit == string::npos){
			if(str2.find(";", foundtemp + 1) == string::npos && str2.find("@P0", 0) != string::npos){
				temp2 = new basicBlock();
				temp2->set_code(str2);
				temp2->set_name(q + std::to_string(p));
				p++;
				temp2->set_controling();
				controling_basicBlocks.push_back(temp2);

				temp->set_control_basicBlock(temp2);
			}
			else{
				temp = new basicBlock(ABBs[i]->getID());
				temp->set_code(str2);
				if(found1 == 0)
					temp->set_name(ABBs[i]->get_name());
				else{
					temp->set_name(ABBs[i]->get_name()+tt);
					tt++;
				}
				basic_blocks.push_back(temp);
			}
		}
	}
	//***********************************************************************************

	//*********************Setting Predecessors and Successors***************************
	vector<basicBlock*> P, S;
	bool isNextBB;
	int firstID;
	int lastID;
	string name;
	for(int i = 0; i < ABBs.size(); i++){
		P = ABBs[i]->get_predecessors();
		S = ABBs[i]->get_successors();

		int count = 0;
		for(int k = 0; k < basic_blocks.size(); k++){
			if(basic_blocks[k]->get_ABB() == ABBs[i]->getID()){
				if(count == 0)
					firstID = k;
				count++;
			}
		}
		lastID = firstID + count - 1;


		for(int j = 0; j < S.size(); j++){
			if(S[j]->getID() == ABBs[i]->getID()+1){
				basic_blocks[lastID]->add_successor(basic_blocks[lastID+1]);
				basic_blocks[lastID+1]->add_predecessor(basic_blocks[lastID]);
			}
		}

		for(int j = firstID; j < lastID; j++){
			basic_blocks[j]->add_successor(basic_blocks[j+1]);
			basic_blocks[j+1]->add_predecessor(basic_blocks[j]);
		}

		for(int j = firstID; j <= lastID; j++){
			if(basic_blocks[j]->get_controling_basicBlock() != nullptr){
				str = (basic_blocks[j]->get_controling_basicBlock())->get_code();
				for(int k = 0; k < basic_blocks.size(); k++){
					name = basic_blocks[k]->get_name();
					if(str.find(name, 0) != string::npos && name.size() != 0){
						basic_blocks[j]->add_successor(basic_blocks[k]);
						basic_blocks[k]->add_predecessor(basic_blocks[j]);
					}
				}
			}
		}
	}
        
        return basic_blocks;
}

void basic_blocks_log(vector<basicBlock*> basic_blocks, char* file_name){
	std::string File(file_name);
	int p = File.find("/");
	int p1 = p;
	while(p != string::npos){
		p1 = p;
		p = File.find("/", p + 1);
	}
	int p2 = File.find(".");
	File = File.substr(p1 + 1, p2 - p1 - 1);
        std::ofstream out("output/BasicBlocks_" + File + ".txt");
        
        vector<basicBlock*> P ,S;
	out << "The number of basic_blocks = " << basic_blocks.size() << endl;
	for (int i = 0; i < basic_blocks.size(); i++){
		out << " #The basic_block ID : " << basic_blocks[i]->getID() << endl;
		out << "		#Number of Instructions = " << basic_blocks[i]->numberOfInstructions() << endl;
		out << "		#Register_list = {";
		set<string> L = basic_blocks[i]->get_output_list();
		set<string>::iterator it;
		for (it = L.begin(); it != L.end(); it++)
			out << *it << " ";
		out << "}; #Number of Registers = " << L.size() << ";" << endl;
		P = basic_blocks[i]->get_predecessors();
		S = basic_blocks[i]->get_successors();
		out << "		#predecessors = {";
		for(int j=0;j<P.size();j++){
			out << P[j]->getID() << " ";
		}
		out << "};		#successors = {";
		for(int j=0;j<S.size();j++){
			out << S[j]->getID() << " ";
		}
		out <<"};" << endl;
	}
}

void registerInterval_log(vector<registerInterval*> intervals, int iteration, char* file_name){
	std::string File(file_name);
	int p = File.find("/");
	int p1 = p;
	while(p != string::npos){
		p1 = p;
		p = File.find("/", p + 1);
	}
	int p2 = File.find(".");
	File = File.substr(p1 + 1, p2 - p1 - 1);
        std::ofstream out("output/registerIntervals_" + File + ".txt");
        
	out << "The number of Intervals = " << intervals.size() << endl;
	for (int i = 0; i < intervals.size(); i++){
		out << " #Interval ID = " << intervals[i]->getID() << " #Number of Instructions = " << intervals[i]->getNumberOfInstructions() << endl;
		out << "		 #Register_list = {";
		set<string> L = intervals[i]->get_register_list();
		set<string>::iterator it;
		for (it = L.begin(); it != L.end(); it++)
			out << *it << " ";
		out << "}; #Number of Registers = " << L.size() << ";" << endl;
			
		vector<registerInterval*> ii;

		ii = intervals[i]->get_predecessors();
		out <<"		Predecessors = { ";
		for(int j = 0; j < ii.size(); j++){
			out << ii[j]->getID() << " ";
		}

		ii = intervals[i]->get_successors();
		out << "}		Successors = { ";
		for(int j = 0; j < ii.size(); j++){
			out << ii[j]->getID() << " ";
		}
		out << "};" << endl;
	}
}
