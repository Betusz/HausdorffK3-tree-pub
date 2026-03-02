cmake_minimum_required(VERSION 3.28)
project(HausdorffK3_tree_pub LANGUAGES CXX)

# Estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opciones de compilación (Optimización para Release)
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# --- BIBLIOTECA PRINCIPAL ---
# Usamos CONFIGURE_DEPENDS para que CMake detecte nuevos archivos automáticamente en CLion
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Excluir archivos específicos si fuera necesario
# list(FILTER LIB_SOURCES EXCLUDE REGEX "src/main.cpp$")

# Incluir todos los archivos .cpp de libcds (ya incluidos por GLOB_RECURSE en src/*.cpp)
# Pero nos aseguramos de que se incluyan todos los subdirectorios
file(GLOB_RECURSE LIBCDS_SOURCES CONFIGURE_DEPENDS "src/libcds/**/*.cpp")
list(APPEND LIB_SOURCES ${LIBCDS_SOURCES})
list(REMOVE_DUPLICATES LIB_SOURCES)

# Incluir todos los .h y .hpp de include/ para que aparezcan en el árbol de CLion
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "include/*.h" "include/*.hpp")

add_library(hausdorff_lib ${LIB_SOURCES} ${LIB_HEADERS})

# Añadir los directorios de inclusión
# Usamos include/libcds para que libcds encuentre sus propias cabeceras
target_include_directories(hausdorff_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/libcds>
)

# --- EJECUTABLES ---
# Función auxiliar para añadir ejecutables de la carpeta apps
function(add_app_executable name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE hausdorff_lib)
endfunction()

add_app_executable(main_app apps/main.cpp)
add_app_executable(benchmark_app apps/benchmark.cpp)
add_app_executable(experiment_app apps/experiment.cpp)
add_app_executable(experiment1_app apps/experiment1.cpp)
add_app_executable(compare_metrics_app apps/compare_metrics.cpp)
add_app_executable(benchmark_ryu_kamata_app apps/benchmark_ryu_kamata.cpp)
add_app_executable(comprehensive_benchmark_app apps/comprehensive_benchmark.cpp)
add_app_executable(snapshot_benchmark_app apps/snapshot_benchmark.cpp)
if(WIN32)
    target_link_libraries(snapshot_benchmark_app PRIVATE psapi)
endif()

# --- GESTIÓN DE PRUEBAS ---
enable_testing()
add_subdirectory(tests)