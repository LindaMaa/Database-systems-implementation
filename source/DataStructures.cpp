#include "libs.h"
#include "TupleStorage.cpp"
#include "DataStructures.h"
using namespace std;

// function used in LOAD -> supports loading relations to the main datastructure
// provided relation name and file name, add relation
int DataStructures::addRelation(string relation_name, string file_name)
{
    clock_t start_time = clock();

    // create hashtable which stores (x,y) -> counts for relation with provided filename
    unordered_map<tuple<int, int>, int, hash_tuple> relation_table;
    int number_of_tuples = 0;

    string filepath = "./data/" + file_name;

    string line;
    ifstream file(filepath);

    if (file.is_open())
    {
        while (getline(file, line))
        {
            stringstream ss(line);
            int x, y;
            char delimiter;

            ss >> x;

            if (!ss || ss.peek() != ',')
            {
                clock_t end_time = clock();
                double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
                cout << "The input could not be parsed due to a syntax error." << endl;
                cout << "Loading interrupted: Number of tuples loaded into relation " << relation_name << " is " << number_of_tuples << endl;
                cout << "Time taken to load until interruption: " << time_taken << " ms" << endl;
                return 0;
            }

            ss.ignore();
            ss >> y;

            if (!ss || !ss.eof())
            {
                clock_t end_time = clock();
                double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
                cout << "The input could not be parsed due to a syntax error." << endl;
                cout << "Loading interrupted: Number of tuples loaded into relation " << relation_name << " is " << number_of_tuples << endl;
                cout << "Time taken to load until interruption: " << time_taken << " ms" << endl;
                return 0;
            }

            auto parsed_tuple = make_tuple(x, y);
            addTuple(relation_table, parsed_tuple);
            number_of_tuples++;
        }
        file.close();
        this->loaded_relations[relation_name] = relation_table;
        clock_t end_time = clock();
        cout << "Number of tuples loaded into relation " << relation_name << " is " << number_of_tuples << endl;
        double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
        cout << "Time taken to load: " << time_taken << " ms" << endl;
        return number_of_tuples;
    }
    else
    {
        cout << "Error: File could not be opened." << endl;
        cout << "Number of tuples loaded into relation " << relation_name << " is " << number_of_tuples << endl;
        clock_t end_time = clock();
        double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
        cout << "Time taken to load: " << time_taken << " ms" << endl;
        return 0;
    }
};

// return how many relations are stored in the datastructure
int DataStructures::getNumberOfLoadedRelations()
{
    int n = loaded_relations.size();
    cout << "Number of loaded relations: " << n << endl;
    return n;
}

// remove relation from the datastructure
int DataStructures::removeRelation(string relation_name)
{
    if (loaded_relations.count(relation_name) > 0)
    {
        loaded_relations.erase(relation_name);
        cout << "Successfully removed relation with name: " << relation_name << endl;
        return 1;
    }
    else
    {
        cout << "Could not remove relation with name: " << relation_name << endl;
        return 0;
    }
}

// get names of all tables (relations) stored in the datastructure
vector<string> DataStructures::getRelationNames()
{
    vector<string> names;
    for (const auto &key_value_pair : loaded_relations)
    {
        names.push_back(key_value_pair.first);
    }
    return names;
}

// retrieve relation with specified name
unordered_map<tuple<int, int>, int, hash_tuple> DataStructures::getRelation(string relation_name)
{
    if (loaded_relations.count(relation_name) > 0)
    {
        return loaded_relations[relation_name];
    }
    cout << "Could not find relation with name: " << relation_name << endl;
    return unordered_map<tuple<int, int>, int, hash_tuple>();
}

// prints single relation
void DataStructures::displayRelation(const unordered_map<tuple<int, int>, int, hash_tuple> &relation_table)
{
    for (const auto &kv : relation_table)
    {
        cout << "(" << get<0>(kv.first) << ", " << get<1>(kv.first) << "): " << kv.second << endl;
    }
}

// prints all loaded relations
void DataStructures::displayLoadedRelations(const unordered_map<string, unordered_map<tuple<int, int>, int, hash_tuple>> &loaded_relations)
{
    cout << "Printing all loaded relations:" << std::endl;
    for (const auto &relation : loaded_relations)
    {
        cout << "Relation: " << relation.first << endl;
        for (const auto &tuple_entry : relation.second)
        {
            cout << "\t";
            cout << "(" << get<0>(tuple_entry.first) << ", " << get<1>(tuple_entry.first) << "): " << tuple_entry.second << endl;
        }
    }
};
