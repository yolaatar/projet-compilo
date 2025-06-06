grammar ifcc;

axiom : prog* EOF ;

prog : type ID '(' decl_params ')' '{' (decl | inst)* '}' ;

block : '{' (decl | inst)* '}' ;

decl_params : ( param (',' param)* )? ;
param : 'int' ID ;

inst : declaration 
     | assignment 
     | return_stmt 
     | function_call ';' 
     | if_stmt 
     | while_stmt
     | block
     ;

declaration : 'int' decl (',' decl)* ';' ;
decl : ID ('=' expr)? ;
assignment 
    : ID '=' expr ';'               # Assign
    | ID '+=' expr ';'              # PlusAssign
    | ID '-=' expr ';'              # MinusAssign
    | ID '*=' expr ';'              # MulAssign
    | ID '/=' expr ';'              # DivAssign
    ;
if_stmt : 'if' '(' expr ')' block ('else' block)? ;
while_stmt : 'while' '(' expr ')' block ;

return_stmt : 'return' expr? ';' ;

type : 'int' | 'void' ;

expr
    : '-' expr                           # MoinsExpr
    | '!' expr                           # NotExpr
    | expr op=('*'|'/'|'%') expr         # MulDivExpr 
    | expr op=('+'|'-') expr             # AddSubExpr
    | '(' expr ')'                       # ParExpr
    | expr op=('<'|'>'|'<='|'>=') expr     # CompExpr
    | expr op=('=='|'!=') expr           # EgalExpr
    | expr '&' expr                      # EtLogExpr
    | expr '^' expr                      # OuExcExpr
    | expr '|' expr                      # OuIncExpr
    | expr '&&' expr                     # EtParExpr
    | expr '||' expr                     # OuParExpr
    | function_call                      # FuncCallExpr 
    | ID                                 # IdExpr
    | CONST                              # ConstExpr
    | CHAR                               # CharExpr
    ;

function_call : ID '(' (expr (',' expr)*)? ')' ;

RETURN : 'return' ;
CONST : [0-9]+ ;
ID : [a-zA-Z_][a-zA-Z0-9_]* ;
CHAR : '\'' [a-zA-Z] '\'' ;

COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS : [ \t\r\n] -> channel(HIDDEN);
