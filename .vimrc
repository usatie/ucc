:autocmd BufEnter *.c,*.h setl cc=80
let g:syntastic_c_checkers = ['gcc', 'norminette']
let g:syntastic_c_norminette_exec = './mynorminette'
