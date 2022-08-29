# 07 Shell Lab

## 文件内容

- 项目文件：`shlab-handout` 
- 实现文件：`shlab-handout-my`
    - `tsh.c`
    
- 相关说明教程：`guide`

## 如何运行

``` sh
# 在 cachelab-handout_my 目录下
make test01 # 01 ~ 16
```

## 题目分析

实现一个简易 Shell，包含四条内置命令：`quit`，`jobs`，`bg`，`fg`

具体在框架内实现以下函数：

- `eval`
- `builtin_cmd`
- `do_bgfg`
- `waitfg`
- `sigchid_handler`
- `sigint_handler`
- `sigtstp_handler`

这个 Lab 我个人理解的不是非常深入，写的过程中参考了很多书上和网上的代码，因此就不分析了，推荐几个网上的文章：

- [CSAPP-Lab07 Shell Lab 深入解析](https://zhuanlan.zhihu.com/p/492645370)

- [Introduction to CSAPP（二十二）：shell lab - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/151050267)

