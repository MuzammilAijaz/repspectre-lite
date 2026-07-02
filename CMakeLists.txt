# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# # Tell ESP-IDF where to find extra components
# set(EXTRA_COMPONENT_DIRS
#     $ENV{TFLITE_MICRO_PATH} # Add the esp-tflite-micro library as an esp component
# )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(magic-wand)
