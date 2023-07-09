set(SRC lentil.cpp lentil_loader.cpp lentil_operator.cpp lentil_camera.cpp lentil_filter.cpp lentil_imager.cpp)
set(SHADER lentil)
set(UI ${CMAKE_CURRENT_SOURCE_DIR}/lentil_camera.ui)
set(MTD ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.mtd)
set(AE ${CMAKE_CURRENT_BINARY_DIR}/lentilTemplate.py)
set(KARGS ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.args)
set(C4DRES ${CMAKE_CURRENT_BINARY_DIR}/C4DtoA)
set(PRESET_SCENES ${CMAKE_SOURCE_DIR}/preset_scenes)

add_library(${SHADER} SHARED ${SRC})

target_link_libraries(${SHADER} ai)
set_target_properties(${SHADER} PROPERTIES PREFIX "")

execute_process(COMMAND 
    python3 ${CMAKE_SOURCE_DIR}/src/uigen.py
    --ui_file ${UI} 
    --mtd_output ${MTD} 
    --ae_template_output ${AE} 
    --args_output ${KARGS} 
    --c4d_output ${C4DRES}
)

install(TARGETS ${SHADER} DESTINATION ${DSO_INSTALL_DIR})
install(FILES ${AE} DESTINATION ${AE_INSTALL_DIR})
install(FILES ${KARGS} DESTINATION ${ARGS_INSTALL_DIR})
install(DIRECTORY ${C4DRES} DESTINATION ${INSTALL_DIR})
install(DIRECTORY ${PRESET_SCENES} DESTINATION ${SCENES_INSTALL_DIR})

# Concat MTD files
set(MTD_FILES ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.mtd ${CMAKE_CURRENT_SOURCE_DIR}/lentil_hardcode.mtd)

function(cat IN_FILE OUT_FILE)
  file(READ ${IN_FILE} CONTENTS)
  file(APPEND ${OUT_FILE} "${CONTENTS}")
endfunction()

# Not sure why these files need to be deleted otherwise they don't update
file (REMOVE ${MTD_INSTALL_DIR}/lentil.mtd)
file (REMOVE ${CMAKE_CURRENT_BINARY_DIR}/lentil.mtd)

file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/lentil.mtd "")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/lentil.mtd "")
foreach(MTD_FILE ${MTD_FILES})
  cat(${MTD_FILE} ${CMAKE_CURRENT_BINARY_DIR}/lentil.mtd)
endforeach()

set(MTD_LENTIL ${CMAKE_CURRENT_BINARY_DIR}/lentil.mtd)
install(FILES ${MTD_LENTIL} DESTINATION ${MTD_INSTALL_DIR})