#include "classification-main-ps.h"

using namespace std;


// TupleInfo main_tuple_info;
// extern int rangecuts_split_rules_num;

void PerformClassificationPS(CommandStruct &command, ProgramState *program_state, 
                       PacketClassifier &classifier, vector<PSRule> &rules, vector<PSPacket> &traces, vector<int> &ans, 
                       int (PacketClassifier::*Lookup)(const PSPacket& packet), 
                       int (PacketClassifier::*LookupAccess)(const PSPacket& packet, ProgramState *program_state)) {
    //printf("\nPerformClassification\n");
    int rules_num = rules.size();
    int traces_num = traces.size();

    timeval timeval_start, timeval_end;

    // build
    gettimeofday(&timeval_start,NULL);
    classifier.ConstructClassifier(rules);
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
            (classifier.*Lookup)(traces[i]);
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
        int priority = (classifier.*LookupAccess)(traces[i], program_state);
        // int priority = (classifier.*LookupAccess)(traces[i], 0, NULL, program_state);
        //int priority = (classifier.*LookupAccess)(traces[i], 0, &rules[i], program_state);
        if (command.force_test > 0) {
            if (ans[i] != priority) {
                printf("May be wrong : %d ans %d priority %d\n", i, ans[i], priority);
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
    program_state->data_memory_size = sizeof(PSRule) * rules.size() / 1024.0 / 1024.0;
    program_state->index_memory_size = classifier.MemSizeBytes() / 1024.0 / 1024.0;


    
    // free
    //printf("free\n");
    // classifier.Free(false);
}

int ClassificationMainPS(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans) {
    vector<PSRule> ps_rules = GeneratePSRules(rules);
    // PrintByteCutsRules(bc_rules);
    vector<PSPacket> packets = GeneratePSPackets(traces);

    // PerformPartitionSort();
    if (command.method_name == "PSTSS") {
        PriorityTupleSpaceSearch ptss;
        // PerformOnlyPacketClassification(ptss, ps_rules, packets);
        PerformClassificationPS(command, program_state, ptss, ps_rules, packets, ans, &PacketClassifier::ClassifyAPacket, &PacketClassifier::ClassifyAPacketAccess);
    } else if (command.method_name == "PartitionSort") {
        PartitionSort partitionsort;
        // PerformOnlyPacketClassification(ptss, ps_rules, packets);
        PerformClassificationPS(command, program_state, partitionsort, ps_rules, packets, ans, &PacketClassifier::ClassifyAPacket, &PacketClassifier::ClassifyAPacketAccess);
    } else if (command.method_name == "TupleMerge") {
        unordered_map<std::string, std::string> args;
        TupleMergeOnline tuplemerge(args);
        // PerformOnlyPacketClassification(ptss, ps_rules, packets);
        PerformClassificationPS(command, program_state, tuplemerge, ps_rules, packets, ans, &PacketClassifier::ClassifyAPacket, &PacketClassifier::ClassifyAPacketAccess);
    }
    return 0;
}