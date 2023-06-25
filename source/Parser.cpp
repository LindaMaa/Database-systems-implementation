#ifndef __PARSER__
#define __PARSER__

#include "libs.h"
#include "Constraints.cpp"
#include "TupleStorage.h"
#include "DataStructures.h"

using namespace std;

class Parser
{

public:
    int query_relations = 0;
    Constraints *constrs;                                         // constraints parsed from query
    map<string, set<int>> *variableMap;                           // stores variable -> domain map
    unordered_set<string> variables_set;                          // stores variables parsed from query
    map<string, set<vector<int>>> *relationsToTuples;             // relation name -> set of tuples in relation
    map<string, set<vector<string>>> *relationsToVariablePairs;   // relation name -> set of pairs of variable names
    unordered_map<string, vector<int>> *variablesToRelationIndex; // stores map from variable keys (var1,var2) -> table index
    unordered_map<int, int> loaded_relations_totals;              // stores total number of tuples in each relation

    // cached relations
    vector<unordered_map<string, int>> *hashMaps;
    vector<unordered_map<tuple<int, int>, int, hash_tuple>> *hashMapsTuple;

    // inverted indices
    vector<unordered_map<int, vector<tuple<int, int>>>> invertedIndicesLeft;
    vector<unordered_map<int, vector<tuple<int, int>>>> invertedIndicesRight;

    Parser(Constraints *c, vector<unordered_map<string, int>> *hmps, vector<unordered_map<tuple<int, int>, int, hash_tuple>> *ht, map<string, set<int>> *vm, map<string, set<vector<int>>> *relTup,
           map<string, set<vector<string>>> *varPairs, unordered_map<string, vector<int>> *v)
    {
        constrs = c;
        hashMaps = hmps;
        variableMap = vm;
        relationsToTuples = relTup;
        relationsToVariablePairs = varPairs;
        variablesToRelationIndex = v;
        hashMapsTuple = ht;
    }

    // reads query from a txt file
    int readQuery(string filename, DataStructures &ds)
    {
        string filepath = "./queries/" + filename;
        string line;
        ifstream file(filepath);
        if (file.is_open())
        {
            while (getline(file, line))
            {
                cout << "Original query" << endl;
                cout << line << endl;
                int res = this->parseQuery(line, ds);
                return res;
            }
            file.close();
        }
        else
        {
            cout << "Error opening file. File with this query name not found." << endl;
            return 0;
        }
        return 1;
    }

    // Parses query, stores variables, and prepares required relations
    int parseQuery(string query, DataStructures &ds)
    {
        cout << "Parsing query..." << endl;
        int result;

        this->query_relations = count_relations(query);
        invertedIndicesLeft.resize(query_relations);
        invertedIndicesRight.resize(query_relations);

        // Updated regex pattern to validate the entire query and ensure it does not end with a comma
        regex full_query_pattern(R"(\s*\w+\((\d+|[a-zA-Z]\w*),\s*(\d+|[a-zA-Z]\w*)\)(?:\s*,\s*\w+\((\d+|[a-zA-Z]\w*),\s*(\d+|[a-zA-Z]\w*)\))*\s*)");
        smatch matches; // stores the regex matches

        // Check if the entire query matches the required format
        if (!regex_match(query, matches, full_query_pattern))
        {
            cout << "Error: Invalid query format. Please follow strictly this format: R1(var1,var2), R2(var1,var2), R3(var1,var2), R4(var1,var2)" << endl;
            return 0;
        }

        // Regex pattern to extract individual relations and their arguments
        regex relation_pattern(R"((\w+)\((\d+|[a-zA-Z]\w*),\s*(\d+|[a-zA-Z]\w*)\))");

        // loop through the query and extract each relation and its arguments
        while (regex_search(query, matches, relation_pattern))
        {
            string relation_name = matches[1].str();
            string arg1 = matches[2].str();
            string arg2 = matches[3].str();

            result = this->createTable(relation_name, arg1, arg2, ds);

            if (result == 0)
            {
                return 0;
            }

            //  move to the next relation
            query = matches.suffix().str();
        }

        return result;
    }

