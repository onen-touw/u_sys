# u_cns (File system library for ESP32)

[![License](https://img.shields.io/badge/license-ASL-red.svg)](LICENSE)
[![C++ Version](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](https://isocpp.org/)

This library is designed for runtime configuration of sensors, system parameters, and anything else you configure. It runs in the UFO environment and requires the installation of some dependent components (see below).

## Contents

- [Features](#Features)
- [Requirements](#Requirements)
- [Dependencies](#Dependencies)
- [Instalation](#Instalation)
- [How to use](#How-to)
- [Documentation](#Documentation)
- [License](#License)
- [Autor](#Autor)

## Features

- C++17 stl containers are used
- Simple API
- Independ on used file system (fatfs, spiffs, ...)

## Requirements

- Any compiler with C++17 and more (GCC 8+, Clang 7+, MSVC 2019+) 
- Other requared UFO modules([see Dependencies](#Dependencies))
- esp-idf with enabled C++17 

## Dependencies

| Library | Version | Description | LIC |
|------------|--------|-----------|----------|
| [![u_sys](https://img.shields.io/badge/u_sys-0.5.0-blue.svg)](link) | ≥ 0.5.0 | UFO System components | ASL |
| [![Console-parser](https://img.shields.io/badge/ConsoleParser-1.0.0-blue.svg)](link) | ≥ 1.0.0 | Console arguments parser | ASL |

## Instalation

For ESP32 instalation depend on which IDE you are using. 
Typically, this involves cloning the repository into the libraries folder and setting up paths in your project's build system.
For PIO, clone the repository into the /lib folder, and specify 
```
lib_ldf_mode = chain+ 
```
in the .ini file for automatic builds or

```
lib_extra_dirs = lib/u_configurator
```

## Documentation

See Doxygen commentaries in code 

Full doc maybe later, sorry

## How-to

Examples of library usage can be found in the following projects:

| Project | Description |
|------------|--------|
| [![u_gte](https://img.shields.io/badge/u_gte-0.0.0-blue.svg)](link) | Gas turbine engien control system 
| [![u_gte](https://img.shields.io/badge/u_gte-0.0.0-blue.svg)](link) | Gas turbine engien control system 


## License

This project is distributed under the terms of the **Academic Software License (ASL)**.

This license permits the use, copying, and distribution of the software exclusively for academic and non-commercial research purposes. Commercial use is prohibited.

The detailed terms of the license are available in the [LICENSE](LICENSE) file located at the root of the repository.

```
/*
* u_sys is © 2026, Anton Granitov (onen-touw), BSTU Voenmeh
*
* u_sys is published and distributed under 
* the Academic Software License v1.0 (ASL).
*
* u_sys is distributed in the hope that it will be useful 
* for non-commercial academic research, but WITHOUT ANY WARRANTY; without
* even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the ASL for more details.
*
* You should have received a copy of the ASL along with this program; 
* if not, write to anton.granitov123@gmail.com or https://github.com/onen-touw.  
* It is also published at LICENSE.md in root folder of this repository.
*
* You may contact the original licensor at anton.granitov123@gmail.com or https://github.com/onen-touw.
*/
```

## Autor

Anton Granitov (onen-touw) [![onen-touw](https://img.shields.io/badge/@onen-touw-181717?logo=github&logoColor=white)](https://github.com/onen-touw)

Email for communication: anton.granitov123@gmail.com
