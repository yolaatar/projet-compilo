grammar ifcc;

axiom : prog EOF ;

prog : 'int' 'main' '(' ')' '{' inst* return_stmt '}' ;

inst : declaration 
     | assignment 
     | function_call ';' ;

declaration : 'int' decl (',' decl)* ';' ;
decl : ID ('=' expr)? ;
assignment : ID '=' expr ';' ;

return_stmt : RETURN expr ';' ;

expr
    : expr op=('*'|'/'|'%') expr         # MulDivExpr 
    | expr op=('+'|'-') expr             # AddSubExpr
    | '(' expr ')'                       # ParExpr
    | expr op=('<'|'>'|'<='|'>=') expr   # CompExpr
    | expr op=('=='|'!=') expr           # EgalExpr
    | expr '&' expr                      # EtLogExpr
    | expr '^' expr                      # OuExcExpr
    | expr '|' expr                      # OuIncExpr
    | function_call                      # FuncCallExpr 
    | ID                                 # IdExpr
    | CONST                              # ConstExpr
    ;
function_call : ID '(' (expr (',' expr)*)? ')' ;

RETURN : 'return' ;
CONST : [0-9]+ ;
ID : [a-zA-Z_][a-zA-Z0-9_]* ;

COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS : [ \t\r\n] -> channel(HIDDEN);
