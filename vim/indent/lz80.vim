" Vim indent file
" Language: lz80

if exists("b:did_indent")
  finish
endif

runtime! indent/l65.vim

setlocal indentexpr=GetLz80Indent()

" Only define the shared function once; the buffer-local setup above must still
" run for every lz80 buffer.
if exists("*GetLz80Indent")
  finish
endif

let s:lz80_label = '^\s*@@\?\k\+'
let s:lz80_mnemonic = '^\s*\%(adc\|add\|ana\|and\|bit\|call\|ccf\|cp\|cpd\|cpdr\|cpi\|cpir\|cpl\|daa\|dec\|di\|djnz\|ei\|ex\|exx\|halt\|im\|in\|inc\|ind\|indr\|ini\|inir\|jp\|jr\|ld\|ldd\|lddr\|ldh\|ldi\|ldir\|neg\|nop\|or\|ora\|otdr\|otir\|out\|outd\|outi\|pop\|push\|res\|ret\|reti\|retn\|rl\|rla\|rlc\|rlca\|rld\|rr\|rra\|rrc\|rrca\|rrd\|rst\|sbc\|scf\|set\|sla\|sra\|srl\|stop\|sub\|swap\|xor\|dc\.[bwl]\)\%([.]\%(nz\|z\|nc\|c\)\)\?\%($\|\s\)'
let s:lz80_operand = '\%(\k\+\|\[[^]]\+\]\)'
let s:lz80_assignment = s:lz80_operand . '\s*\%(:=\|+=\|-=\|&=\||=\|\^=\)'
let s:lz80_postfix = s:lz80_operand . '\s*\%(++\|--\)\%($\|\s\)'

function s:IsLz80Statement(line)
  return a:line =~# s:lz80_mnemonic
        \ || a:line =~# '^\s*' . s:lz80_assignment
        \ || a:line =~# '^\s*' . s:lz80_postfix
endfunction

function GetLz80Indent()
  let base = GetL65Indent()
  let current = getline(v:lnum)
  let previous_lnum = prevnonblank(v:lnum - 1)
  if previous_lnum == 0
    return base
  endif

  let previous = getline(previous_lnum)
  let current_is_statement = s:IsLz80Statement(current)
  let previous_is_statement = s:IsLz80Statement(previous)

  if current =~# s:lz80_label
    return previous_is_statement ? max([indent(previous_lnum) - shiftwidth(), 0]) : base
  endif

  if previous =~# s:lz80_label && current_is_statement
    return indent(previous_lnum) + shiftwidth()
  endif

  if current_is_statement && previous_is_statement
    return indent(previous_lnum)
  endif

  " Return to the label's indentation when Lua resumes after assembly lines.
  if previous_is_statement && current !~# '^\s*--'
    return max([base - shiftwidth(), 0])
  endif

  return base
endfunction
