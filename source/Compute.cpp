#include "libs.h"
#include "Constraints.cpp"
#include "Parser.cpp"

using namespace std;

class Compute
{
private:
    // checks if partial variable assignment combination is valid, used for early pruning of invalid combinations
    bool isPartialCombinationValid(const string &var, int value, const unordered_map<string, int> &current, vector<unordered_map<string, int>> &HashMaps, Constraints &constr, unordered_map<string, vector<int>> &varPairToTableIdx)
    {
        for (const auto &c : (constr).constrs)
        {
            if ((c.name1 == var && current.count(c.name2)) || (c.name2 == var && current.count(c.name1)))
            {
                int value1 = c.name1 == var ? value : (c.repl[0] ? current.at(c.name1) : c.var[0]);
                int value2 = c.name2 == var ? value : (c.repl[1] ? current.at(c.name2) : c.var[1]);
                const string key = "(" + to_string(value1) + "," + to_string(value2) + ")";
                const string var_combo = "(" + c.name1 + "," + c.name2 + ")";

                if ((HashMaps)[(varPairToTableIdx)[var_combo][0]].count(key) == 0)
                {
                    return false;
                }
            }
        }
        return true;
    }

    // recursively generates valid variable assignment combinations that satisfy provided constraints, using DFS with pruning of invalid combinations early
    void generateCombinations(int index, vector<string> &keys, unordered_map<string, int> &current, vector<unordered_map<string, int>> *result, vector<unordered_map<string, int>> &HashMaps, Constraints &constr, map<string, set<int>> &variableMap, unordered_map<string, vector<int>> &varPairToTableIdx)
    {
        if (index == keys.size())
        {
            result->emplace_back(current);
            return;
        }

        for (int value : (variableMap)[keys[index]])
        {
            if (isPartialCombinationValid(keys[index], value, current, HashMaps, constr, varPairToTableIdx))
            {
                current[keys[index]] = value;
                generateCombinations(index + 1, keys, current, result, HashMaps, constr, variableMap, varPairToTableIdx);
            }
            current.erase(keys[index]);
        }
    }

    // reduces the domains of variables by iteratively propagating constraints until no more values can be removed from the vaiable domains
    // implements arc consistency algorithm for binary CSP
    int reduction(map<string, set<vector<int>>> &relationsToTuples,
                  map<string, set<vector<string>>> &relationsToVariablePairs,
                  map<string, set<int>> &variableMap)
    {
        int removed = 1;
        while (removed != 0)
        {
            removed = 0;
            for (int por = 0; por < 2; por++)
            {
                for (const auto &kvp : relationsToVariablePairs)
                {
                    const string &relation_name = kvp.first;
                    const auto &variable_pairs = kvp.second;
                    for (const auto &variable_pair : variable_pairs)
                    {
                        const string &var1 = variable_pair[por];
                        const string &var2 = variable_pair[1 - por];
                        set<int> infered_domain;
                        const set<int> &main_domain = variableMap[var1];
                        for (const auto &tuple : relationsToTuples[relation_name])
                        {
                            int a = tuple[0];
                            int b = tuple[1];
                            if ((por == 0 && main_domain.count(a) > 0) || (por == 1 && main_domain.count(b) > 0))
                            {
                                infered_domain.insert((por == 0) ? b : a);
                            }
                        }
                        set<int> &target_domain = variableMap[var2];
                        int before = target_domain.size();
                        set<int> infered_set(infered_domain.begin(), infered_domain.end());
                        target_domain = intersection(target_domain, infered_set);
                        int after = target_domain.size();
                        removed += before - after;
                    }
                }
            }
        }
        return 0;
    }

    // generates all valid combinations of variable assignments that satisfy the constraints
    vector<unordered_map<string, int>> *getAllCombinations(vector<unordered_map<string, int>> &HashMaps, Constraints &constr, map<string, set<int>> &variableMap, unordered_map<string, vector<int>> &var_pair_to_table_index)
    {
        auto result = new vector<unordered_map<string, int>>();
        unordered_map<string, int> current;
        vector<string> keys;
        keys.reserve(variableMap.size());

        for (const auto &pair : variableMap)
        {
            keys.emplace_back(pair.first);
        }

        // sort keys
        sort(keys.begin(), keys.end(), [&variableMap](const string &a, const string &b)
             { return variableMap.at(a).size() < variableMap.at(b).size(); });

        // apply backtracking algorithm
        generateCombinations(0, keys, current, result, HashMaps, constr, variableMap, var_pair_to_table_index);

        return result;
    }

    static set<int> intersection(const set<int> &set1, const set<int> &set2)
    {
        set<int> result;
        set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                         inserter(result, result.begin()));
        return result;
    }

public:
    // compute exact number of answers and measure time taken (combines reduction, generating combinations, backtracking and pruning)
    int exact_count = 0;
    int computeExactAnswer(vector<unordered_map<string, int>> &HashMaps, map<string, set<vector<int>>> &relationsToTuples, map<string, set<vector<string>>> &relationsToVariablePairs, map<string, set<int>> &variableMap, Constraints &constr, unordered_map<string, vector<int>> &varPairToTableIdx)
    {
        clock_t start_time = clock();

        // iteratively reduce variable domains
        int reduced = reduction(relationsToTuples, relationsToVariablePairs, variableMap);

        int assignment_row_count = 1;

        // generate valid variable assignment combinations
        const auto &allCombinations = *this->getAllCombinations(HashMaps, constr, variableMap, varPairToTableIdx);

        int num_of_tables = (HashMaps).size();
        int i = 0;

        vector<string> konfig;
        konfig.reserve(num_of_tables);

        // count the exact number of answers
        for (auto &assignment : allCombinations)
        {
            assignment_row_count = 1;
            konfig = (constr).getKey(assignment);
            for (int c = 0; c < num_of_tables; c++)
            {
                assignment_row_count *= ((HashMaps)[c])[konfig[c]];
            }
            this->exact_count += assignment_row_count;
            i++;
        }

        clock_t end_time = clock();
        double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
        cout << "Exact number of answers: " << exact_count << endl;
        cout << "Exact answering time: " << time_taken << " ms" << endl;
        return exact_count;
    }
};