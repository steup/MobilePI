LDPATHS      := $(addprefix -L, ${LDPATHS})
LIBS         := $(addprefix -l, ${LIBS})
INCLUDES     := $(addprefix -I, ${INCLUDES})
OBJECTS      := $(addprefix ${BUILD}/,$(addsuffix .o, ${ARCH_OBJECTS} ${COMMON_OBJECTS}))
TARGET       := ${BIN}/${TARGET}

DEPENDANCIES := $(wildcard ${BUILD}/*.o.d)

.PHONY: all clean

all: ${TARGET}

clean:
	rm -rf ${BUILD} ${BIN}

${BIN} ${BUILD}: %:
	mkdir -p $@

${TARGET}: ${OBJECTS} | ${BIN}
	${CXX} ${LDFLAGS} $^ ${LDPATHS} ${LIBS} -o $@

${OBJECTS}: ${BUILD}/%.o : ${SRC}/%.cpp | ${BUILD}
	${CXX} -MM -MT $@ ${CXXFLAGS} ${INCLUDES} $< -o $@.d
	${CXX} -c ${CXXFLAGS} ${INCLUDES} $< -o $@

include ${DEPENDANCIES}
