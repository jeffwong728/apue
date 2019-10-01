import os
import sys
import vim

def rg(s, e):
    str = ""
    for i in range(s, e):
        str += "%d, " % i
    str = str[0:-2]
    pos = vim.current.window.cursor
    vim.current.buffer[pos[0]-1]=str

def re(r, n):
    str = r*n
    pos = vim.current.window.cursor
    vim.current.buffer[pos[0]-1]=str