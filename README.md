# async-epoll-server
Simple asynchronous server written in C++ that handles TCP and UDP requests using Linux's  epoll  mechanism. Supports basic commands like  /time ,  /stats , and  /shutdown . Designed without external libraries for maximum portability and performance under GNU/Linux environments.

```
src/
|- server.c   # Основной код сервера
|- makefile   # Файл компиляции
|- service.sh # Скрипт установки сервиса
|- asynserver.service # Конфигурация SystemD
doc/
|- README.md  # Документация по проекту
build/
|- debian     # Директория для подготовки .deb-пакета
```

# Подготовка структуры директорий
mkdir src doc build && cd src
touch server.c makefile service.sh
cd ../build
mkdir debian
 
