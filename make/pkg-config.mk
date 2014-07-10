CXXFLAGS      := ${CXXFLAGS} $(shell pkg-config ${PKG_NAME} --cflags)
LD_POST       := ${LD_POST} $(shell pkg-config ${PKG_NAME} --libs-only-L --libs-only-l) 
LD_PRE        := ${LD_PRE} $(shell pkg-config ${PKG_NAME} --libs-only-other) 
