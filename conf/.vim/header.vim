"reference from :http://www.vimer.cn/2010/01/%E7%94%A8vim%E5%9C%A8%E4%BB%A3%E7%A0%81%E6%96%87%E4%BB%B6%E4%B8%AD%E8%87%AA%E5%8A%A8%E6%B7%BB%E5%8A%A0ifdefdefineendif.html
function! InsertHeadDef(firstLine, lastLine)
	"校验范围是否在当前Buffer的总行数之内
	if a:firstLine <1 || a:lastLine> line('$')
		echoerr 'InsertHeadDef : Range overflow !(FirstLine:'.a:firstLine.';LastLine:'.a:lastLine.';ValidRange:1~'.line('$').')'
		return ''
	endif
	let sourcefilename=expand("%:t")
	let definename=substitute(sourcefilename,' ','','g')
	let definename=substitute(definename,'\.','_','g')
	let definename = toupper(definename)
	" 跳转到指定区域的第一行，开始操作
	exe 'normal '.a:firstLine.'GO'
	call setline('.', '#ifndef _'.definename."_")
	normal == o
	call setline('.', '#define _'.definename."_")
	normal == o
	exe 'normal =='.(a:lastLine-a:firstLine+1).'jo'
	call setline('.', '#endif')
	normal == O 
	" 跳转光标
	let goLn = a:firstLine+2
	exe 'normal =='.goLn.'G'
endfunction
"Normal模式下插入头定义
function! InsertHeadDefN()
	let firstLine = 1
	let lastLine = line('$')
	let n=1
	while n < 20
		let line = getline(n)
		if n==1 
			if line =~ '^\/\*.*$'
				let n = n + 1
				continue
			else
				break
			endif
		endif
		if line =~ '^.*\*\/$'
			let firstLine = n+1
			break
		endif
		let n = n + 1
	endwhile
	call InsertHeadDef(firstLine, lastLine)
endfunction
"nmap hd :call InsertHeadDefN()<CR>
command! -nargs=0 HD :call InsertHeadDefN()