    int createTable(string relation_name, string variable1, string variable2, DataStructures &DS)

    {
        regex relation_name_format(R"(\w+)");
        regex variable_format(R"(\w+)");

        if (!regex_match(relation_name, relation_name_format) || !regex_match(variable1, variable_format) || !regex_match(variable2, variable_format))
        {
            cout << "Invalid relation name or variables format. Please use the correct format." << endl;
            return 0;
        }

        vector<string> relation_names_list = DS.getRelationNames();
        auto it = find(relation_names_list.begin(), relation_names_list.end(), relation_name);
        if (it == relation_names_list.end())
        {
            cout << "The relation table with name: " << relation_name << " is not loaded. Terminate counting." << endl;
            return 0;
        }
        map<string, set<int>> temp;
        unordered_map<string, int> *HashMap = new unordered_map<string, int>();
        unordered_map<tuple<int, int>, int, hash_tuple> *HashMapTuple = new unordered_map<tuple<int, int>, int, hash_tuple>();

        int number_of_tuples = 0;
        unordered_map<tuple<int, int>, int, hash_tuple> loaded_relation = DS.getRelation(relation_name);

        if (loaded_relation.empty())
        {
            cout << "The relation table with name: " << relation_name << " is not loaded. Terminate counting." << endl;
            return 0;
        }

        // filter both variables ints
        if (isInteger(variable1) && isInteger(variable2))
        {

            for (const auto &key_value_pair : loaded_relation)
            {
                int x = get<0>(key_value_pair.first);
                int y = get<1>(key_value_pair.first);
                int multi = key_value_pair.second;
                if (x == stoi(variable1) && y == stoi(variable2))
                {

                    (*HashMap)["(" + to_string(x) + "," + to_string(y) + ")"] += multi;
                    (*HashMapTuple)[make_tuple(x, y)] += multi;
                    number_of_tuples += multi;

                    int currentIndex = hashMaps->size();

                    invertedIndicesLeft[currentIndex].emplace(x, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));
                    invertedIndicesRight[currentIndex].emplace(y, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));

                    vector<int> s = {x, y};
                    ((*relationsToTuples)[relation_name]).insert(s);
                }
            }

