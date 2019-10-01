import vim
def py(s, e):
    for l in range(s-1, e):
        vim.current.buffer[l]="#"+vim.current.buffer[l]
		
def cpp(s, e):
    for l in range(s-1, e):
        vim.current.buffer[l]="//"+vim.current.buffer[l]