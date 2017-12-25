参考:
	http://www.vimer.cn/2010/01/%E7%94%A8vim%E5%9C%A8%E4%BB%A3%E7%A0%81%E6%96%87%E4%BB%B6%E4%B8%AD%E8%87%AA%E5%8A%A8%E6%B7%BB%E5%8A%A0ifdefdefineendif.html
	http://www.vimer.cn/2009/10/%E7%94%A8vim%E5%9C%A8%E6%BA%90%E4%BB%A3%E7%A0%81%E4%B8%AD%E6%B7%BB%E5%8A%A0%E4%BD%A0%E7%9A%84%E4%B8%AA%E4%BA%BA%E4%BF%A1%E6%81%AF.html
	http://www.vimer.cn/2009/10/%E6%8A%8Avim%E6%89%93%E9%80%A0%E6%88%90%E4%B8%80%E4%B8%AA%E7%9C%9F%E6%AD%A3%E7%9A%84ide3.html
	http://www.ibm.com/developerworks/cn/linux/l-vim-script-1/index.html	

安装:
	1. 将header.vim ntitle.vim a.vim DoxygenToolkit.vim拷贝到自己的根目录
        mkdir ~/.vim/; cp a.vim  DoxygenToolkit.vim  header.vim  ntitle.vim ~/.vim/;
	2. 配置~/.vimrc
        cat t_ade.vim >> ~/.vimrc;

使用:
	1.	:HD-------------针对.h文件，可以增加预编译宏 #ifndef #define #endif
	2.	:TT-------------新增文件注释
	3.	:Dox------------在函数名所在行执行，新增函数注释等
	4.	:A--------------在相应的.h和.cpp/.c文件来回切换(支持../include ../src)
	5.	:AS-------------以上下布局打开相应的.h/.c/.cpp文件(支持../include ../src)
	6.	:AV-------------以左右布局打开相应的.h/.c/.cpp文件(支持../include ../src)
	7.      :AL-------------给shell脚本增加公用的一些函数，比如log级别的支持,rsync的封装以及配置文件的读取
