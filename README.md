# Leitor de sensor
Este repositório contém um código em C++ que é inserido em uma Esp32.
O código recebe os dados de dois sensores, um de presença e um de proximidade.
Após isso, os dados são enviados para um [servidor Python](https://github.com/GuiSmith/sharkiot-python), que envia os dados para um servidor [Node.js](https://github.com/GuiSmith/sharkiot-back)
