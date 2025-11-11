# Register all source files in the component directory
file(GLOB SRCS "*.cc" "*.cpp")

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "."
    REQUIRES nvs_flash MPU6050 I2Cdev bt esp-tflite-micro
)

target_include_directories(${COMPONENT_TARGET} PRIVATE
    "${IDF_PATH}/components/bt/host/nimble/nimble/nimble/host/include/host"
)
