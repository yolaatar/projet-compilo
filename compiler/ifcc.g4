grammar ifcc;

axiom : prog EOF ;

prog : 'int' 'main' '(' ')' '{' inst* return_stmt '}' ;

inst : declaration 
     | assignment ;

declaration : 'int' decl (',' decl)* ';' ;
decl : ID ('=' expr)? ;

assignment : ID '=' expr ';' ;

return_stmt : RETURN expr ';' ;

expr
    : expr '*' expr               # MulExpr
    | expr op=('+'|'-') expr      # AddSubExpr
    | '(' expr ')'                # ParExpr
    | ID                          # IdExpr
    | CONST                       # ConstExpr
    ;

RETURN : 'return' ;
CONST : [0-9]+ ;
ID : [a-zA-Z_][a-zA-Z0-9_]* ;

COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS : [ \t\r\n] -> channel(HIDDEN);
