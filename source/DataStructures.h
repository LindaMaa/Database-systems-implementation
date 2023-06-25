#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "libs.h"

using namespace std;

class DataStructures
{
public:
    unordered_map<string, unordered_map<tuple<int, int>, int, hash_tuple>> loaded_relations; // main datastructure for storing loaded relations

    int addRelation(string relation_name, string file_name);
    int getNumberOfLoadedRelations();
    int removeRelation(string relation_name);
    vector<string> getRelationNames();
    unordered_map<tuple<int, int>, int, hash_tuple> getRelation(string relation_name);
    void displayRelation(const unordered_map<tuple<int, int>, int, hash_tuple> &relation_table);
    void displayLoadedRelations(const unordered_map<string, unordered_map<tuple<int, int>, int, hash_tuple>> &loaded_relations);
};

#endif