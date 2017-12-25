" vimrc by xiaobin.wxb@taobao.com
" Last Update: 2013-02-28

set background=dark
syntax enable
syntax on       		" 语法高亮
set nocompatible    " 关闭兼容模式(vi)
set clipboard+=unnamed "与windows共享剪贴板

set incsearch       " 增量式搜索
set hlsearch        " 高亮搜索

set autoindent      " 自动缩进
set smartindent     " 智能缩进
set cindent         " C/C++风格缩进

set number
set showmatch
set cursorline

" tab转化为4个字符
set expandtab
set smarttab
set shiftwidth=4
set tabstop=4

"set t_Co=65536
"set runtimepath=$HOME/.vim,$VIMRUNTIME
colo elflord

"hi 	Statement 		    ctermfg=White 	    cterm=Bold  "return
"hi 	Normal 				ctermbg=black 	    ctermfg=Green
"hi 	Function 			cterm=Bold 			ctermfg=Red 		
hi 	Search 				cterm=Bold			ctermbg=LightYellow 	ctermfg=Green
hi 	Comment 			ctermfg=DarkCyan    "Brown   "LightGray
hi 	Number   			ctermfg=White
hi 	Float   			ctermfg=White
hi 	Type     			cterm=Bold          ctermfg=LightBlue
hi 	PreProc  			ctermfg=LightCyan
hi 	LineNr				cterm=Bold  		ctermfg=DarkCyan
hi 	Operator 			ctermfg=Blue "Magenta 
hi 	Constant			cterm=Bold 			ctermfg=White
hi 	CursorLine		    ctermbg=DarkGray   "Cyan
hi 	String 				ctermfg=Magenta
hi 	Keyword 			ctermfg=Yellow 	    cterm=Bold 
hi 	Boolean				ctermfg=White 	"true,false  
hi 	Conditional 	    ctermfg=Yellow 	"if,else,swith
hi	Repeat				ctermfg=Yellow 	"while
hi 	Character    	    ctermfg=White 
hi	Special				ctermfg=Red
hi	Typedef				ctermfg=Yellow
hi  MyMathOperator      ctermfg=Yellow  
hi  MyIOOperator        ctermfg=Yellow  
hi  MyCompOperator      ctermfg=Yellow  
hi  StatusLine          ctermbg=White
hi  StatusLineNC        ctermfg=Red

" 设置字符集编码，默认使用utf8
set encoding=utf8
set fileencodings=utf8,gb2312,gb18030,ucs-bom,latin1

" 自动增加title(:TT)
filetype on
au FileType c,cpp,java so ~/.vim/ntitle.vim

" .h文件自动增加#ifndef #define 宏(:HD)
au BufNewFile,BufRead *.h so ~/.vim/header.vim

" .h .cpp/.c文件切换(:A)
au FileType c,cpp so ~/.vim/a.vim

" 自动增加函数注释(:Dox)
au FileType c,cpp so ~/.vim/DoxygenToolkit.vim 
