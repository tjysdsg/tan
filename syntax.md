# Grammar of `tan`

**OUTDATED**

## Symbols/Operators

### Unary operators
```
POS = '+'
NEG = '-'
LNOT = '!'
BNOT = '~'
ADR_REF = '&'
ADR_DEREF = '*'
```

### Binary operators
```
LSHIFT = '<<'
RSHIFT = '>>'
EQ = '=='
NE = '!='
LT = '<'
LE = '<='
GT = '>'
GE = '>='
LAND = '&&'
BAND = '&'
LOR = '||'
BOR = '|'
BXOR = '^'
ADD = '+'
SUB = '-'
MUL = '*'
DIV = '/'
MOD = '%'
```

### Assignment operators
```
ADD_ASSIGN = '+='
SUB_ASSIGN = '-='
BOR_ASSIGN = '|='
BXOR_ASSIGN = '^='
BAND_ASSIGN = '&='
LSHIFT_ASSIGN = '<<='
RSHIFT_ASSIGN = '>>='
MUL_ASSIGN = '*='
DIV_ASSIGN = '/='
MOD_ASSIGN = '%='
```

### Identifier And Literals
```
IDENTIFIER = [a-zA-Z_][a-zA-Z0-9_]*
```

```
DOUBLE_QUOTE = '"'
SINGLE_QUOTE = '\''
INT = 0
     |[1-9][0-9]*
     |0[xX][0-9a-fA-F]+
     |0[0-7]+
     |0[bB][0-1]+
FLOAT = [0-9]* '.' [0-9]* ([eE],[+-]?,[0-9]+)?
CHAR = SINGLE_QUOTE . SINGLE_QUOTE
STR = DOUBLE_QUOTE [^DOUBLE_QUOTE^'\n'^'\l']* DOUBLE_QUOTE
```

## EBNF Grammar For `tan`

### Declarations
```
type_list = type (',' type)*
name_list = IDENTIFIER (',' IDENTIFIER)*

base_type = IDENTIFIER
          | 'fn' '(' type_list? ')' ('->' type)?
          | '(' type ')'
type = base_type ('[' expr? ']' | '*')*

enum_item = IDENTIFIER ('=' expr)?
enum_items = enum_item (',' enum_item)* ','?
enum_decl = IDENTIFIER '{' enum_items? '}'

aggregate_field = name_list ':' type ';'
aggregate_decl = IDENTIFIER '{' aggregate_field* '}'

var_decl = 'var' IDENTIFIER '=' expr
         | 'var' IDENTIFIER ':' type ('=' expr)?

const_decl = 'let' IDENTIFIER '=' expr

typedef_decl = 'typename' IDENTIFIER '=' type

func_param = IDENTIFIER ':' type
func_param_list = func_param (',' func_param)*
func_decl = 'fn' IDENTIFIER '(' func_param_list? ')' '->' type stmt_block

decl = 'enum' enum_decl
     | 'struct' aggregate_decl
     | 'union' aggregate_decl
     |  var_decl
     |  const_decl
     |  typedef_decl
     |  func_decl
```

### Statements
```
assign_op = '='
        |ADD_ASSIGN
        |SUB_ASSIGN|OR_ASSIGN
        |XOR_ASSIGN |LSHIFT_ASSIGN
        |RSHIFT_ASSIGN
        |MUL_ASSIGN|DIV_ASSIGN|MOD_ASSIGN

switch_case = ('case' expr | 'default') ':' stmt*

switch_block = '{' switch_case* '}'

stmt_block = '{' stmt* '}'

stmt = 'return' expr ';'
     | 'break' ';'
     | 'continue' ';'
     | stmt_block
     | 'if' '(' expr ')' stmt_block ('else' 'if' '(' expr ')'
            stmt_block)* ('else' stmt_block)?
     | 'while' '(' expr ')' stmt_block
     | 'for' '(' stmt_list ';' expr ';' stmt_list ')' stmt_block
     | 'do' stmt_block 'while' '(' expr ')' ';'
     | switch '(' expr ')' switch_block
     | expr (assign_op expr)? ';'
```

### Expression
```
cast_expr = '#cast' '(' expr ',' type ')'
sizeof_expr = '#sizeof' '(' expr ')'
              |'#sizeof' '(' type ')'

literal_expr = INT
             | FLOAT
             | STR

expr0 = '(' expr ')'
        |IDENTIFIER

expr1 = expr0
        |literal_expr

func_call = IDENTIFIER '(' expr (',' expr)* ')'
            |IDENTIFIER '(' ')'

expr2 = func_call
        |expr0 '[' expr ']'
        |expr0 '.' func_call

expr3 = (POS|NEG|LNOT|BNOT) expr2
        |cast_expr
        |(ADR_DEREF|ADR_REF) IDENTIFIER
        |sizeof_expr

expr4 = expr3 (MUL|DIV|MOD) expr3

expr5 = expr4 (ADD|SUB) expr4

expr6 = expr5 (LSHIFT|RSHIFT) expr5

expr7 = expr6 (LT|GT|LE|GE) expr6

expr8 = expr7 (EQ|NE) expr7

expr9 = expr8 BAND expr8
expr10 = expr9 BXOR expr9
expr11 = expr10 BOR expr10
expr12 = expr11 BOR expr11
expr13 = expr12 LAND expr12
expr14 = expr13 LOR expr13
expr15 = expr14
            ('='|ADD_ASSIGN|SUB_ASSIGN|MUL_ASSIGN|MOD_ASSIGN|DIV_ASSIGN|LSHIFT_ASSIGN|
            RSHIFT_ASSIGN|BOR_ASSIGN|BXOR_ASSIGN|BAND_ASSIGN) expr14

expr = expr15
```
