# Register all source files in the component directory
file(GLOB SRCS "*.cc")

idf_component_register(
    SRCS
        ${SRCS}
    INCLUDE_DIRS
        "."
)

