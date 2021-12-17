#include "classification-main.h"

using namespace std;

void PrintProgramState(CommandStruct command, ProgramState *program_state) {

    if (command.print_mode == 0) {
        printf("\n");
        // PrintTree(program_state);
        printf("method_name: %s\n\n", command.method_name.c_str());

        printf("rules_num: %d\n", program_state->rules_num);
        printf("traces_num: %d\n", program_state->traces_num);
        printf("data_memory_size: %.3f MB\n", program_state->data_memory_size);
        printf("index_memory_size: %.3f MB\n\n", program_state->index_memory_size);

        printf("build_time: %.2f S\n", program_state->build_time);
        printf("lookup_speed: %.2f MLPS\n", program_state->lookup_speed);
        printf("insert_speed: %.2f MUPS\n", program_state->insert_speed);
        printf("delete_speed: %.2f MUPS\n", program_state->delete_speed);
        printf("update_speed: %.2f MUPS\n\n", program_state->update_speed);

        printf("tuples_num: %d\n", program_state->tuples_num);
        printf("tuples_sum: %d\n", program_state->tuples_sum);
        printf("access_tuples_sum: %d\taccess_tuples_avg: %.2f\taccess_tuples_max: %d\n", program_state->access_tuples.sum, 1.0 * program_state->access_tuples.sum / program_state->traces_num, program_state->access_tuples.maxn);
        printf("access_rules_sum: %d\taccess_rules_avg: %.2f\taccess_rules_max: %d\n", program_state->access_rules.sum, 1.0 * program_state->access_rules.sum / program_state->traces_num, program_state->access_rules.maxn);
        
        printf("next_layer_num: %d\n", program_state->next_layer_num);
        printf("\n\n");
    } else if (command.print_mode == 1) {
        //printf("%s\t", command.method_name.c_str());
        printf("%d\t", program_state->rules_num);
        printf("%d\t", program_state->traces_num);

        printf("%.3f\t", program_state->data_memory_size);
        printf("%.3f\t", program_state->index_memory_size);

        printf("%.4f\t", program_state->build_time);
        printf("%.4f\t", program_state->lookup_speed);
        printf("%.2f\t", program_state->insert_speed);
        printf("%.2f\t", program_state->delete_speed);
        printf("%.2f\t", program_state->update_speed);

        printf("%d\t", program_state->tuples_num);
        printf("%d\t", program_state->tuples_sum);
        printf("%d\t%.2f\t%d\t", program_state->access_tuples.sum, 1.0 * program_state->access_tuples.sum / program_state->traces_num, program_state->access_tuples.maxn);
        printf("%d\t%.2f\t%d\t", program_state->access_rules.sum, 1.0 * program_state->access_rules.sum / program_state->traces_num, program_state->access_rules.maxn);

        printf("%d\t", program_state->next_layer_num);
        printf("\n");
    }
}

int ClassificationMain(CommandStruct command) {

    vector<Rule*> rules = ReadRules(command.rules_file, command.rules_shuffle);

    vector<Trace*> traces = ReadTraces(command.traces_file);

    if (command.prefix_dims_num == 5)
        rules = RulesPortPrefix(rules, true);
    
    vector<int> ans = GenerateAns(rules, traces, command.force_test);
    
    ProgramState *program_state = new ProgramState();
    program_state->rules_num = rules.size();
    program_state->traces_num = traces.size();

    if (command.method_name == "MultilayerTuple" || command.method_name == "DimTSS") {
        ClassificationMainZcy(command, program_state, rules, traces, ans);
    } else if (command.method_name == "PSTSS" || command.method_name == "PartitionSort" ||
        command.method_name == "TupleMerge") {
        ClassificationMainPS(command, program_state, rules, traces, ans);
    } else {
        printf("No such method %s\n", command.method_name.c_str());
    }

    PrintProgramState(command, program_state);
    
    FreeRules(rules);
    FreeTraces(traces);
    return 0;
}