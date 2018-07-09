/*
 * \file    ControlFlowGraph.cpp
 * 
 * \authors Ali Hajiabadi <hajiabadi@ce.sharif.edu>
 *          Mohammad Sadrosadati
 *          Amirhossein Mirhosseini
 *
 * \brief   Implementation of Control Flow Graph (CFG)
 * 
 * Note: Register-interval creation algorithm is used in paper "LTRF: Enabling High-Capacity Register Files for GPUs via Hardware/Software Cooperative Register Prefetching" accepted in ASPLOS’18.
 */

#include "../interface/ControlFlowGraph.h"

using namespace std;

//******************************************registerInterval Methods**************************************
int registerInterval::IDgenerator = 0;

registerInterval::registerInterval(){
	ID = IDgenerator++;
	registerInterval_numberOfInstructions = 0;
}

registerInterval::~registerInterval(){}

int registerInterval::getID(){
	return ID;
}

int registerInterval::getNumberOfInstructions(){
	return registerInterval_numberOfInstructions;
}

void registerInterval::set_code(string s){
	code = s;
	registerInterval_numberOfInstructions = 0;

	int found = 0;
	found = code.find(";", found + 1, 1);
	while (found != string::npos){
		registerInterval_numberOfInstructions++;
		found = code.find(";", found + 1, 1);
	}
}

string registerInterval::get_code(){
	return code;
}

set<string> registerInterval::get_register_list(){
	return register_list;
}

vector<bool> registerInterval::get_register_vector(){
	return register_vector;
}

void registerInterval::set_register_list(set<string> s){
	register_list = s;

	int pos;
	string str;
	set<string>::iterator it;
	for(int i = 0; i < 64; i++){
		register_vector.push_back(false);
	}

	for(it = register_list.begin(); it != register_list.end(); ++it){
		str = *it;
		str.erase(str.begin());
		pos = stoi(str);
		register_vector[pos] = true;
	}
}

void registerInterval::add_predecessor(registerInterval* i){
	bool found = false;
	for (int j = 0; j < predecessors.size(); j++){
		if (predecessors[j] == i)
			found = true;
	}

	if (!found)
		predecessors.push_back(i);
}

void registerInterval::add_successors(registerInterval* i){
	bool found = false;
	for (int j = 0; j < successors.size(); j++){
		if (successors[j] == i)
			found = true;
	}

	if (!found)
		successors.push_back(i);
}

vector<registerInterval*> registerInterval::get_predecessors(){
	return predecessors;
}

vector<registerInterval*> registerInterval::get_successors(){
	return successors;
}

void registerInterval::set_next_level_registerInterval(registerInterval* i){
	next_level_registerInterval = i;
}

registerInterval* registerInterval::nextLevelInterval(){
	return next_level_registerInterval;
}

//*******************************************basicBlock Methods****************************************
int basicBlock::IDgenerator = 0;
int basicBlock::IDgenerator2 = 0;

basicBlock::basicBlock(){
	ID = IDgenerator++;
	NumberOfInstructions = 0;
	controlBB = false;
	controling_basicBlock = nullptr;
}

basicBlock::~basicBlock(){}

basicBlock::basicBlock(string s){
	ID = IDgenerator++;
	name = s;
	NumberOfInstructions = 0;
	controlBB = false;
	controling_basicBlock = nullptr;
}

basicBlock::basicBlock(int i){
	ID = IDgenerator2++;
	ABB = i;
	NumberOfInstructions = 0;
	controlBB = false;
	exit_state = false;
	controling_basicBlock = nullptr;
}

void basicBlock::set_name(string s)
{
	name = s;
}

void basicBlock::set_code(string s)
{
	code = s;
	NumberOfInstructions = 0;

	int found = 0;
	found = code.find(";", found + 1, 1);
	while (found != string::npos){
		NumberOfInstructions++;
		found = code.find(";", found + 1, 1);
	}
}

int basicBlock::numberOfInstructions(){
	return NumberOfInstructions;
}

void basicBlock::add_register(string r)
{
	output_list.insert(r);
}

void basicBlock::add_predecessor(basicBlock* b)
{
	bool found = false;
	for (int i = 0; i < predecessors.size(); i++){
		if (predecessors[i] == b)
			found = true;
	}

	if (!found)
		predecessors.push_back(b);
}

void basicBlock::add_successor(basicBlock* b)
{
	bool found = false;
	for (int i = 0; i < successors.size(); i++){
		if (successors[i] == b)
			found = true;
	}

	if (!found)
		successors.push_back(b);
}

void basicBlock::set_successors(vector<basicBlock*> Set){
	successors = Set;
}

void basicBlock::delete_successor(basicBlock* b){
	vector<basicBlock*>::iterator it;
	for (it = successors.begin(); it != successors.end(); it++){
		if(*it == b){
			successors.erase(it);
			break;
		}
	}
}

string basicBlock::get_name()
{
	return name;
}

int basicBlock::getID(){
	return ID;
}

string basicBlock::get_code()
{
	return code;
}

int basicBlock::numberOfRegisters()
{
	return output_list.size();
}

set<string> basicBlock::get_output_list()
{
	return output_list;
}

set<string> basicBlock::get_input_list(){
	return input_list;
}

void basicBlock::set_output_list(set<string> s){
	output_list = s;
}

void basicBlock::set_input_list(set<string> s){
	input_list = s;
}

vector<basicBlock*> basicBlock::get_predecessors()
{
	return predecessors;
}

vector<basicBlock*> basicBlock::get_successors()
{
	return successors;
}

registerInterval* basicBlock::RegisterInterval(){
	return BB_registerInterval;
}

void basicBlock::clear_output_list(){
	output_list.clear();
}

void basicBlock::clear_input_list(){
	input_list.clear();
}

void basicBlock::set_registerInterval(registerInterval* i){
	BB_registerInterval = i;
}

void basicBlock::set_ABB(int x){
	ABB = x;
}

int basicBlock::get_ABB(){
	return ABB;
}

void basicBlock::set_controling(){
	controlBB = true;
}

bool basicBlock::is_controling(){
	return controlBB;
}

void basicBlock::set_control_basicBlock(basicBlock* b){
	controling_basicBlock = b;
}

basicBlock* basicBlock::get_controling_basicBlock(){
	return controling_basicBlock;
}

void basicBlock::set_exit_state(){
	exit_state = true;
}

bool basicBlock::is_exit_state(){
	return exit_state;
}
