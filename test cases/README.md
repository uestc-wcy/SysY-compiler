# 测试用例

`functinal_test` ：存放功能测试用例,共80个测试用例，每个测试用例由同名文件的`.sy` , `.out`和`.in`文件组成

`performance_test` ：存放性能测试用例,共6个测试用例

`libsysy.a`: arm架构的SysY运行时静态库

`libsysy.so`: arm架构的SysY运行时动态库

`sylib.h`: SysY运行时库头文件

# 测评镜像

测评镜像的Dockerfile被存放在`docker`文件夹下,其中ARM-Dockerfile是树莓派上的汇编链接镜像,x86-Dockerfile是x86上的编译测评镜像