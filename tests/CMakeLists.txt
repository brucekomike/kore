cmake_minimum_required(VERSION 3.16)

foreach(test_name IN ITEMS test_workspaceresolver test_projectscanner test_appsettings)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE kore_core Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
