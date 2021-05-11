#include "Simulation.h"

using namespace std;


vector<int> PerformOnlyPacketClassification(PacketClassifier& classifier, vector<PSRule> &rules, vector<PSPacket> &packets) {

	std::map<std::string, std::string> summary;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;
	std::chrono::duration<double,std::milli> elapsed_milliseconds;

	start = std::chrono::steady_clock::now();
	classifier.ConstructClassifier(rules);
	end = std::chrono::steady_clock::now();
	elapsed_milliseconds = end - start;
	printf("\tConstruction time: %f ms\n", elapsed_milliseconds.count());
	summary["ConstructionTime(ms)"] = std::to_string(elapsed_milliseconds.count());

	const int trials = 10;
	std::chrono::duration<double> sum_time(0);
	std::vector<int> results;
	for (int t = 0; t < trials; t++) {
		results.clear();
		start = std::chrono::steady_clock::now();
		for (auto const &p : packets) {
			results.push_back(classifier.ClassifyAPacket(p));
		}
		end = std::chrono::steady_clock::now();
		elapsed_seconds = end - start;
		sum_time += elapsed_seconds; 
	} 

	printf("\tClassification time: %f s\n", sum_time.count() / trials);
	summary["ClassificationTime(s)"] = std::to_string(sum_time.count() / trials);
	
	int memSize = classifier.MemSizeBytes();
	printf("\tSize(bytes): %d \n", memSize);
	summary["Size(bytes)"] = std::to_string(memSize);
	int memAccess = classifier.MemoryAccess();
	printf("\tMemoryAccess: %d \n", memAccess);
	summary["MemoryAccess"] = std::to_string(memAccess);
	int numTables = classifier.NumTables();
	printf("\tTables: %d \n", numTables);
	summary["Tables"] = std::to_string(numTables);
	
	// std::stringstream ssTableSize, ssTableQuery;
	// for (int i = 0; i < numTables; i++) {
	// 	if (i != 0) {
	// 		ssTableSize << "-";
	// 		ssTableQuery << "-";
	// 	}
	// 	ssTableSize << classifier.RulesInTable(i);
	// 	ssTableQuery << classifier.NumPacketsQueriedNTables(i + 1);
	// }
	// summary["TableSizes"] = ssTableSize.str();
	// summary["TableQueries"] = ssTableQuery.str();

	printf("\tTotal tables queried: %d\n", classifier.TablesQueried());
	printf("\tAverage tables queried: %f\n", 1.0 * classifier.TablesQueried() / (trials * packets.size()));
	summary["AvgQueries"] = std::to_string(1.0 * classifier.TablesQueried() / (trials * packets.size()));

	return results;
}