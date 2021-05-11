# ZCY Makefile

OBPATH = objects/
OBJECTS = $(shell find -name "*.cpp")
OBJECTS_O = $(OBJECTS:./%.cpp=$(OBPATH)%.o)
OBJECTS_D = $(OBJECTS:./%.cpp=$(OBPATH)%.d)

CXX = g++ -g -std=c++14 -O3
CXXFLAGS = -fpermissive -fopenmp -mpopcnt -mbmi2

main: $(OBJECTS_O)
	@$(CXX) -o main $(OBJECTS_O) $(CXXFLAGS)
	@echo $(CXX) -o main *.o $(CXXFLAGS)

-include $(OBJECTS_D)

$(OBJECTS_D) : $(OBPATH)%.d : %.cpp
	@mkdir -p $(dir $@); \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,$(notdir $*.o):,$(OBPATH)$*.o:,g' $@.$$$$ > $@; \
	echo "\t$(CXX) -o $(OBPATH)$*.o -c $< $(CXXFLAGS)" >> $@; \
	rm -f $@.$$$$

.PHONY: clean

clean:
	rm -rf $(OBPATH)
	rm -rf output
	rm -rf main
