# HausdorffK3-tree
Implementación del cálculo de la distancia de hausdorff 3D

## Memoria de título:
### Autor 
Bastian Sepúlveda. Estudiante de la carrera Ingeniería Civil Informática de la Universidad del Bío-Bío
2025
### Profesor Guía:
Dr. Miguel Romero Vásquez

## Descripción
Este trabajo incluye la implementación del cálculo de la distancia de Hausdorff en k3-tree de Fernando Silva Coira (https://gitlab.lbd.org.es/fsilva/k3-tree). Para ello toma como base el artículo:
* Domínguez, F., Gutiérrez, G., Penabad, M.R. et al. Efficient algorithms to calculate the Hausdorff distance on point sets represented by a 
k3-tree. Geoinformatica 29, 1067–1092 (2025). https://doi.org/10.1007/s10707-025-00557-9

---

## Estructura del Proyecto

Este proyecto utiliza una estructura moderna de CMake que separa la lógica principal (biblioteca) de los programas que la utilizan (ejecutables) y las pruebas.

```
HausdorffK3-tree/
├── include/                # Archivos de cabecera (.h, .hpp) públicos de la biblioteca.
│   └── HausdorffK3-tree/   # Subdirectorio con el nombre del proyecto para evitar colisiones.
├── src/                    # Archivos de implementación (.cpp) de la biblioteca.
├── apps/                   # Aplicaciones ejecutables que usan la biblioteca.
├── tests/                  # Pruebas unitarias y de integración (usando Google Test).
├── build/                  # Directorio donde se guardan los archivos de compilación.
└── CMakeLists.txt          # Archivo principal que orquesta la compilación de todo el proyecto.
```

- **`src/` y `include/`**: Juntos forman la biblioteca `hausdorff_lib`. Esta contiene toda la funcionalidad principal del proyecto, pero no es un programa ejecutable por sí misma.
- **`apps/`**: Contiene uno o más programas ejecutables. Cada archivo `.cpp` aquí tiene una función `main()` y utiliza la biblioteca para realizar una tarea específica.
- **`tests/`**: Contiene el código para verificar que la biblioteca funciona correctamente. Se compila en un ejecutable de pruebas separado.

## Cómo Compilar y Ejecutar

### Prerrequisitos

- **CMake** (versión 3.15 o superior)
- Un **compilador de C++17** (ej. GCC, Clang, MSVC)
- **Git** (para descargar las dependencias)

### 1. Clonar y Configurar el Proyecto

Primero, clona el repositorio y crea un directorio de compilación:

```bash
# Clona este repositorio
git clone <URL_DEL_REPOSITORIO>
cd HausdorffK3-tree-pub

# Configura el proyecto con CMake. Esto crea el directorio 'build' y descarga dependencias.
cmake -B build
```

### 2. Compilar el Proyecto

Usa CMake para invocar al compilador. Esto compilará la biblioteca, las aplicaciones y las pruebas.

```bash
cmake --build build
```

### 3. Ejecutar las Aplicaciones

Los ejecutables se encontrarán dentro del directorio `build/apps/`.

```bash
# En Windows
build\apps\main_app.exe

# En Linux o macOS
./build/apps/main_app
```
Si se añaden más aplicaciones en la carpeta `apps/`, sus ejecutables aparecerán en el mismo lugar.

### 4. Ejecutar las Pruebas

La forma más sencilla de correr las pruebas es usando la herramienta `ctest` de CMake, que las ejecuta automáticamente.

```bash
# Navega al directorio de compilación
cd build

# Ejecuta el conjunto de pruebas
ctest
```

`ctest` mostrará un resumen de las pruebas que pasaron o fallaron. Para un resultado más detallado, puedes ejecutar el programa de pruebas directamente:

```bash
# En Windows
build\tests\run_tests.exe

# En Linux o macOS
./build/tests/run_tests
```
