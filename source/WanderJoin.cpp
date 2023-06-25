#include "libs.h"
#include "Constraints.cpp"
#include "Parser.cpp"

using namespace std;

random_device rd;
mt19937 gen(rd());

class WanderJoin
{
private:
    vector<int> tableIndices;
    vector<vector<string>> joiningVariablesForPaths;

    // functions for path generation
    bool isValidSequence(const vector<int> &sequence, const unordered_map<string, vector<int>> &varToTable)
    {
        for (int i = 0; i < sequence.size() - 1; ++i)
        {
            bool found = false;
            for (const auto &entry : varToTable)
            {
                const vector<int> &tables = entry.second;
                if (find(tables.begin(), tables.end(), sequence[i]) != tables.end() &&
                    find(tables.begin(), tables.end(), sequence[i + 1]) != tables.end())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return false;
            }
        }
        return true;
    }

    long factorial(const int n)
    {
        long f = 1;
        for (int i = 1; i <= n; ++i)
            f *= i;
        return f;
    }

    void generatePermutations(vector<int> &elements, int index, vector<vector<int>> &result, const unordered_map<string, vector<int>> &varToTable)
    {
        if (index == elements.size() - 1)
        {
            if (isValidSequence(elements, varToTable))
            {
                result.emplace_back(elements);
            }
        }
        for (int i = index; i < elements.size(); ++i)
        {
            swap(elements[index], elements[i]);
            generatePermutations(elements, index + 1, result, varToTable);
            swap(elements[index], elements[i]);
        }
    }

    const vector<vector<int>> generatePaths(const unordered_map<string, vector<int>> &varToTable)
    {
        int n = this->tableIndices.size();
        vector<int> all_tables;
        for (int i = 0; i < n; ++i)
        {
            all_tables.emplace_back(i);
        }

        vector<vector<int>> paths;
        paths.reserve(factorial(n));
        generatePermutations(all_tables, 0, paths, varToTable);

        for (const auto &path : paths)
        {
            joiningVariablesForPaths.emplace_back(getJoiningVariablesForPath(path, varToTable));
        }

        return paths;
    }

    const vector<string> getJoiningVariablesForPath(const vector<int> &path, const unordered_map<string, vector<int>> &varToTable)
    {
        vector<string> joiningVariables;
        joiningVariables.reserve(path.size() - 1);
        for (int i = 0; i < path.size() - 1; ++i)
        {
            for (const auto &entry : varToTable)
            {
                const vector<int> &tables = entry.second;
                if (find(tables.begin(), tables.end(), path[i]) != tables.end() &&
                    find(tables.begin(), tables.end(), path[i + 1]) != tables.end())
                {
                    joiningVariables.emplace_back(entry.first);
                    break;
                }
            }
        }
        return joiningVariables;
    }

    const unordered_map<string, vector<int>> createVarToIndexMap(const vector<struct constr> &constrs)
    {
        unordered_map<string, vector<int>> var_to_index_map;

        for (size_t i = 0; i < constrs.size(); i++)
        {
            tableIndices.emplace_back(i);

            const struct constr &c = constrs[i];
            if (c.repl[0])
            {
                var_to_index_map[c.name1].emplace_back(i);
            }
            if (c.repl[1])
            {
                var_to_index_map[c.name2].emplace_back(i);
            }
        }

        return var_to_index_map;
    }

    // sample tuple in 1st relation
    tuple<int, int> getFirstRandomTuple(const unordered_map<tuple<int, int>, int, hash_tuple> &hashMap, int &cnt, long double &prob, int total_count)
    {
        tuple<int, int> selectedTuple;
        uniform_int_distribution<> dis(1, total_count);
        int randomIndex = dis(gen);

        int runningCount = 0;
        for (const auto &entry : hashMap)
        {
            runningCount += entry.second;
            if (runningCount >= randomIndex)
            {
                selectedTuple = entry.first;
                cnt = entry.second;
                break;
            }
        }

        prob = ((long double)cnt) / total_count; // multiplicity of selected tuple / all tuples in relation
        return selectedTuple;
    }

