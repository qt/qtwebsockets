# Generated from src.pro.

if(QT_FEATURE_textcodec)
    add_subdirectory(websockets)
endif()
if(QT_FEATURE_textcodec AND TARGET Qt::Quick)
    add_subdirectory(imports)
endif()
