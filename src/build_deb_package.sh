# 1. Компилируем бинарник
make

# 2. Создаём временную рабочую директорию для DEB-пакета
mkdir -p tmp_install_dir/{usr/bin,etc/systemd/system,DEBIAN}

# 3. Копируем собранные файлы в нужную структуру
cp src/asynserver tmp_install_dir/usr/bin/
cp src/asynserver.service tmp_install_dir/etc/systemd/system/

# 4. Добавляем обязательные файлы DEB-пакета
cat > tmp_install_dir/DEBIAN/control <<EOF
Package: my-async-server
Version: 1.0
Architecture: amd64
Maintainer: Your Name <you@example.com>
Depends: libc6
Section: net
Priority: extra
Description: Simple asynchronous server using epoll
EOF

# 5. Создаём DEB-пакет
dpkg-deb --build tmp_install_dir build/mypackage.deb

# 6. Удаляем временные файлы
rm -rf tmp_install_dir
