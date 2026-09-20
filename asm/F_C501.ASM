; F_C501 -- dispatch one secondary stream command by the AH selector.
; The six helper calls are direct same-segment relationships in the original
; no-fixup object, retained here as symbolic relative expressions.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS
public _f_c501
_f_c501:
 push bp
 mov bp,sp
 mov al,es:[di+1]
 cmp ah,0
 jnz short test_two
 call $+3ch
 jmp short done
 nop
test_two:
 cmp ah,2
 jg short test_three
 call $+4fh
 jmp short done
 nop
test_three:
 cmp ah,3
 jnz short test_four
 call $+85h
 jmp short done
 nop
test_four:
 cmp ah,4
 jnz short test_five
 call $+6ch
 jmp short done
 nop
test_five:
 cmp ah,5
 jnz short test_six
 call $+7ah
 jmp short done
 nop
test_six:
 cmp ah,6
 jnz short done
 call $+82h
done:
 pop bp
 ret
_TEXT ends
end
