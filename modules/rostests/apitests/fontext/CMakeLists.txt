
add_definitions(
    -D__ROS_LONG64__
    -DWINETEST_USE_DBGSTR_LONGLONG
    -DUNICODE
    -D_UNICODE)

list(APPEND SOURCE
    GetDisplayNameOf.cpp
    shellext.cpp
    testlist.c)

add_executable(fontext_apitest ${SOURCE})
set_module_type(fontext_apitest win32cui)
target_link_libraries(fontext_apitest uuid ${PSEH_LIB} cpprt atl_classes)
add_importlibs(fontext_apitest oleaut32 shlwapi ole32 shell32 user32 msvcrt kernel32 ntdll)
add_rostests_file(TARGET fontext_apitest)
