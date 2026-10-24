# DPF.dll on alandtse's CommonLibSSE-NG (Skyrim 1.6.1170 and newer (AE, including 1.7.x)); included by E:/WorkSpace/ng-build/CMakeLists.txt (see ng_plugin there).
ng_plugin(TARGET DPF NAME DPF VERSION 0.1.0 AUTHOR "Thiago e Viny, CageTV" ROOT "${CMAKE_CURRENT_LIST_DIR}"
    SOURCES src/Services.cpp src/form.cpp src/form_record.cpp src/form_record_serializer.cpp src/form_serializer.cpp src/model.cpp
            src/papyrus.cpp src/persistence.cpp src/plugin.cpp src/serializer.cpp
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/include"
    DEFINES NOMINMAX WIN32_LEAN_AND_MEAN
    OPTIONS /utf-8 /wd4200
    PCH "${CMAKE_CURRENT_LIST_DIR}/include/PCH.h")
