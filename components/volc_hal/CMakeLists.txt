idf_component_register(
SRCS 
    "eeprom.c"
    "external_sensor.c"
    "gpio_input_output.c"
    "internal_sensor.c"
    #"power.c"
    "pwm_fan.c"
    "volc_i2c.c"
    "TPS546.c"
    "vcore.c"

INCLUDE_DIRS 
    "include"
    "../asic/include"
    "../stratum/include"

REQUIRES 
    "freertos"
    "driver"
    "esp_event"
    "json"
    idf::app_trace 
)


# Include the header files from "main" directory
target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../main")

# Include the header files from "main/tasks" directory
target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../main/tasks")

# target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../asic/")

# target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../stratum/")