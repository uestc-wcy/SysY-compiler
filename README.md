# SysY-compiler
一个简化版的C语言编译器，实现了源码-中间代码-汇编语言的完整编译流程，实现的具体功能如下：
### 抽象语法树生成
使用Bison工具识别关键字、变量、函数等元素，并生成相应的抽象语法树。
### 语义分析
编译器会对语法树进行语义分析，检查变量类型、函数调用等语义相关的错误。
### 中间代码生成
通过对语法树进行遍历，编译器会生成相应的中间代码，以便后续的优化和生成汇编代码。  
中间代码格式为自定义，下面包含了一些示例：  
&nbsp;&nbsp;&nbsp;
label label1 :&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;定义标签 label1  
function int|void f(a,b[],c...){body}&nbsp;&nbsp;&nbsp;&nbsp;函数定义  
x = alloca n&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;在堆栈分配n字节内存，赋给x。   
x = y&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;赋值操作，将变量 y 赋值给 x  
x = y[z]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;将数组y的第z个值赋给x  
y[z] = x&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;将x赋给数组y的第z个位置上  
x = y [op] z&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;将变量 y 和 z 进行 op 运算的结果传递给 x  
x = #10&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;将立即数 10 的值传递给变量 x  
goto label1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;无条件跳转至 lable1  
if x [relop] y goto z&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;如果 x 与 y 满足[relop]关系则跳转至标号 z  
x = call f(a,b[],c...)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;调用函数 f，并将其返回值赋给 x  
call f(a,b[],c...)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;调用函数 f  
return x&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;退出当前函数并返回 x 值    
&nbsp;&nbsp;&nbsp;
更详细的中间代码介绍可以查看ir.txt   
### 中间代码优化
编译器提供了一系列的优化算法，包括常量折叠、公共子表达式消除、冗余代码消除、活跃变量分析等，可以对生成的中间代码进行优化，以提高程序的运行效率。
### 汇编代码生成
最后，编译器将优化后的中间代码转换为机器可执行的arm平台汇编代码，生成可执行文件。
