#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib> // exit(), system()
#include <cstdio>  // popen()
#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#endif
#include "astar.h"
#include "Server.h"
#include "ClientConnection.h"

#define ROBOT_RADIUS 17

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

// Ejecutar figlet (popen variante cruzada)
void printFigletTitle(const std::string &title)
{
#ifdef _WIN32
    std::string cmd = "bin\\figlet.exe -d fonts \"" + title + "\"";
    FILE *pipe = _popen(cmd.c_str(), "r");
#else
    std::string cmd = "figlet \"" + title + "\"";
    FILE *pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe)
        return;

    char buf[128];
    while (fgets(buf, sizeof(buf), pipe))
        std::cout << "\033[1;35m" << buf << "\033[0m";

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
}

// ... [INCLUDES IGUALES]
#ifdef __linux__

#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

void setupSystemdService(const std::string &binaryPath)
{
    const char *servicePath = "/etc/systemd/system/coproc.service";

    // Verifica si ya existe el servicio
    struct stat buffer;
    if (stat(servicePath, &buffer) == 0)
    {
        std::cout << "✅ Servicio systemd ya está instalado.\n";
        std::cout << "\n\033[1;33m❗ Para ver la salida completa del servicio en ejecución, abre otra terminal y ejecuta:\n"
                  << "    sudo journalctl -u coproc.service -f\n\033[0m\n\n";
        return;
    }

    std::string serviceContent = R"(
[Unit]
Description=Coprocesador Nautilus 4010
After=network.target

[Service]
ExecStart=/usr/local/bin/coproc
WorkingDirectory=/usr/local/bin
Restart=always
RestartSec=3
User=root
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
)";

    std::ofstream serviceFile(servicePath);
    if (!serviceFile)
    {
        std::cerr << "❌ No se pudo crear el archivo de servicio systemd. ¿Ejecutaste con sudo?\n";
        return;
    }

    serviceFile << serviceContent;
    serviceFile.close();
    std::cout << "✅ Archivo de servicio creado.\n";

    // Copiar el ejecutable
    if (binaryPath != "/usr/local/bin/coproc")
    {
        std::string copyCommand = "cp \"" + binaryPath + "\" /usr/local/bin/coproc && chmod +x /usr/local/bin/coproc";
        if (system(copyCommand.c_str()) != 0)
        {
            std::cerr << "❌ Error copiando el ejecutable a /usr/local/bin/coproc\n";
            return;
        }
    }

    system("systemctl daemon-reload");
    system("systemctl enable coproc.service");
    system("systemctl start coproc.service");

    std::cout << "✅ Servicio habilitado y ejecutándose.\n";
    std::cout << "\n\033[1;33m❗ Para ver la salida completa del servicio en ejecución, abre otra terminal y ejecuta:\n"
              << "    sudo journalctl -u coproc.service -f\n\033[0m\n\n";
    std::exit(0);
}
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    system("chcp 65001 > nul");
#else
    // Si no es root, relanzar con sudo
    if (getuid() != 0)
    {
        std::cout << "Requiere permisos de superusuario. Intentando relanzar con sudo...\n";

        std::string cmd = "sudo ";
        for (int i = 0; i < argc; ++i)
        {
            cmd += "\"";
            cmd += argv[i];
            cmd += "\" ";
        }

        return system(cmd.c_str());
    }

    std::cout << "Ejecutando con permisos de root.\n";

    if (argc > 1 && std::strcmp(argv[1], "--install") == 0)
    {
        setupSystemdService(argv[0]);
        return 0;
    }
    
    // Detener el servicio para correr en foreground (solo si existe)
    system("systemctl stop coproc.service");
    setenv("LC_ALL", "en_US.UTF-8", 1);
#endif

    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";
    const std::string cyan = "\033[36m";

    std::cout << bold << cyan;
    std::cout << "\n============================================================\n";
    std::cout << "\033[1;32m=                                                          =\n";
    std::cout << "\033[1;32m=              INICIANDO CODIGO COPROCESADOR               =\n";
    std::cout << "\033[1;32m=                       by: André 🍐                       =\n";
    std::cout << "\033[1;32m=                                                          =\n";
    std::cout << bold << cyan;
    std::cout << "============================================================\n\n";
    std::cout << reset;
    printFigletTitle("Nautilus 4010");

    std::string ipOverride = "";

    for (int i = 1; i < argc - 1; ++i)
    {
        if (std::strcmp(argv[i], "--ip") == 0)
        {
            ipOverride = argv[i + 1];
            break;
        }
    }
    
    Server server(27015, ipOverride);

    if (server.initialize() != 0 || server.listen() != 0)
        return 1;

    Grid grid(ROW, std::vector<int>(COL));
    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if (
                (abs(-j + (176.745 * QUALITY)) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) / 2 &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) - j + (176.745 * QUALITY) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY) &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) + j - (176.745 * QUALITY) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) ||

                (abs(-j + (-176.745 * QUALITY + (COL - 1))) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) / 2 &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) - j + (-176.745 * QUALITY + (COL - 1)) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY) &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) + j - (-176.745 * QUALITY + (COL - 1)) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) ||

                (j > ((COL - 1) / 2) - (6 + ROBOT_RADIUS) * QUALITY &&
                 j < ((COL - 1) / 2) + (6 + ROBOT_RADIUS) * QUALITY &&
                 i > ((ROW - 1) / 2) - (6 + ROBOT_RADIUS) * QUALITY &&
                 i < ((ROW - 1) / 2) + (6 + ROBOT_RADIUS) * QUALITY))
            {
                grid[i][j] = 0;
            }
            else
            {
                grid[i][j] = 1;
            }
        }
    }

    while (true)
    {
        std::cout << "\n🟢 \033[1;32mEsperando nuevo cliente...\033[0m\n";
        ClientConnection client = server.accept();
        if (client.getSocket() != -1)
        {
            std::thread clientThread(&Server::handleClient, &server, std::move(client), std::ref(grid));
            clientThread.detach();
        }
    }

    return 0;
}
