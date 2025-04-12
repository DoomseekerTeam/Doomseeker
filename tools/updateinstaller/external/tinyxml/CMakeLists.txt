project(tinyxml)

set(SOURCES
  tinystr.cpp
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

set(HEADERS
  tinystr.h
  tinyxml.h
)

add_library(tinyxml
  ${SOURCES}
  ${HEADERS}
)

target_compile_definitions(tinyxml PRIVATE TIXML_USE_STL)

