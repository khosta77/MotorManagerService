# Задание

Проект собирается и компилируется! Проверил на `mac OS 15` и `Ubuntu 24`.

1. Работа с Json реализована через [nlohmann/json](https://github.com/nlohmann/json?ysclid=m9h6e6grnw955784922)
 просто:

```cmd
git clone git@github.com:nlohmann/json.git
```

2. Надо проверить зависимости, должен быть `cmake>=3.28`, `clang++>=16.0`, `gtest` и `boost` (На `mac OS`
установлен через homebrew) и еще

* cmake `cmake --version`

* make `make --version`

* lcov `lcov --version`

* genhtml `genhtml --version`

* clang++ `clang++ --version`

3. Сборка & скомпилировать все

```cmd
cmake -DENABLE_COVERAGE=ON -B build
cd build
make
```

4. Подготовка файлов покрытия

```cmd
make coverage
```

5. Вызов программ:

```cmd
./TransformationServer.out 127.0.0.1 34100 127.0.0.1 34000
```

