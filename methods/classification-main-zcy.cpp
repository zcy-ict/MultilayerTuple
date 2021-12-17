#include "classification-main-zcy.h"

using namespace std;


extern int prefix_dims_num;
extern int max_layers_num;
extern uint32_t create_next_layer_rules_num;
extern uint32_t delete_next_layer_rules_num;

void PerformClassificationZcy(CommandStruct &command, ProgramState *program_state, 
                       Classifier &classifier, vector<Rule*> &rules, vector<Trace*> &traces, vector<int> &ans, 
                       int (Classifier::*Lookup)(Trace *trace, int priority), 
                       int (Classifier::*LookupAccess)(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state)) {
    //printf("\nPerformClassification\n");
    int rules_num = rules.size();
    int traces_num = traces.size();

    timeval timeval_start, timeval_end;

    // build
    gettimeofday(&timeval_start,NULL);
    classifier.Create(rules, true);
    gettimeofday(&timeval_end,NULL);
    program_state->build_time = GetRunTimeUs(timeval_start, timeval_end) / 1000000.0;

    // lookup
    //printf("lookup\n");
    int lookup_round = command.lookup_round;
    // printf("lookup_round %d\n", lookup_round);
    // classifier.Reconstruct();

    vector<uint64_t> lookup_times;
    for (int k = 0; k < lookup_round; ++k) {
        gettimeofday(&timeval_start,NULL);
        for (int i = 0; i < traces_num; ++i)
            (classifier.*Lookup)(traces[i], 0);
        gettimeofday(&timeval_end,NULL);
        lookup_times.push_back(GetRunTimeUs(timeval_start, timeval_end));
    }
    uint64_t lookup_time = GetAvgTime(lookup_times);
    program_state->lookup_speed = traces_num / (lookup_time / 1.0);

    // delete
    //printf("delete\n");
    gettimeofday(&timeval_start,NULL);
    for (int i = 0; i < rules_num; i += 4) {
        // printf("delete %d\n", i + 1);
        classifier.DeleteRule(rules[i]);
    }
    gettimeofday(&timeval_end,NULL);
    uint64_t delete_time = GetRunTimeUs(timeval_start, timeval_end);
    int delete_num = (rules_num + 3) / 4;
    program_state->delete_speed = delete_num / (delete_time / 1.0);

    // insert
    //printf("insert\n");
    gettimeofday(&timeval_start,NULL);
    if (command.force_test <= 1)
    for (int i = 0; i < rules_num; i += 4) {
        classifier.InsertRule(rules[i]);
    }
    gettimeofday(&timeval_end,NULL);
    uint64_t insert_time = GetRunTimeUs(timeval_start, timeval_end);
    int insert_num = (rules_num + 3) / 4;
    program_state->insert_speed = insert_num / (insert_time / 1.0);
    program_state->update_speed = (program_state->insert_speed + program_state->delete_speed) / 2;
    
    // others
    classifier.CalculateState(program_state);
    
    // verify
    //printf("verify\n");
    int hit_num = 0;
    for (int i = 0; i < traces_num; ++i) {
        //int priority = classifier.Lookup(traces[i], 0);
        // printf("verify %d\n", i);
        int priority = (classifier.*LookupAccess)(traces[i], 0, NULL, program_state);
        //int priority = (classifier.*LookupAccess)(traces[i], 0, &rules[i], program_state);
        if (command.force_test > 0) {
            if (ans[i] != priority) {
                printf("May be wrong : %d ans %d lookup %d\n", i, ans[i], priority);
                exit(1);
            }
            if (priority != 0)
                ++hit_num;
            //printf("%d\n", priority);
            //break;
        }
        // printf("verify end \n");
    }

    // memory
    program_state->data_memory_size = sizeof(Rule) * rules.size() / 1024.0 / 1024.0;
    program_state->index_memory_size = classifier.MemorySize() / 1024.0 / 1024.0;
    
    // free
    //printf("free\n");
    classifier.Free(false);
}

int ClassificationMainZcy(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans) {
    
    if (command.method_name == "DimTSS") {
        MultilayerTuple multilayertuple;
        prefix_dims_num = command.prefix_dims_num;
        multilayertuple.Init(max_layers_num, true);
        PerformClassificationZcy(command, program_state, multilayertuple, rules, traces, ans, &Classifier::Lookup, &Classifier::LookupAccess);
    } else if (command.method_name == "MultilayerTuple") {
        MultilayerTuple multilayertuple;
        prefix_dims_num = command.prefix_dims_num;
        if (command.next_layer_rules_num > 0) {
            create_next_layer_rules_num = command.next_layer_rules_num;
            delete_next_layer_rules_num = command.next_layer_rules_num / 2;
        }
        multilayertuple.Init(1, true);
        PerformClassificationZcy(command, program_state, multilayertuple, rules, traces, ans, &Classifier::Lookup, &Classifier::LookupAccess);
    } else {
        printf("No such method %s\n", command.method_name.c_str());
    }
    return 0;
}