"reference from :http://www.vimer.cn/2009/10/%E7%94%A8vim%E5%9C%A8%E6%BA%90%E4%BB%A3%E7%A0%81%E4%B8%AD%E6%B7%BB%E5%8A%A0%E4%BD%A0%E7%9A%84%E4%B8%AA%E4%BA%BA%E4%BF%A1%E6%81%AF.html
"进行版权声明的设置
"添加或更新头
"nmap tt :call TitleDet()<cr>
command! -nargs=0 TT :call TitleDet()
"nmap tm : exe 'normal i'.strftime("%Y-%m-%d")<CR>
command! -nargs=0 TM : exe 'normal i'.strftime("%Y-%m-%d")
"call i|strftime("%Y-%m-%d %H:%M")<CR>'s
function! AddTitle()
	call append(0, "/*============================================================================")
	call append(1,"# File Name: ".expand("%:t"))
	call append(2,"# Author: ")
	call append(3,"# Create Time: ".strftime("%Y-%m-%d %H:%M"))
	call append(4,"# File Description: ")
	call append(5,"#")
	call append(6,"# History:")
	call append(7,"# Version\tModifier\tTime\t\t\tDescription")
	call append(8,"# ")
	call append(9,"=============================================================================*/")
	call append(10,"")
	echohl WarningMsg | echo "Successful in adding the title." | echohl None
endf
"更新最近修改时间和文件名
function! UpdateTitle()
	normal m'
	execute '/# *Create Time:/s@:.*$@\=strftime(": %Y-%m-%d %H:%M")@'
	normal ''
	normal mk
	execute '/# *File Name:/s@:.*$@\=": ".expand("%:t")@'
	execute "noh"
	normal 'k
	echohl WarningMsg | echo "Successful in updating the title." | echohl None
endfunction
"判断前10行代码里面，是否有Last modified这个单词，
"如果没有的话，代表没有添加过作者信息，需要新添加；
"如果有的话，那么只需要更新即可
function! TitleDet()
	let n=1
	"默认为添加
	while n < 10
		let line = getline(n)
		if line =~ '^\#\s*\S*Create\sTime:\S*.*$'
			call UpdateTitle()
			return
		endif
		let n = n + 1
	endwhile
	call AddTitle()
endfunction
