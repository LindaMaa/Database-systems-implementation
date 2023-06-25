#include "libs.h"
#include "Parser.cpp"
#include "Constraints.cpp"
#include "Compute.cpp"
#include "WanderJoin.cpp"
#include "WJOptim.cpp"
#include "DataStructures.h"

using namespace std;

int main()
{
    DataStructures ds;
    string input_line;
    string command;

    while (true)
    {
        cout << "> ";
        getline(cin, input_line);
        istringstream input_stream(input_line);
        input_stream >> command;

        if (command == "LOAD")
        {
            string rel_name, file_name, extra;
            input_stream >> rel_name >> file_name;

            if (input_stream.fail() || input_stream >> extra)
            {
                cout << "Error: LOAD command could not be processed. Please follow the format: LOAD <rel_name> <file_name>" << endl;
            }
            else
            {
                int num_tuples = ds.addRelation(rel_name, file_name);
            }
        }

        else if (command == "COUNT")
        {
            string query;
            getline(input_stream, query);
            if (query.empty())
            {
                cout << "Error: COUNT command could not be processed. Please use the format: COUNT <query>" << endl;
            }
            else
            {
                int result;
                vector<unordered_map<string, int>> HashMaps;
                vector<unordered_map<tuple<int, int>, int, hash_tuple>> HashMapsTuple;
                map<string, set<int>> variableMap;
                map<string, set<vector<int>>> relationsToTuples;
                map<string, set<vector<string>>> relationsToVariablePairs;
                unordered_map<string, vector<int>> variablesToRelationIndex;
                Constraints constrs;
                Parser p(&constrs, &HashMaps, &HashMapsTuple, &variableMap, &relationsToTuples, &relationsToVariablePairs, &variablesToRelationIndex);
                Compute c;
                result = p.parseQuery(query, ds);
                if (result == 0)
                {
                    cout << "Could not parse query. Try again if you want." << endl;
                    continue;
                }
                cout << "Query parsed successfully. Start counting." << endl;
                c.computeExactAnswer(HashMaps, relationsToTuples, relationsToVariablePairs, variableMap, constrs, variablesToRelationIndex);
            }
        }
        else if (command == "ESTIMATE")
        {
            string query;
            getline(input_stream, query);
            if (query.empty())
            {
                cout << "Error: ESTIMATE command could not be processed. Please use the format: ESTIMATE <query>" << endl;
            }
            else
            {
                int result;
                vector<unordered_map<string, int>> HashMaps;
                vector<unordered_map<tuple<int, int>, int, hash_tuple>> HashMapsTuple;
                map<string, set<int>> variableMap;
                map<string, set<vector<int>>> relationsToTuples;
                map<string, set<vector<string>>> relationsToVariablePairs;
                unordered_map<string, vector<int>> variablesToRelationIndex;
                Constraints constrs;
                Parser p(&constrs, &HashMaps, &HashMapsTuple, &variableMap, &relationsToTuples, &relationsToVariablePairs, &variablesToRelationIndex);
                Compute c;
                result = p.parseQuery(query, ds);
                if (result == 0)
                {
                    cout << "Could not parse query. Try again if you want." << endl;
                    continue;
                }
                cout << "Query parsed successfully. Start estimation." << endl;
                WJOptim w;
                w.computeEstimate(100, constrs, HashMapsTuple, p);
            }
        }
        else if (command == "QUIT")
        {
            break;
        }
        else
        {
            cout << "Error: Invalid command. Please only use one of the following commands: LOAD, COUNT, ESTIMATE, or QUIT." << endl;
        }
    }

    return 0;
}