    // sample next tuple
    int findMatchInNextTable(int right_table_index, int joining_key, int position, tuple<int, int> &next_key, int &multi, long double &prob, vector<unordered_map<tuple<int, int>, int, hash_tuple>> &hashMaps, Parser &p)
    {
        int totalCount;

        const vector<tuple<int, int>> &matchedEntries = (position == 0) ? (p).invertedIndicesLeft[right_table_index][joining_key] : (p).invertedIndicesRight[right_table_index][joining_key];

        totalCount = matchedEntries.size();

        if (totalCount == 0)
        {
            return 0; // no match found
        }

        uniform_int_distribution<> distr(0, totalCount - 1);
        int randomIndex = distr(gen);
        next_key = matchedEntries[randomIndex];

        multi = (hashMaps)[right_table_index][next_key]; // multiplicity of next key in hashmap

        prob = ((long double)multi) / totalCount;

        return multi;
    }

public:
    const long double computeEstimate(int num_samples, Constraints &constr, vector<unordered_map<tuple<int, int>, int, hash_tuple>> &hashMaps, Parser &p)
    {
        clock_t start_time = clock();
        double totalEstimate = 0.0;
        const unordered_map<string, vector<int>> varToTable = createVarToIndexMap((constr).constrs);
        const vector<vector<int>> generatedPaths = generatePaths(varToTable);
        int size = generatedPaths.size();
        if (size == 0)
        {
            clock_t end_time = clock();
            double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
            std::cout << "CLASSIC WJ -> Estimated number of answers: 0" << endl;
            cout << "CLASSIC WJ -> Estimation time: " << time_taken << " ms" << endl;
            return 0;
        }
        int path_size = generatedPaths[0].size();
        uniform_int_distribution<size_t> dis(0, size - 1);
        int joins_count;
        long double joins_prob;
        long double prob;
        int cnt;
        tuple<int, int> curr_tuple;
        int left_table_index;
        int left_table_total_count;
        int right_table_index;
        string joining_var;
        int joining_attr_value;

        for (int s = 0; s < num_samples; s++)
        {

            // pick random order of joining
            size_t path_index = dis(gen);
            vector<int> joiningPath = generatedPaths[path_index];
            const vector<string> &joiningVariables = joiningVariablesForPaths[path_index];
            // vector<string> joiningVariables = joiningVariablesForPaths.at(path_index);

            // initialize count and probs
            joins_count = 1;
            joins_prob = 1.0;

            // pick first tuple in first table at random
            left_table_index = joiningPath[0];
            left_table_total_count = (p.loaded_relations_totals)[left_table_index];

            curr_tuple = this->getFirstRandomTuple((hashMaps)[left_table_index], cnt, prob, left_table_total_count);
            joins_count *= cnt;
            joins_prob *= prob;

            // only single relation
            if (path_size == 1)
            {
                totalEstimate += joins_count / joins_prob;
                continue;
            }

            // iterate through other tables and try to join

            joining_var = joiningVariablesForPaths[path_index][0]; // variable on which we are joining

            for (int i = 1; i < joiningPath.size(); i++)
            {

                if (constr.constrs[left_table_index].name1 == joining_var)
                {
                    joining_attr_value = get<0>(curr_tuple);
                }
                else if (constr.constrs[left_table_index].name2 == joining_var)
                {
                    joining_attr_value = get<1>(curr_tuple);
                }

                right_table_index = joiningPath[i];

                if (constr.constrs[right_table_index].name1 == joining_var)
                {

                    joins_count = joins_count * findMatchInNextTable(right_table_index, joining_attr_value, 0, curr_tuple, cnt, prob, hashMaps, p);

                    if (joins_count == 0)
                    {

                        break;
                    }

                    joins_prob *= prob;
                }

                else if (constr.constrs[right_table_index].name2 == joining_var)
                {

                    joins_count = joins_count * findMatchInNextTable(right_table_index, joining_attr_value, 1, curr_tuple, cnt, prob, hashMaps, p);

                    if (joins_count == 0)
                    {
                        break;
                    }
                    joins_prob *= prob;
                }
                left_table_index = right_table_index;

                if (i < joiningVariablesForPaths[path_index].size())
                {
                    joining_var = joiningVariablesForPaths[path_index][i];
                }
            }
            totalEstimate += joins_count / joins_prob;
        }
        long double averageEstimate = totalEstimate / num_samples;

        clock_t end_time = clock();
        double time_taken = double(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;

        std::cout << "CLASSIC WJ -> Estimated number of answers: " << averageEstimate << endl;
        cout << "CLASSIC WJ -> Estimation time: " << time_taken << " ms" << endl;
        std::cout << "CLASSIC WJ -> Number of samples taken: " << num_samples << endl;

        return averageEstimate;
    }
};
