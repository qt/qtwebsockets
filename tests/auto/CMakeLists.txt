# Generated from auto.pro.

# add_subdirectory(cmake) # special case remove
add_subdirectory(websockets)
if(TARGET Qt::Quick)
    add_subdirectory(qml)
endif()
