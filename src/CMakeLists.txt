# Generated from src.pro.

add_subdirectory(websockets)
if(TARGET Qt::Quick)
    add_subdirectory(imports)
endif()
