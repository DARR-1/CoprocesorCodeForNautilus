<p align="center">
    <img src="https://raw.githubusercontent.com/PKief/vscode-material-icon-theme/ec559a9f6bfd399b82bb44393651661b08aaf7ba/icons/folder-markdown-open.svg" align="center" width="30%">
</p>
<p align="center"><h1 align="center">COPROCESORCODEFORNAUTILUS</h1></p>
<p align="center">
    <em><code>Servidor del algoritmo A* con conexión cliente/servidor</code></em>
</p>
<p align="center">
    <img src="https://img.shields.io/github/license/DARR-1/CoprocesorCodeForNautilus?style=default&logo=opensourceinitiative&logoColor=white&color=eaff00" alt="license">
    <img src="https://img.shields.io/github/last-commit/DARR-1/CoprocesorCodeForNautilus?style=default&logo=git&logoColor=white&color=eaff00" alt="last-commit">
    <img src="https://img.shields.io/github/languages/top/DARR-1/CoprocesorCodeForNautilus?style=default&color=eaff00" alt="repo-top-language">
    <img src="https://img.shields.io/github/languages/count/DARR-1/CoprocesorCodeForNautilus?style=default&color=eaff00" alt="repo-language-count">
</p>
<br>

## 📍 Overview

CoprocesorCodeForNautilus implementa el algoritmo A* en C++ con un servidor TCP para recibir peticiones de ruta y un cliente en C++ para obtener los resultados. **Nota:** La visualización de los resultados ya no está incluida en esta versión.

---

## 👾 Features

- Búsqueda de rutas óptimas usando A* (`astar.cpp`, `astar.h`)
- Servidor TCP en C++ ([Server.cpp](Server.cpp), [Server.h](Server.h))
- Cliente TCP en C++ ([Client/main.cpp](Client/main.cpp))
- Conexión cliente/servidor mediante sockets (WinSock)
- Modularidad: separación de lógica de A* y conexión de cliente (`ClientConnection.cpp`, `ClientConnection.h`)

---

## 📁 Project Structure

```sh
└── CoprocesorCodeForNautilus/
    ├── CMakeLists.txt
    ├── Client
    │   ├── CMakeLists.txt
    │   ├── build
    │   └── main.cpp
    ├── ClientConnection.cpp
    ├── ClientConnection.h
    ├── LICENSE
    ├── README.md
    ├── Server.cpp
    ├── Server.h
    ├── astar.cpp
    ├── astar.h
    ├── build
    │   ├── NautilusCoproccesor.sln
    │   ├── NautilusCoproccesor.exe
    │   └── Debug
```

---

## 🚀 Getting Started

### ☑️ Prerequisites

- **CMake**: Versión 3.10 o superior.
- **Visual Studio**: Versión 17 2022 (x64).
- **Windows Sockets Library**: Ws2_32.

### ⚙️ Installation

1. Clona el repositorio:
   ```bash
   git clone https://github.com/DARR-1/CoprocesorCodeForNautilus.git
   cd CoprocesorCodeForNautilus
   ```

2. Crea el proyecto con CMake:
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022"
   ```

3. Abre el archivo de solución generado (`NautilusCoproccesor.sln`) en Visual Studio y compila el proyecto.

---

## 🤖 Usage

### Servidor
Ejecuta el servidor para procesar las solicitudes de ruta:
```bash
./NautilusCoproccesor.exe
```

### Cliente
Ejecuta el cliente para enviar solicitudes y obtener resultados:
```bash
./Client.exe
```

---

## 📌 Project Roadmap

- **Visualización**: Implementar una herramienta de visualización en futuras versiones.
- **Compatibilidad multiplataforma**: Extender soporte a Linux y macOS.
- **Optimización**: Mejorar el rendimiento del algoritmo A* para grandes conjuntos de datos.

---

## 🔰 Contributing

¡Las contribuciones son bienvenidas! Haz un fork del repositorio, realiza tus cambios y envía un pull request.

---

## 🎗 License

Este proyecto está licenciado bajo la licencia MIT. Consulta el archivo LICENSE para más detalles.

---

## 🙌 Acknowledgments

Gracias a todos los colaboradores y usuarios que han apoyado este proyecto.