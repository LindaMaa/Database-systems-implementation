#ifndef __CONSTRAINTS__
#define __CONSTRAINTS__
using namespace std;

// stores constraints parsed from the query

struct constr
{
    int var[2];        // 0 if repl variable, otherwise store the value of constant
    bool repl[2];      // 0 if constant, otherwise 1
    string name1;      // left var name
    string name2;      // right var name
    string table_name; // relation name
};

class Constraints
{
public:
    vector<struct constr> constrs;

    void push(int *v, bool *r, string n1, string n2, string tn)
    {
        struct constr c;
        c.var[0] = v[0];
        c.var[1] = v[1];
        c.repl[0] = r[0];
        c.repl[1] = r[1];
        c.name1 = n1;
        c.name2 = n2;
        c.table_name = tn;
        constrs.push_back(c);
    }

    vector<string> getKey(unordered_map<string, int> configur)
    {
        vector<string> ret;

        for (int i = 0; i < constrs.size(); i++)
        {
            struct constr c = constrs[i];
            string key = "(";
            if (c.repl[0])
            {
                key += to_string(configur[c.name1]);
            }
            else
            {
                key += to_string(c.var[0]);
            }
            key += ',';
            if (c.repl[1])
            {
                key += to_string(configur[c.name2]);
            }
            else
            {
                key += to_string(c.var[1]);
            }
            key += ')';
            ret.push_back(key);
        }
        return ret;
    }
};

#endif