grammar ifcc;

axiom : prog EOF ;

prog : 'int' 'main' '(' ')' '{' stmt+ '}' ;

stmt : decl ';'                   // déclaration de variable (avec ou sans initialisation)
     | assignment ';'             // affectation à une variable
     | return_stmt ';'            // instruction de retour
     ;

decl : 'int' ID ('=' expr)? ;      // par exemple, "int x;" ou "int x = expr;"

assignment : ID '=' expr ;        // affectation : x = expr;

return_stmt : RETURN expr ;       // retour : return expr;

expr : additiveExpr ;             // point d'entrée pour l'expression

// Opérateurs additifs (gère + et -) avec associativité gauche
additiveExpr : multiplicativeExpr ( ('+' | '-') multiplicativeExpr )* ;

// Opérateurs multiplicatifs (gère * et /) avec associativité gauche
multiplicativeExpr : primaryExpr ( ('*' | '/') primaryExpr )* ;

// Expression primaire : constante, variable ou parenthésée
primaryExpr : CONST            # ConstExpr
            | ID               # ExprVariable
            | '(' expr ')'     # ParensExpr
            ;

RETURN  : 'return' ;
ID      : [a-zA-Z_][a-zA-Z0-9_]* ;
CONST   : [0-9]+ ;

COMMENT   : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS        : [ \t\r\n]+ -> skip ;
