
include_directories(../include)

#
# PoIrp
#
list(APPEND POIRP_DRV_SOURCE
    ../kmtest_drv/kmtest_standalone.c
    PoIrp_drv.c)

add_library(poirp_drv MODULE ${POIRP_DRV_SOURCE})
set_module_type(poirp_drv kernelmodedriver)
target_link_libraries(poirp_drv kmtest_printf ${PSEH_LIB})
add_importlibs(poirp_drv ntoskrnl hal)
target_compile_definitions(poirp_drv PRIVATE KMT_STANDALONE_DRIVER)
#add_pch(poirp_drv ../include/kmt_test.h)
add_rostests_file(TARGET poirp_drv)

#
# PoPowerSetting
#
list(APPEND POPOWERSETTING_DRV_SOURCE
    ../kmtest_drv/kmtest_standalone.c
    PoPowerSetting_drv.c)

add_library(popowersetting_drv MODULE ${POPOWERSETTING_DRV_SOURCE})
set_module_type(popowersetting_drv kernelmodedriver)
target_link_libraries(popowersetting_drv kmtest_printf poguid ${PSEH_LIB})
add_importlibs(popowersetting_drv ntoskrnl hal)
target_compile_definitions(popowersetting_drv PRIVATE KMT_STANDALONE_DRIVER)
#add_pch(popowersetting_drv ../include/kmt_test.h)
add_rostests_file(TARGET popowersetting_drv)

#
# PoIdle
#
list(APPEND POIDLE_DRV_SOURCE
    ../kmtest_drv/kmtest_standalone.c
    PoIdle_drv.c)

add_library(poidle_drv MODULE ${POIDLE_DRV_SOURCE})
set_module_type(poidle_drv kernelmodedriver)
target_link_libraries(poidle_drv kmtest_printf ${PSEH_LIB})
add_importlibs(poidle_drv ntoskrnl hal)
target_compile_definitions(poidle_drv PRIVATE KMT_STANDALONE_DRIVER)
#add_pch(poidle_drv ../include/kmt_test.h)
add_rostests_file(TARGET poidle_drv)

#
# PoPowerState
#
list(APPEND POPOWERSTATE_DRV_SOURCE
    ../kmtest_drv/kmtest_standalone.c
    PoPowerState_drv.c)

add_library(popowerstate_drv MODULE ${POPOWERSTATE_DRV_SOURCE})
set_module_type(popowerstate_drv kernelmodedriver)
target_link_libraries(popowerstate_drv kmtest_printf ${PSEH_LIB})
add_importlibs(popowerstate_drv ntoskrnl hal)
target_compile_definitions(popowerstate_drv PRIVATE KMT_STANDALONE_DRIVER)
#add_pch(popowerstate_drv ../include/kmt_test.h)
add_rostests_file(TARGET popowerstate_drv)

#
# PoPowerRequest
#
list(APPEND POPOWERREQUEST_DRV_SOURCE
    ../kmtest_drv/kmtest_standalone.c
    PoPowerRequest_drv.c)

add_library(popowerrequest_drv MODULE ${POPOWERREQUEST_DRV_SOURCE})
set_module_type(popowerrequest_drv kernelmodedriver)
target_link_libraries(popowerrequest_drv kmtest_printf ${PSEH_LIB})
add_importlibs(popowerrequest_drv ntoskrnl hal)
target_compile_definitions(popowerrequest_drv PRIVATE KMT_STANDALONE_DRIVER)
#add_pch(popowerrequest_drv ../include/kmt_test.h)
add_rostests_file(TARGET popowerrequest_drv)
