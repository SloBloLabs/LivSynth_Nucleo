.PHONY : clean all helpsummary

CXX        = clang++
#CXXFLAGS   = -std=c++11 \
             -Weverything -Wall -Wextra -Werror -Wpointer-arith -Wcast-qual \
             -Wno-missing-braces -Wempty-body -Wno-error=uninitialized \
             -Wno-error=deprecated-declarations \
             -pedantic-errors -pedantic \
             -Os

CXXFLAGS   = -std=c++11 \
             -Wall -Wextra -Wpointer-arith -Wcast-qual \
             -Wno-missing-braces -Wempty-body -Wno-error=uninitialized \
             -Wno-error=deprecated-declarations \
             -Os

LD         = clang++ -o
LDFLAGS    = -Wall -pedantic

RM         = rm -f

INCLUDE := -I. -I../App
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.obj, $(SOURCES))
TARGETS := $(patsubst %.cpp,%.app, $(SOURCES))

all: $(TARGETS)

$(TARGETS): $(OBJECTS)
	@$(LD) $@ $(LDFLAGS) $(OBJECTS)
	@echo "Linking complete!"

%.obj : %.cpp
	@$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@
	@echo "Compilation complete!"

helpsummary:
	@echo "SOURCES  : $(SOURCES)"
	@echo "OBJECTS  : $(OBJECTS)"
	@echo "TARGETS  : $(TARGETS)"
	@echo "CXXFLAGS : $(CXXFLAGS)"

clean:
	@$(RM) $(OBJECTS) $(TARGETS)
	@echo "Cleanup complete!"