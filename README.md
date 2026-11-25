            "args": [
                "-fdiagnostics-color=always",
                "-g",
                "-std=c++11",
                "-I${workspaceFolder}\\include",
                "${workspaceFolder}\\src\\main.cpp",
                "${workspaceFolder}\\src\\product.cpp",
                "${workspaceFolder}\\src\\stock.cpp",
                "${workspaceFolder}\\src\\query.cpp",
                "${workspaceFolder}\\src\\data_io.cpp",
                "-o",
                "${workspaceFolder}\\supermarket.exe"
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            },
