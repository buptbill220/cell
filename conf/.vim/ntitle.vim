command! -nargs=0 TT :call TitleDet()
function! AddTitle()
    call append(0,"/**")
    call append(1," * 名称: ".expand("%:t"))
    call append(2," * 摘要：")
    call append(3," * 作者：".system("whoami| awk '{printf $1}'"))
    call append(4," * 日期：".strftime("%Y.%m.%d"))
    call append(5," *")
    call append(6," * 修改摘要：")
    call append(7," * 修改者：")
    call append(8," * 修改日期：")
	call append(9," */")
endf

function! UpdateTitle()
endfunction

function! TitleDet()
	let n=1
	while n < 10
		let line = getline(n)
		if line =~ ' 文件名称：'
			call UpdateTitle()
			return
		endif
		let n = n + 1
	endwhile
	call AddTitle()
endfunction
