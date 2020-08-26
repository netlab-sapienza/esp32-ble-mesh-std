cmake_minimum_required(VERSION 3.5)
list(APPEND PROJ_COMPS_LIST)

# PROJ_COMPS stores the name of the all custom components required by the project
set(PROJ_COMPS "init" "server" "client" "common" "relay")

# Each component directory is in "components"
set(BASE_COMPS_PATH "components/")

# We build each project component path in the form "components/component_name" and store it in PROJ_COMPS_LIST
foreach (item ${PROJ_COMPS})
    set(tmp ${BASE_COMPS_PATH}${item})
    list(APPEND PROJ_COMPS_LIST ${tmp})
endforeach ()

# We include the project.cmake which is necessary to run "idf.py build"
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# We include additional esp32 standard components and store them in ADDITIONAL_COMPONENTS
list(APPEND ADDITIONAL_COMPONENTS "$ENV{IDF_PATH}/components/nvs_flash")

set(EXTRA_COMPONENT_DIRS ${ADDITIONAL_COMPONENTS} ${PROJ_COMPS_LIST})

project(esp32-ble-mesh-std)
