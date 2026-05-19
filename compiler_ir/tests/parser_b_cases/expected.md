# Parser B Test Cases - Expected Results

All cases use the default C-- grammar from Parser A and the Parser B driver.
The AST structure is shown as a tree (parent -> children). Terminal nodes use their grammar terminals.
Positions are expected to be taken from the first token of each subtree (see AST node position rule).

## Case 1: var declaration
Input: tests/parser_b_cases/case1.sy

Expected:
- Parse result: accept
- Reduction sequence (production ids): 11 18 17 15 7 4 3 1
- AST (structure):
  - Program
    - compUnit
      - element
        - decl
          - varDecl
            - bType
              - int
            - varDefList
              - varDef
                - Ident (lexeme: a)
            - ;

## Case 2: function with return
Input: tests/parser_b_cases/case2.sy

Expected:
- Parse result: accept
- Reduction sequence (production ids): 11 72 71 65 61 58 53 50 48 46 45 40 34 32 29 22 5 3 1
- AST (structure):
  - Program
    - compUnit
      - element
        - funcDef
          - bType
            - int
          - Ident (lexeme: main)
          - (
          - )
          - block
            - {
            - blockItemList
              - blockItem
                - stmt
                  - return
                  - exp
                    - lOrExp
                      - lAndExp
                        - eqExp
                          - relExp
                            - addExp
                              - mulExp
                                - unaryExp
                                  - primaryExp
                                    - number
                                      - IntConst (lexeme: 0)
                  - ;
            - }

## Case 3: syntax error
Input: tests/parser_b_cases/case3.sy

Expected:
- Parse result: error
- Reduction sequence (production ids): <empty>
- Error message format: "Syntax error at line 1, column 5: unexpected ';' (;)"
- AST: empty
