# SmartTradePlatform
目的：通用交易平台：旨在开发一套通用的、高性能、高可用以及高扩展的可以承载证券、期货以及现货交易系统或柜台系统的技术平台！

语言：C、C++、GOLANG、LUA等；
产品：mysql等；
架构：CS+BS。

开发环境：本项目用eclipse-cdt开发，打开Eclipse-cdt后直接import进去即可，很方便：）。

编译环境：可以直接在eclipse中编译，也可以用cmake/make编译。


run test...

[rock@localhost CMakeBuild]$ ls
CMakeBuild  CMakeCache.txt  CMakeFiles  cmake_install.cmake  install_manifest.txt  Makefile  SmartUtils  STPBIN  STPTest  Testing
[rock@localhost CMakeBuild]$ pwd
/home/rock/git/STP/CMakeBuild
[rock@localhost CMakeBuild]$ make test
make: *** No rule to make target 'test'.  Stop.
[rock@localhost CMakeBuild]$ cd STPTest/
[rock@localhost STPTest]$ ctest
Test project /home/rock/git/STP/CMakeBuild/STPTest
    Start 1: STPTesting
^C
[rock@localhost STPTest]$ 


coding style: follow c++ std library

author: TheRockLiuHY(rockjohnson@163.com).
注意：部分代码暂时不能开源，所以近期更新会比较慢。






