LD_PRE       += $(foreach flag, ${LDFLAGS}, -Wl,${flag})
LD_POST      += $(addprefix -L, ${LDPATHS}) $(addprefix -l, ${LIBS})
CXXFLAGS     += $(addprefix -I, ${INCLUDE} ${INCLUDES})
OBJECTS      := $(addprefix ${BUILD}/,$(addsuffix .o, ${OBJECTS}))
TARGET       := ${BIN}/${TARGET}

DEPENDANCIES := $(wildcard ${BUILD}/*.o.d)

.PHONY: all clean run

all: ${TARGET}

run: ${TARGET}
	@echo -e "Executing $<\n"
	@./$<

clean:
	@echo "Cleaning [${BUILD} ${BIN}]"
	@rm -rf ${BUILD} ${BIN}

${BIN} ${BUILD}: %:
	@echo "Creating Directory $@"
	@mkdir -p $@

${TARGET}: ${OBJECTS} | ${BIN}
	@echo "Linking $@ <- [$^]"
	@${CXX} ${LD_PRE} $^ ${LD_POST} -o $@

${OBJECTS}: ${BUILD}/%.o : ${SRC}/%.cpp | ${BUILD}
	@echo "Building Dependancy $@ <- $<"
	@${CXX} -MM -MT $@ ${CXXFLAGS} $< -o $@.d
	@echo "Compiling $@ <- $<"
	@${CXX} -c ${CXXFLAGS} $< -o $@

include ${DEPENDANCIES}
