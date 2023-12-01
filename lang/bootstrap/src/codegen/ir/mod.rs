mod block;
mod expression;
mod statement;

use crate::{ast::block::Block, syntax_error::SyntaxError};
use core::fmt;
use std::{collections::HashMap, fmt::Display};

#[derive(Debug, PartialEq)]
pub struct IR {
    pub statements: Vec<IRStatement>,
}
impl fmt::Display for IR {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for statement in &self.statements {
            writeln!(f, "{}", statement)?
        }
        Ok(())
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Register(pub u32);
impl fmt::Display for Register {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "r{}", self.0)
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Label(u32);
impl fmt::Display for Label {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "L{}", self.0)
    }
}

#[derive(Debug, PartialEq)]
pub enum IRStatement {
    LoadImmediate {
        rd: Register,
        imm: u64,
    },
    Add {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    Subtract {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    And {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    Or {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    Xor {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    LeftShift {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    RightShift {
        rd: Register,
        rs1: Register,
        rs2: Register,
    },
    Branch {
        label: Label,
    },
    BranchNotZero {
        rs1: Register,
        label: Label,
    },
    BranchZero {
        rs1: Register,
        label: Label,
    },
    Label {
        label: Label,
    },
    Push {
        rs1: Register,
    },
    Pop {
        rs1: Register,
    },
}

impl Display for IRStatement {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IRStatement::LoadImmediate { rd, imm } => write!(f, "li {}, {}", rd, imm),
            IRStatement::Add { rd, rs1, rs2 } => write!(f, "add {}, {}, {}", rd, rs1, rs2),
            IRStatement::Subtract { rd, rs1, rs2 } => write!(f, "sub {}, {}, {}", rd, rs1, rs2),
            IRStatement::And { rd, rs1, rs2 } => write!(f, "and {}, {}, {}", rd, rs1, rs2),
            IRStatement::Or { rd, rs1, rs2 } => write!(f, "or {}, {}, {}", rd, rs1, rs2),
            IRStatement::Xor { rd, rs1, rs2 } => write!(f, "xor {}, {}, {}", rd, rs1, rs2),
            IRStatement::LeftShift { rd, rs1, rs2 } => write!(f, "sll {}, {}, {}", rd, rs1, rs2),
            IRStatement::RightShift { rd, rs1, rs2 } => write!(f, "srl {}, {}, {}", rd, rs1, rs2),
            IRStatement::Branch { label } => write!(f, "j {}", label),
            IRStatement::BranchNotZero { rs1, label } => write!(f, "bnz {}, {}", rs1, label),
            IRStatement::BranchZero { rs1, label } => write!(f, "bz {}, {}", rs1, label),
            IRStatement::Label { label } => write!(f, "{}:", label),
            IRStatement::Push { rs1 } => write!(f, "push {}", rs1),
            IRStatement::Pop { rs1 } => write!(f, "pop {}", rs1),
        }
    }
}

trait IRWalkable {
    type Output;
    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError>;
}

#[derive(Debug)]
struct IRState<'a> {
    statements: Vec<IRStatement>,
    scope: Option<Scope<'a>>,
    current_register: u32,
    current_label: u32,
    current_loop_continue_label: Option<u32>,
    current_loop_break_label: Option<u32>,
}
#[derive(Debug, Clone)]
struct Scope<'a> {
    previous_scope: Option<Box<Scope<'a>>>,
    symbols: HashMap<&'a str, u32>,
}

pub fn get_ir<'a>(program: &'a Block) -> Result<IR, SyntaxError> {
    let mut ir = IRState {
        statements: vec![],
        scope: None,
        current_register: 0,
        current_label: 0,
        current_loop_continue_label: None,
        current_loop_break_label: None,
    };

    program.walk_ir(&mut ir)?;
    Ok(IR {
        statements: ir.statements,
    })
}

fn get_identifier_register<'a>(scope: Option<Scope>, identifier: &'a str) -> Option<u32> {
    let mut current_scope_option = scope;
    loop {
        if current_scope_option.is_none() {
            break;
        }

        let current_scope = current_scope_option.unwrap();

        match current_scope.symbols.get(identifier) {
            Some(symbol) => return Some(*symbol),
            None => {}
        }

        current_scope_option = match current_scope.previous_scope {
            Some(scope) => Some(*scope),
            None => None,
        };
    }

    None
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            block::Block,
            expression::{BinaryOperation, BinaryOperationType, Expression, Literal},
            statement::{AssignmentStatement, Statement},
        },
        codegen::ir::{IRStatement, Register, IR},
        parser::Parser,
    };

    use super::get_ir;

    #[test]
    fn test_get_ir() {
        let stmt = Statement::Assignment(AssignmentStatement {
            identifier: "x".to_string(),
            expression: Expression::BinaryOperation(BinaryOperation {
                operation_type: BinaryOperationType::Add,
                left_expression: Box::new(Expression::Literal(Literal::IntegerLiteral(1))),
                right_expression: Box::new(Expression::Literal(Literal::IntegerLiteral(2))),
            }),
        });

        let program = Block {
            statements: vec![stmt],
        };

        let ir = get_ir(&program).unwrap();

        assert_eq!(
            ir,
            IR {
                statements: vec![
                    IRStatement::LoadImmediate {
                        rd: Register(1),
                        imm: 1
                    },
                    IRStatement::LoadImmediate {
                        rd: Register(2),
                        imm: 2
                    },
                    IRStatement::Add {
                        rd: Register(3),
                        rs1: Register(1),
                        rs2: Register(2),
                    },
                ]
            },
        );
    }

    #[test]
    fn test_get_ir_simple_if() {
        let mut parser = Parser::new("if (10 + 20 - 30 | 40 & 50 ^ 60 << 70 >> 80) {};");
        let program = parser.get_ast().unwrap().unwrap();
        let ir = get_ir(&program).unwrap();

        for stmt in ir.statements {
            println!("{}", stmt);
        }
    }

    #[test]
    fn test_get_ir_simple_loop() {
        let mut parser = Parser::new("loop { if 1 + 1 { break; }; };");
        let program = parser.get_ast().unwrap().unwrap();
        let ir = get_ir(&program).unwrap();

        for stmt in ir.statements {
            println!("{}", stmt);
        }
    }

    #[test]
    fn test_get_ir_branched_assignment() {
        let mut parser = Parser::new("let x; if 1 { x = 1; } else { x = 2; };");
        let program = parser.get_ast().unwrap().unwrap();
        let ir = get_ir(&program).unwrap();

        for stmt in ir.statements {
            println!("{}", stmt);
        }
    }
}
