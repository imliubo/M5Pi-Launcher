file(GLOB_RECURSE SOURCES src/*.c src/assets/source/*.c)
add_library(launcher STATIC ${SOURCES})