            /// cout << "Number of tuples after filtering from table " << relation_name << " is " << number_of_tuples << endl;
        }

        // left int, right variable
        else if (isInteger(variable1))
        {

            for (const auto &key_value_pair : loaded_relation)
            {
                int x = get<0>(key_value_pair.first);
                int y = get<1>(key_value_pair.first);
                int multi = key_value_pair.second;
                if (x == stoi(variable1))
                {
                    if ((*variableMap)[variable2].size() > 0)
                    {
                        if ((*variableMap)[variable2].count(y) == 0)
                        {
                            continue;
                        }
                    }

                    (*HashMap)["(" + to_string(x) + "," + to_string(y) + ")"] += multi;
                    (*HashMapTuple)[make_tuple(x, y)] += multi;

                    int currentIndex = hashMaps->size();

                    invertedIndicesLeft[currentIndex].emplace(x, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));
                    invertedIndicesRight[currentIndex].emplace(y, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));

                    number_of_tuples += multi;

                    temp[variable2].insert(y);

                    vector<int> s = {x, y};
                    ((*relationsToTuples)[relation_name]).insert(s);
                }
            }

            // cout << "Number of tuples after filtering from table " << relation_name << " is " << number_of_tuples << endl;
        }
        else if (isInteger(variable2))
        {

            for (const auto &key_value_pair : loaded_relation)
            {
                int x = get<0>(key_value_pair.first);
                int y = get<1>(key_value_pair.first);
                int multi = key_value_pair.second;

                if (y == stoi(variable2))
                {

                    if ((*variableMap)[variable1].size() > 0)
                    {
                        if ((*variableMap)[variable1].count(x) == 0)
                            continue;
                    }

                    (*HashMap)["(" + to_string(x) + "," + to_string(y) + ")"] += multi;
                    (*HashMapTuple)[make_tuple(x, y)] += multi;

                    int currentIndex = hashMaps->size();

                    invertedIndicesLeft[currentIndex].emplace(x, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));
                    invertedIndicesRight[currentIndex].emplace(y, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));

                    number_of_tuples += multi;

                    temp[variable1].insert(x);

                    std::vector<int> s = {x, y};
                    ((*relationsToTuples)[relation_name]).insert(s);
                }
            }

            // cout << "Number of tuples after filtering from table " << relation_name << " is " << number_of_tuples << endl;
        }
        else
        {

            for (const auto &key_value_pair : loaded_relation)
            {
                int x = get<0>(key_value_pair.first);
                int y = get<1>(key_value_pair.first);
                int multi = key_value_pair.second;

                if ((*variableMap)[variable1].size() > 0 || (*variableMap)[variable2].size() > 0)
                {
                    if ((*variableMap)[variable1].size() > 0)
                    {
                        if ((*variableMap)[variable1].count(x) == 0)
                            continue;
                    }
                    if ((*variableMap)[variable2].size() > 0)
                    {
                        if ((*variableMap)[variable2].count(y) == 0)
                            continue;
                    }
                }

                (*HashMap)["(" + to_string(x) + "," + to_string(y) + ")"] += multi;
                (*HashMapTuple)[make_tuple(x, y)] += multi;

                int currentIndex = hashMaps->size();

                invertedIndicesLeft[currentIndex].emplace(x, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));
                invertedIndicesRight[currentIndex].emplace(y, vector<tuple<int, int>>()).first->second.push_back(make_tuple(x, y));
                number_of_tuples += multi;

                temp[variable1].insert(x);
                temp[variable2].insert(y);

                vector<int> s = {x, y};
                ((*relationsToTuples)[relation_name]).insert(s);

                (*variablesToRelationIndex)["(" + variable1 + "," + variable2 + ")"].emplace_back((*hashMaps).size());
                (*variablesToRelationIndex)["(" + variable2 + "," + variable1 + ")"].emplace_back((*hashMaps).size());
            }
        }

        if (!isInteger(variable1) && !isInteger(variable2))
        {
            (*relationsToVariablePairs)[relation_name].insert({variable1, variable2});
        }

        if (!isInteger(variable1))
        {
            if (variables_set.count(variable1) == 0)
            {
                (*variableMap)[variable1].insert(temp[variable1].begin(), temp[variable1].end());
                variables_set.insert(variable1);
            }
            else
            {
                (*variableMap)[variable1] = intersection(((*variableMap)[variable1]), temp[variable1]);
            }
        }
        if (!isInteger(variable2))
        {
            if (variables_set.count(variable2) == 0)
            {
                (*variableMap)[variable2].insert(temp[variable2].begin(), temp[variable2].end());
                variables_set.insert(variable2);
            }
            else
            {
                (*variableMap)[variable2] = intersection(((*variableMap)[variable2]), temp[variable2]);
            }
        }
        loaded_relations_totals[hashMaps->size()] = number_of_tuples;
        hashMaps->emplace_back((*HashMap));
        hashMapsTuple->push_back((*HashMapTuple));

        int vars[] = {0, 0};
        bool repl[] = {true, true};

        if (isInteger(variable1))
        {
            vars[0] = stoi(variable1);
            repl[0] = false;
        }
        if (isInteger(variable2))
        {
            vars[1] = stoi(variable2);
            repl[1] = false;
        }
        constrs->push(vars, repl, variable1, variable2, relation_name);
        return 1;
    }

    //________________________________________________ Just some helper / print functions below _________________________________________________________________________________

    bool isInteger(const string &s)
    {
        try
        {
            stoi(s);
        }
        catch (...)
        {
            return false;
        }
        return true;
    }

    int count_relations(const string &query)
    {
        int count = 0;
        regex relation_regex(R"(\b\w+\s*\()");
        sregex_iterator it(query.begin(), query.end(), relation_regex);
        sregex_iterator end;

        while (it != end)
        {
            ++count;
            ++it;
        }
        return count;
    }

    static const set<int> intersection(set<int> &set1, set<int> &set2)
    {
        set<int> result;
        set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                         inserter(result, result.begin()));
        return result;
    }
};

#endif