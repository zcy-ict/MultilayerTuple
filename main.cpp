#include "elementary.h"
#include "methods/classification-main.h"


using namespace std;

int main(int argc, char *argv[]) {
    CommandStruct command = ParseCommandLine(argc, argv);
    if (command.run_mode == "classification") {
        ClassificationMain(command);
    } else {
    	printf("run_mode does not exist\n");
    }
}